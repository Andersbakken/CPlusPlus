/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "cppmodelmanager.h"

#include "builtinindexingsupport.h"
//#include "cppcompletionassist.h"
#include "cppindexingsupport.h"
#include "cpptoolsconstants.h"
#include "cppfindreferences.h"

#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>
#include <utils/runextensions.h>

#include <cplusplus/AST.h>
#include <cplusplus/ASTVisitor.h>
#include <cplusplus/Control.h>
#include <cplusplus/CoreTypes.h>
#include <cplusplus/Lexer.h>
#include <cplusplus/Literals.h>
#include <cplusplus/NameVisitor.h>
#include <cplusplus/Names.h>
#include <cplusplus/Overview.h>
#include <cplusplus/Parser.h>
#include <cplusplus/Scope.h>
#include <cplusplus/Symbols.h>
#include <cplusplus/Token.h>
#include <cplusplus/TranslationUnit.h>
#include <cplusplus/TypeVisitor.h>
#include <cplusplus/pp.h>

#include <QCoreApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QTime>
#include <QTimer>
#include <QtConcurrentMap>
#include <QTextBlock>
#include <QtConcurrentRun>
#include <QFutureSynchronizer>
#include <QDir>

#include <functional>
#include <iostream>
#include <sstream>

namespace CppTools {

uint qHash(const ProjectPart &p)
{
    uint h = qHash(p.defines) ^ p.cVersion ^ p.cxxVersion ^ p.cxxExtensions ^ p.qtVersion;

    foreach (const QString &i, p.includePaths)
        h ^= qHash(i);

    foreach (const QString &f, p.frameworkPaths)
        h ^= qHash(f);

    return h;
}

bool operator==(const ProjectPart &p1,
                const ProjectPart &p2)
{
    if (p1.defines != p2.defines)
        return false;
    if (p1.cVersion != p2.cVersion)
        return false;
    if (p1.cxxVersion != p2.cxxVersion)
        return false;
    if (p1.cxxExtensions != p2.cxxExtensions)
        return false;
    if (p1.qtVersion!= p2.qtVersion)
        return false;
    if (p1.includePaths != p2.includePaths)
        return false;
    return p1.frameworkPaths == p2.frameworkPaths;
}

} // namespace CppTools

using namespace CppTools;
using namespace CppTools::Internal;
using namespace CPlusPlus;

#if defined(QTCREATOR_WITH_DUMP_AST) && defined(Q_CC_GNU)

#include <cxxabi.h>

class DumpAST: protected ASTVisitor
{
public:
    int depth;

    DumpAST(Control *control)
        : ASTVisitor(control), depth(0)
    { }

    void operator()(AST *ast)
    { accept(ast); }

protected:
    virtual bool preVisit(AST *ast)
    {
        std::ostringstream s;
        PrettyPrinter pp(control(), s);
        pp(ast);
        QString code = QString::fromStdString(s.str());
        code.replace('\n', ' ');
        code.replace(QRegExp("\\s+"), " ");

        const char *name = abi::__cxa_demangle(typeid(*ast).name(), 0, 0, 0) + 11;

        QByteArray ind(depth, ' ');
        ind += name;

        printf("%-40s %s\n", ind.constData(), qPrintable(code));
        ++depth;
        return true;
    }

    virtual void postVisit(AST *)
    { --depth; }
};

#endif // QTCREATOR_WITH_DUMP_AST

static const char pp_configuration[] =
    "# 1 \"<configuration>\"\n"
    "#define __cplusplus 1\n"
    "#define __extension__\n"
    "#define __context__\n"
    "#define __range__\n"
    "#define   restrict\n"
    "#define __restrict\n"
    "#define __restrict__\n"

    "#define __complex__\n"
    "#define __imag__\n"
    "#define __real__\n"

    "#define __builtin_va_arg(a,b) ((b)0)\n"

    // ### add macros for win32
    "#define __cdecl\n"
    "#define __stdcall\n"
    "#define QT_WA(x) x\n"
    "#define CALLBACK\n"
    "#define STDMETHODCALLTYPE\n"
    "#define __RPC_FAR\n"
    "#define __declspec(a)\n"
    "#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method\n"
    "#define __try try\n"
    "#define __except catch\n"
    "#define __finally\n"
    "#define __inline inline\n"
    "#define __forceinline inline\n";

