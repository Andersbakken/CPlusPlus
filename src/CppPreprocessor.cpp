#include "CppPreprocessor.h"
#include <fnmatch.h>
#include <utility>

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

CppPreprocessor::CppPreprocessor(const Path &pwd)
    : mPreprocess(this, &mEnv), mPwd(pwd)
{
    mPreprocess.setKeepComments(true);
}

static inline bool isMac()
{
#ifdef OS_Darwin
    return true;
#else
    return false;
#endif
}

CppPreprocessor::~CppPreprocessor()
{ }

void CppPreprocessor::setIncludePaths(const List<Path> &includePaths)
{
    mIncludePaths.clear();

    for (int i = 0; i < includePaths.size(); ++i) {
        const Path path = includePaths.at(i);

        // if (isMac()) {
        //     if (i + 1 < includePaths.size() && path.endsWith(".framework/Headers")) {
        //         Path framework = pathInfo.parentDir();
        //         const QFileInfo frameworkFileInfo(pathInfo.path());
        //         const String frameworkName = frameworkFileInfo.baseName();

        //         const QFileInfo nextIncludePath = includePaths.at(i + 1);
        //         if (nextIncludePath.fileName() == frameworkName) {
        //             // We got a QtXXX.framework/Headers followed by $QTDIR/include/QtXXX.
        //             // In this case we prefer to include files from $QTDIR/include/QtXXX.
        //             continue;
        //         }
        //     }
        // }
        mIncludePaths.append(cleanPath(path));
    }
}

void CppPreprocessor::setFrameworkPaths(const List<Path> &frameworkPaths)
{
    mFrameworkPaths.clear();

    for (int i=0; i<frameworkPaths.size(); ++i) {
        addFrameworkPath(frameworkPaths.at(i));
    }
}

struct VisitData {
    List<Path> paths;
    const char *filter;
};

Path::VisitResult pathVisitor(const Path &path, void *userData)
{
    if (path.isDir()) {
        VisitData &data = *reinterpret_cast<VisitData*>(userData);
        if (!data.filter || !fnmatch(path.constData(), data.filter, 0)) {
            // path.paths
        }

    }
    return Path::Continue;
}

List<Path> files(const Path &dir, const char *match)
{
    VisitData data;
    data.filter = match;
    dir.visit(&pathVisitor, &data);
    return data.paths;
}

// Add the given framework path, and expand private frameworks.
//
// Example:
//  <framework-path>/ApplicationServices.framework
// has private frameworks in:
//  <framework-path>/ApplicationServices.framework/Frameworks
// if the "Frameworks" folder exists inside the top level framework.
void CppPreprocessor::addFrameworkPath(const Path &frameworkPath)
{
// #warning not done
    // The algorithm below is a bit too eager, but that's because we're not getting
    // in the frameworks we're linking against. If we would have that, then we could
    // add only those private frameworks.
    // String cleanFrameworkPath = cleanPath(frameworkPath);
    // if (!mFrameworkPaths.contains(cleanFrameworkPath))
    //     mFrameworkPaths.append(cleanFrameworkPath);

    // const QDir frameworkDir(cleanFrameworkPath);
    // const List<Path> filter = List<Path>() << "*.framework";
    // foreach (const QFileInfo &framework, frameworkDir.entryInfoList(filter)) {
    //     if (!framework.isDir())
    //         continue;
    //     const QFileInfo privateFrameworks(framework.absoluteFilePath(), "Frameworks");
    //     if (privateFrameworks.exists() && privateFrameworks.isDir())
    //         addFrameworkPath(privateFrameworks.absoluteFilePath());
    // }
}

void CppPreprocessor::setTodo(const Set<Path> &files)
{
    mTodo = files;
}

void CppPreprocessor::run(const Path &fileName)
{
    Path absoluteFilePath = fileName;
    sourceNeeded(0, absoluteFilePath, IncludeGlobal);
}

void CppPreprocessor::removeFromCache(const Path &fileName)
{
    // mSnapshot.remove(fileName);
}

void CppPreprocessor::resetEnvironment()
{
    mEnv.reset();
    mProcessed.clear();
}

bool CppPreprocessor::includeFile(const Path &absoluteFilePath, String *result)
{
    if (absoluteFilePath.isEmpty() || mIncluded.contains(absoluteFilePath))
        return true;
    if (!absoluteFilePath.isFile())
        return false;

    // if (mWorkingCopy.contains(absoluteFilePath)) {
    //     mIncluded.insert(absoluteFilePath);
    //     const std::pair<Path, unsigned> r = mWorkingCopy.get(absoluteFilePath);
    //     *result = r.first;
    //     return true;
    // }

    *result = absoluteFilePath.readAll();
    mIncluded.insert(absoluteFilePath);
    return true;
}

String CppPreprocessor::tryIncludeFile(Path &fileName, IncludeType type)
{
    if (type == IncludeGlobal) {
        const Path fn = mFileNameCache.value(fileName);

        if (!fn.isEmpty()) {
            fileName = fn;

            return String();
        }

        const Path originalFileName = fileName;
        const String contents = tryIncludeFile_helper(fileName, type);
        mFileNameCache[originalFileName] = fileName;
        return contents;
    }

    // IncludeLocal, IncludeNext
    return tryIncludeFile_helper(fileName, type);
}

Path CppPreprocessor::cleanPath(const Path &path)
{
    Path ret = Path::resolved(path);
    if (!ret.endsWith('/'))
        ret.append('/');
    return ret;
}

String CppPreprocessor::tryIncludeFile_helper(Path &fileName, IncludeType type)
{
    if (fileName == CPlusPlus::Preprocessor::configurationFileName || fileName.isAbsolute()) {
        String contents;
        includeFile(fileName, &contents);
        return contents;
    }

    if (type == IncludeLocal) {
        const Path path = fileName.resolved(mPwd);
        String contents;
        if (includeFile(path, &contents)) {
            fileName = path;
            return contents;
        }
    }

    for (int i=0; i<mIncludePaths.size(); ++i) {
        const Path path = mIncludePaths.at(i) + fileName;
        String contents;
        if (includeFile(path, &contents)) {
            fileName = path;
            return contents;
        }
    }

    int index = fileName.indexOf('/');
    if (index != -1) {
        String frameworkName = fileName.left(index);
        String name = frameworkName + ".framework/Headers/" + fileName.mid(index + 1);

        for (int i=0; i<mFrameworkPaths.size(); ++i) {
            const Path path = mFrameworkPaths.at(i) + name;
            String contents;
            if (includeFile(path, &contents)) {
                fileName = path;
                return contents;
            }
        }
    }

    //qDebug() << "**** file" << fileName << "not found!";
    return String();
}

void CppPreprocessor::macroAdded(const CPlusPlus::Macro &macro)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->appendMacro(macro);
}

void CppPreprocessor::passedMacroDefinitionCheck(unsigned offset, unsigned line, const CPlusPlus::Macro &macro)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->addMacroUse(revision(mWorkingCopy, macro), offset, macro.name().length(), line,
    //                           List<MacroArgumentReference>());
}

void CppPreprocessor::failedMacroDefinitionCheck(unsigned offset, const CPlusPlus::StringRef &name)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->addUndefinedMacroUse(QByteArray(name.start(), name.size()), offset);
}

void CppPreprocessor::notifyMacroReference(unsigned offset, unsigned line, const CPlusPlus::Macro &macro)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->addMacroUse(revision(mWorkingCopy, macro), offset, macro.name().length(), line,
    //                           List<MacroArgumentReference>());
}

void CppPreprocessor::startExpandingMacro(unsigned offset, unsigned line,
                                          const CPlusPlus::Macro &macro,
                                          const List<CPlusPlus::MacroArgumentReference> &actuals)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->addMacroUse(revision(mWorkingCopy, macro), offset, macro.name().length(), line, actuals);
}