CppPreprocessor::CppPreprocessor(QPointer<CppModelManager> modelManager, bool dumpFileNameWhileParsing)
    : m_snapshot(modelManager->snapshot()),
      m_modelManager(modelManager),
      m_dumpFileNameWhileParsing(dumpFileNameWhileParsing),
      m_preprocess(this, &m_env),
      m_revision(0)
{
    m_preprocess.setKeepComments(true);
}

CppPreprocessor::~CppPreprocessor()
{ }

void CppPreprocessor::setRevision(unsigned revision)
{ m_revision = revision; }

void CppPreprocessor::setWorkingCopy(const CppModelManagerInterface::WorkingCopy &workingCopy)
{ m_workingCopy = workingCopy; }

void CppPreprocessor::addDefinitions(const QStringList &definitions)
{
    QList<Macro> macros;
    macros.reserve(definitions.size());
    for (int i = 0; i < definitions.size(); ++i) {
        Macro macro;
        macro.setName(definitions.at(i).toLocal8Bit());
        macros.append(macro);
    }
    m_env.addMacros(macros);
}

void CppPreprocessor::setIncludePaths(const QStringList &includePaths)
{
    m_includePaths.clear();

    for (int i = 0; i < includePaths.size(); ++i) {
        const QString &path = includePaths.at(i);

        if (Utils::HostOsInfo::isMacHost()) {
            if (i + 1 < includePaths.size() && path.endsWith(QLatin1String(".framework/Headers"))) {
                const QFileInfo pathInfo(path);
                const QFileInfo frameworkFileInfo(pathInfo.path());
                const QString frameworkName = frameworkFileInfo.baseName();

                const QFileInfo nextIncludePath = includePaths.at(i + 1);
                if (nextIncludePath.fileName() == frameworkName) {
                    // We got a QtXXX.framework/Headers followed by $QTDIR/include/QtXXX.
                    // In this case we prefer to include files from $QTDIR/include/QtXXX.
                    continue;
                }
            }
        }
        m_includePaths.append(cleanPath(path));
    }
}

void CppPreprocessor::setFrameworkPaths(const QStringList &frameworkPaths)
{
    m_frameworkPaths.clear();

    foreach (const QString &frameworkPath, frameworkPaths) {
        addFrameworkPath(frameworkPath);
    }
}

// Add the given framework path, and expand private frameworks.
//
// Example:
//  <framework-path>/ApplicationServices.framework
// has private frameworks in:
//  <framework-path>/ApplicationServices.framework/Frameworks
// if the "Frameworks" folder exists inside the top level framework.
void CppPreprocessor::addFrameworkPath(const QString &frameworkPath)
{
    // The algorithm below is a bit too eager, but that's because we're not getting
    // in the frameworks we're linking against. If we would have that, then we could
    // add only those private frameworks.
    QString cleanFrameworkPath = cleanPath(frameworkPath);
    if (!m_frameworkPaths.contains(cleanFrameworkPath))
        m_frameworkPaths.append(cleanFrameworkPath);

    const QDir frameworkDir(cleanFrameworkPath);
    const QStringList filter = QStringList() << QLatin1String("*.framework");
    foreach (const QFileInfo &framework, frameworkDir.entryInfoList(filter)) {
        if (!framework.isDir())
            continue;
        const QFileInfo privateFrameworks(framework.absoluteFilePath(), QLatin1String("Frameworks"));
        if (privateFrameworks.exists() && privateFrameworks.isDir())
            addFrameworkPath(privateFrameworks.absoluteFilePath());
    }
}

void CppPreprocessor::setTodo(const QStringList &files)
{ m_todo = QSet<QString>::fromList(files); }

namespace {
class Process: public std::unary_function<Document::Ptr, void>
{
    QPointer<CppModelManager> _modelManager;
    Document::Ptr _doc;
    Document::CheckMode _mode;

public:
    Process(QPointer<CppModelManager> modelManager,
            Document::Ptr doc,
            const CppModelManager::WorkingCopy &workingCopy)
        : _modelManager(modelManager),
          _doc(doc),
          _mode(Document::FastCheck)
    {

        if (workingCopy.contains(_doc->fileName()))
            _mode = Document::FullCheck;
    }

    void operator()()
    {
        _doc->check(_mode);

        if (_modelManager)
            _modelManager->emitDocumentUpdated(_doc);

        _doc->releaseSourceAndAST();
    }
};
} // end of anonymous namespace

void CppPreprocessor::run(const QString &fileName)
{
    QString fn(fileName);
    sourceNeeded(0, fn, IncludeGlobal);
}

void CppPreprocessor::removeFromCache(const QString &fileName)
{
    m_snapshot.remove(fileName);
}

void CppPreprocessor::resetEnvironment()
{
    m_env.reset();
    m_processed.clear();
}

void CppPreprocessor::getFileContents(const QString &absoluteFilePath,
                                      QString *contents,
                                      unsigned *revision) const
{
    if (absoluteFilePath.isEmpty())
        return;

    if (m_workingCopy.contains(absoluteFilePath)) {
        QPair<QString, unsigned> entry = m_workingCopy.get(absoluteFilePath);
        if (contents)
            *contents = entry.first;
        if (revision)
            *revision = entry.second;
        return;
    }

    QFile file(absoluteFilePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        if (contents)
            *contents = stream.readAll();
        if (revision)
            *revision = 0;
        file.close();
    }
}

bool CppPreprocessor::checkFile(const QString &absoluteFilePath) const
{
    if (absoluteFilePath.isEmpty() || m_included.contains(absoluteFilePath))
        return true;

    QFileInfo fileInfo(absoluteFilePath);
    return fileInfo.isFile() && fileInfo.isReadable();
}

/// Resolve the given file name to its absolute path w.r.t. the include type.
QString CppPreprocessor::resolveFile(const QString &fileName, IncludeType type)
{
    if (type == IncludeGlobal) {
        QString fn = m_fileNameCache.value(fileName);

        if (! fn.isEmpty())
            return fn;

        fn = resolveFile_helper(fileName, type);
        m_fileNameCache.insert(fileName, fn);
        return fn;
    }

    // IncludeLocal, IncludeNext
    return resolveFile_helper(fileName, type);
}

QString CppPreprocessor::cleanPath(const QString &path)
{
    QString result = QDir::cleanPath(path);
    const QChar slash(QLatin1Char('/'));
    if (!result.endsWith(slash))
        result.append(slash);
    return result;
}

QString CppPreprocessor::resolveFile_helper(const QString &fileName, IncludeType type)
{
    QFileInfo fileInfo(fileName);
    if (fileName == Preprocessor::configurationFileName || fileInfo.isAbsolute())
        return fileName;

    if (type == IncludeLocal && m_currentDoc) {
        QFileInfo currentFileInfo(m_currentDoc->fileName());
        QString path = cleanPath(currentFileInfo.absolutePath()) + fileName;
        if (checkFile(path))
            return path;
    }

    foreach (const QString &includePath, m_includePaths) {
        QString path = includePath + fileName;
        if (checkFile(path))
            return path;
    }

    int index = fileName.indexOf(QLatin1Char('/'));
    if (index != -1) {
        QString frameworkName = fileName.left(index);
        QString name = frameworkName + QLatin1String(".framework/Headers/") + fileName.mid(index + 1);

        foreach (const QString &frameworkPath, m_frameworkPaths) {
            QString path = frameworkPath + name;
            if (checkFile(path))
                return path;
        }
    }

    //qDebug() << "**** file" << fileName << "not found!";
    return QString();
}

void CppPreprocessor::macroAdded(const Macro &macro)
{
    if (! m_currentDoc)
        return;

    m_currentDoc->appendMacro(macro);
}

static inline const Macro revision(const CppModelManagerInterface::WorkingCopy &s, const Macro &macro)
{
    Macro newMacro(macro);
    newMacro.setFileRevision(s.get(macro.fileName()).second);
    return newMacro;
}

void CppPreprocessor::passedMacroDefinitionCheck(unsigned offset, unsigned line, const Macro &macro)
{
    if (! m_currentDoc)
        return;

    m_currentDoc->addMacroUse(revision(m_workingCopy, macro), offset, macro.name().length(), line,
                              QVector<MacroArgumentReference>());
}

void CppPreprocessor::failedMacroDefinitionCheck(unsigned offset, const ByteArrayRef &name)
{
    if (! m_currentDoc)
        return;

    m_currentDoc->addUndefinedMacroUse(QByteArray(name.start(), name.size()), offset);
}

void CppPreprocessor::notifyMacroReference(unsigned offset, unsigned line, const Macro &macro)
{
    if (! m_currentDoc)
        return;

    m_currentDoc->addMacroUse(revision(m_workingCopy, macro), offset, macro.name().length(), line,
                              QVector<MacroArgumentReference>());
}