void CppPreprocessor::stopExpandingMacro(unsigned, const CPlusPlus::Macro &)
{
    // if (!mCurrentDoc)
    //     return;

    //qDebug() << "stop expanding:" << macro.name;
}

void CppPreprocessor::markAsIncludeGuard(const String &macroName)
{
    // if (!mCurrentDoc)
    //     return;

    // mCurrentDoc->setIncludeGuardMacroName(macroName);
}

// void CppPreprocessor::mergeEnvironment(Document::Ptr doc)
// {
//     if (!doc)
//         return;

//     const Path fn = doc->fileName();

//     if (mProcessed.contains(fn))
//         return;

//     mProcessed.insert(fn);

//     foreach (const Document::Include &incl, doc->includes()) {
//         String includedFile = incl.fileName();

//         if (Document::Ptr includedDoc = mSnapshot.document(includedFile))
//             mergeEnvironment(includedDoc);
//         else
//             run(includedFile);
//     }

//     mEnv.addMacros(doc->definedMacros());
// }

void CppPreprocessor::startSkippingBlocks(unsigned offset)
{
    //qDebug() << "start skipping blocks:" << offset;
    // if (mCurrentDoc)
    //     mCurrentDoc->startSkippingBlocks(offset);
}

void CppPreprocessor::stopSkippingBlocks(unsigned offset)
{
    //qDebug() << "stop skipping blocks:" << offset;
    // if (mCurrentDoc)
    //     mCurrentDoc->stopSkippingBlocks(offset);
}

void CppPreprocessor::sourceNeeded(unsigned line, Path &fileName, IncludeType type)
{
    // if (fileName.isEmpty())
    //     return;

    // unsigned editorRevision = 0;
    // String contents = tryIncludeFile(fileName, type, &editorRevision);
    // fileName = QDir::cleanPath(fileName);
    // if (mCurrentDoc) {
    //     mCurrentDoc->addIncludeFile(fileName, line);

    //     if (contents.isEmpty() && ! QFileInfo(fileName).isAbsolute()) {
    //         String msg = QCoreApplication::translate(
    //             "CppPreprocessor", "%1: No such file or directory").arg(fileName);

    //         Document::DiagnosticMessage d(Document::DiagnosticMessage::Warning,
    //                                       mCurrentDoc->fileName(),
    //                                       line, /*column = */ 0,
    //                                       msg);

    //         mCurrentDoc->addDiagnosticMessage(d);

    //         //qWarning() << "file not found:" << fileName << mCurrentDoc->fileName() << env.current_line;
    //     }
    // }

    // if (mDumpFileNameWhileParsing) {
    //     qDebug() << "Parsing file:" << fileName
    //         //             << "contents:" << contents.size()
    //         ;
    // }

    // // Document::Ptr doc = mSnapshot.document(fileName);
    // // if (doc) {
    // //     mergeEnvironment(doc);
    // //     return;
    // // }

    // doc = Document::create(fileName);
    // doc->setRevision(mRevision);
    // doc->setEditorRevision(editorRevision);

    // QFileInfo info(fileName);
    // if (info.exists())
    //     doc->setLastModified(info.lastModified());

    // Document::Ptr previousDoc = switchDocument(doc);

    // const QByteArray preprocessedCode = mPreprocess.run(fileName, contents);

    // //    { QByteArray b(preprocessedCode); b.replace("\n", "<<<\n"); qDebug("Preprocessed code for \"%s\": [[%s]]", fileName.toUtf8().constData(), b.constData()); }

    // doc->setUtf8Source(preprocessedCode);
    // doc->keepSourceAndAST();
    // doc->tokenize();

    // mSnapshot.insert(doc);
    // mTodo.remove(fileName);

    // Process process(mModelManager, doc, mWorkingCopy);
    // process();

    // (void) switchDocument(previousDoc);
}