void CppPreprocessor::startExpandingMacro(unsigned offset, unsigned line,
                                          const Macro &macro,
                                          const QVector<MacroArgumentReference> &actuals)
{
    if (! m_currentDoc)
        return;

    m_currentDoc->addMacroUse(revision(m_workingCopy, macro), offset, macro.name().length(), line, actuals);
}

void CppPreprocessor::stopExpandingMacro(unsigned, const Macro &)
{
    if (! m_currentDoc)
        return;

    //qDebug() << "stop expanding:" << macro.name;
}

void CppPreprocessor::markAsIncludeGuard(const QByteArray &macroName)
{
    if (!m_currentDoc)
        return;

    m_currentDoc->setIncludeGuardMacroName(macroName);
}

void CppPreprocessor::mergeEnvironment(Document::Ptr doc)
{
    if (! doc)
        return;

    const QString fn = doc->fileName();

    if (m_processed.contains(fn))
        return;

    m_processed.insert(fn);

    foreach (const Document::Include &incl, doc->includes()) {
        QString includedFile = incl.fileName();

        if (Document::Ptr includedDoc = m_snapshot.document(includedFile))
            mergeEnvironment(includedDoc);
        else
            run(includedFile);
    }

    m_env.addMacros(doc->definedMacros());
}

void CppPreprocessor::startSkippingBlocks(unsigned offset)
{
    //qDebug() << "start skipping blocks:" << offset;
    if (m_currentDoc)
        m_currentDoc->startSkippingBlocks(offset);
}

void CppPreprocessor::stopSkippingBlocks(unsigned offset)
{
    //qDebug() << "stop skipping blocks:" << offset;
    if (m_currentDoc)
        m_currentDoc->stopSkippingBlocks(offset);
}

void CppPreprocessor::sourceNeeded(unsigned line, QString &fileName, IncludeType type)
{
    if (fileName.isEmpty())
        return;

    QString absoluteFileName = resolveFile(fileName, type);
    if (m_included.contains(absoluteFileName))
        return; // we've already seen this file.
    m_included.insert(absoluteFileName);

    absoluteFileName = QDir::cleanPath(absoluteFileName);
    unsigned editorRevision = 0;
    QString contents;
    getFileContents(absoluteFileName, &contents, &editorRevision);
    if (m_currentDoc) {
        m_currentDoc->addIncludeFile(absoluteFileName, line);

        if (contents.isEmpty() && ! QFileInfo(absoluteFileName).isAbsolute()) {
            QString msg = QCoreApplication::translate(
                    "CppPreprocessor", "%1: No such file or directory").arg(fileName);

            Document::DiagnosticMessage d(Document::DiagnosticMessage::Warning,
                                          m_currentDoc->fileName(),
                                          line, /*column = */ 0,
                                          msg);

            m_currentDoc->addDiagnosticMessage(d);

            //qWarning() << "file not found:" << fileName << m_currentDoc->fileName() << env.current_line;

            return;
        }
    }

    if (m_dumpFileNameWhileParsing) {
        qDebug() << "Parsing file:" << absoluteFileName
//             << "contents:" << contents.size()
                    ;
    }

    Document::Ptr doc = m_snapshot.document(absoluteFileName);
    if (doc) {
        mergeEnvironment(doc);
        return;
    }

    doc = Document::create(absoluteFileName);
    doc->setRevision(m_revision);
    doc->setEditorRevision(editorRevision);

    QFileInfo info(absoluteFileName);
    if (info.exists())
        doc->setLastModified(info.lastModified());

    Document::Ptr previousDoc = switchDocument(doc);

    const QByteArray preprocessedCode = m_preprocess.run(absoluteFileName, contents);

//    { QByteArray b(preprocessedCode); b.replace("\n", "<<<\n"); qDebug("Preprocessed code for \"%s\": [[%s]]", fileName.toUtf8().constData(), b.constData()); }

    doc->setUtf8Source(preprocessedCode);
    doc->keepSourceAndAST();
    doc->tokenize();

    m_snapshot.insert(doc);
    m_todo.remove(absoluteFileName);

    Process process(m_modelManager, doc, m_workingCopy);
    process();

    (void) switchDocument(previousDoc);
}

Document::Ptr CppPreprocessor::switchDocument(Document::Ptr doc)
{
    Document::Ptr previousDoc = m_currentDoc;
    m_currentDoc = doc;
    return previousDoc;
}

void CppModelManager::updateModifiedSourceFiles()
{
    const Snapshot snapshot = this->snapshot();
    QStringList sourceFiles;

    foreach (const Document::Ptr doc, snapshot) {
        const QDateTime lastModified = doc->lastModified();

        if (! lastModified.isNull()) {
            QFileInfo fileInfo(doc->fileName());

            if (fileInfo.exists() && fileInfo.lastModified() != lastModified)
                sourceFiles.append(doc->fileName());
        }
    }

    updateSourceFiles(sourceFiles);
}

/*!
    \class CppTools::CppModelManager
    \brief The CppModelManager keeps track of one CppCodeModel instance
           for each project and all related CppCodeModelPart instances.

    It also takes care of updating the code models when C++ files are
    modified within Qt Creator.
*/

QMutex CppModelManager::m_modelManagerMutex;
CppModelManager *CppModelManager::m_modelManagerInstance = 0;

CppModelManager *CppModelManager::instance()
{
    if (m_modelManagerInstance)
        return m_modelManagerInstance;
    QMutexLocker locker(&m_modelManagerMutex);
    if (!m_modelManagerInstance)
        m_modelManagerInstance = new CppModelManager;
    return m_modelManagerInstance;
}

CppModelManager::CppModelManager(QObject *parent)
    : CppModelManagerInterface(parent)
      //, m_completionAssistProvider(0)
    , m_indexingSupporter(0)
{
    m_findReferences = new CppFindReferences(this);
    m_indexerEnabled = qgetenv("QTCREATOR_NO_CODE_INDEXER").isNull();

    m_dirty = true;

    qRegisterMetaType<CPlusPlus::Document::Ptr>("CPlusPlus::Document::Ptr");

    // thread connections
    connect(this, SIGNAL(documentUpdated(CPlusPlus::Document::Ptr)),
            this, SLOT(onDocumentUpdated(CPlusPlus::Document::Ptr)));
    connect(this, SIGNAL(extraDiagnosticsUpdated(QString)),
            this, SLOT(onExtraDiagnosticsUpdated(QString)));

    //m_completionFallback = new InternalCompletionAssistProvider;
    //m_completionAssistProvider = m_completionFallback;
    //ExtensionSystem::PluginManager::addObject(m_completionAssistProvider);
    m_internalIndexingSupport = new BuiltinIndexingSupport;
}

CppModelManager::~CppModelManager()
{
    //ExtensionSystem::PluginManager::removeObject(m_completionAssistProvider);
    //delete m_completionFallback;
    delete m_internalIndexingSupport;
}

Snapshot CppModelManager::snapshot() const
{
    QMutexLocker locker(&m_protectSnapshot);
    return m_snapshot;
}

Document::Ptr CppModelManager::document(const QString &fileName) const
{
    QMutexLocker locker(&m_protectSnapshot);
    return m_snapshot.document(fileName);
}

/// Replace the document in the snapshot.
///
/// \returns true if successful, false if the new document is out-dated.
bool CppModelManager::replaceDocument(Document::Ptr newDoc)
{
    QMutexLocker locker(&m_protectSnapshot);

    Document::Ptr previous = m_snapshot.document(newDoc->fileName());
    if (previous && (newDoc->revision() != 0 && newDoc->revision() < previous->revision()))
        // the new document is outdated
        return false;

    m_snapshot.insert(newDoc);
    return true;
}

void CppModelManager::ensureUpdated()
{
    QMutexLocker locker(&m_mutex);
    if (! m_dirty)
        return;

    m_includePaths = internalIncludePaths();
    m_frameworkPaths = internalFrameworkPaths();
    m_definedMacros = internalDefinedMacros();
    m_dirty = false;
}

QStringList CppModelManager::internalIncludePaths() const
{
    QStringList includePaths;
    return includePaths;
}

QStringList CppModelManager::internalFrameworkPaths() const
{
    QStringList frameworkPaths;
    return frameworkPaths;
}

QByteArray CppModelManager::internalDefinedMacros() const
{
    QByteArray macros;
    return macros;
}

/// This method will aquire the mutex!
void CppModelManager::dumpModelManagerConfiguration()
{
}

QList<int> CppModelManager::references(CPlusPlus::Symbol *symbol, const LookupContext &context)
{
    return m_findReferences->references(symbol, context);
}

void CppModelManager::findUsages(CPlusPlus::Symbol *symbol, const CPlusPlus::LookupContext &context)
{
    if (symbol->identifier())
        m_findReferences->findUsages(symbol, context);
}

void CppModelManager::renameUsages(CPlusPlus::Symbol *symbol, const CPlusPlus::LookupContext &context,
                                   const QString &replacement)
{
    if (symbol->identifier())
        m_findReferences->renameUsages(symbol, context, replacement);
}

void CppModelManager::findMacroUsages(const CPlusPlus::Macro &macro)
{
    m_findReferences->findMacroUses(macro);
}

void CppModelManager::renameMacroUsages(const CPlusPlus::Macro &macro, const QString &replacement)
{
    m_findReferences->renameMacroUses(macro, replacement);
}

void CppModelManager::replaceSnapshot(const CPlusPlus::Snapshot &newSnapshot)
{
    QMutexLocker snapshotLocker(&m_protectSnapshot);
    m_snapshot = newSnapshot;
}

CppModelManager::WorkingCopy CppModelManager::buildWorkingCopyList()
{
    WorkingCopy workingCopy;

    // add the project configuration file
    QByteArray conf(pp_configuration);
    conf += definedMacros();
    workingCopy.insert(configurationFileName(), QString::fromUtf8(conf));

    return workingCopy;
}

CppModelManager::WorkingCopy CppModelManager::workingCopy() const
{
    return const_cast<CppModelManager *>(this)->buildWorkingCopyList();
}

QFuture<void> CppModelManager::updateSourceFiles(const QStringList &sourceFiles)
{
    if (sourceFiles.isEmpty() || !m_indexerEnabled)
        return QFuture<void>();

    if (m_indexingSupporter)
        m_indexingSupporter->refreshSourceFiles(sourceFiles);
    return m_internalIndexingSupport->refreshSourceFiles(sourceFiles);
}

void CppModelManager::emitDocumentUpdated(Document::Ptr doc)
{
    emit documentUpdated(doc);
}

void CppModelManager::onDocumentUpdated(Document::Ptr doc)
{
    (void)replaceDocument(doc);
}

void CppModelManager::onExtraDiagnosticsUpdated(const QString &/*fileName*/)
{
}

void CppModelManager::GC()
{
}

void CppModelManager::finishedRefreshingSourceFiles(const QStringList &files)
{
    emit sourceFilesRefreshed(files);
}

/*
CppCompletionSupport *CppModelManager::completionSupport(Core::IEditor *editor) const
{
    if (TextEditor::ITextEditor *textEditor = qobject_cast<TextEditor::ITextEditor *>(editor))
        return m_completionAssistProvider->completionSupport(textEditor);
    else
        return 0;
}

void CppModelManager::setCppCompletionAssistProvider(CppCompletionAssistProvider *completionAssistProvider)
{
    ExtensionSystem::PluginManager::removeObject(m_completionAssistProvider);
    if (completionAssistProvider)
        m_completionAssistProvider = completionAssistProvider;
    else
        m_completionAssistProvider = m_completionFallback;
    ExtensionSystem::PluginManager::addObject(m_completionAssistProvider);
}
*/

void CppModelManager::setIndexingSupport(CppIndexingSupport *indexingSupport)
{
    if (indexingSupport)
        m_indexingSupporter = indexingSupport;
}

CppIndexingSupport *CppModelManager::indexingSupport()
{
    return m_indexingSupporter ? m_indexingSupporter : m_internalIndexingSupport;
}

void CppModelManager::setExtraDiagnostics(const QString &fileName, int kind,
                                          const QList<Document::DiagnosticMessage> &diagnostics)
{
    {
        QMutexLocker locker(&m_protectExtraDiagnostics);
        if (m_extraDiagnostics[fileName][kind] == diagnostics)
            return;
        m_extraDiagnostics[fileName].insert(kind, diagnostics);
    }
    emit extraDiagnosticsUpdated(fileName);
}

QList<Document::DiagnosticMessage> CppModelManager::extraDiagnostics(const QString &fileName, int kind) const
{
    QMutexLocker locker(&m_protectExtraDiagnostics);
    if (kind == -1) {
        QList<Document::DiagnosticMessage> messages;
        foreach (const QList<Document::DiagnosticMessage> &list, m_extraDiagnostics.value(fileName))
            messages += list;
        return messages;
    }
    return m_extraDiagnostics.value(fileName).value(kind);
}
