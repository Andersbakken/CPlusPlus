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

#include "CppDocument.h"
#include "FastPreprocessor.h"

#include <Bind.h>
#include <Control.h>
#include <TranslationUnit.h>
#include <DiagnosticClient.h>
#include <Literals.h>
#include <Symbols.h>
#include <Names.h>
#include <AST.h>
#include <ASTPatternBuilder.h>
#include <ASTMatcher.h>
#include <Scope.h>
#include <SymbolVisitor.h>
#include <NameVisitor.h>
#include <TypeVisitor.h>
#include <CoreTypes.h>

#include <rct/String.h>
#include <rct/Path.h>

#include <assert.h>

/*!
    \namespace CPlusPlus
    The namespace for C++ related tools.
*/

using namespace CPlusPlus;

namespace {

class LastVisibleSymbolAt: protected SymbolVisitor
{
    Symbol *root;
    unsigned line;
    unsigned column;
    Symbol *symbol;

public:
    LastVisibleSymbolAt(Symbol *root)
        : root(root), line(0), column(0), symbol(0) {}

    Symbol *operator()(unsigned line, unsigned column)
    {
        this->line = line;
        this->column = column;
        this->symbol = 0;
        accept(root);
        if (! symbol)
            symbol = root;
        return symbol;
    }

protected:
    bool preVisit(Symbol *s)
    {
        if (s->asBlock()) {
            if (s->line() < line || (s->line() == line && s->column() <= column))
                return true;
            // skip blocks
        } if (s->line() < line || (s->line() == line && s->column() <= column)) {
            symbol = s;
            return true;
        }

        return false;
    }
};

class FindScopeAt: protected SymbolVisitor
{
    TranslationUnit *_unit;
    unsigned _line;
    unsigned _column;
    Scope *_scope;

public:
    /** line and column should be 1-based */
    FindScopeAt(TranslationUnit *unit, unsigned line, unsigned column)
        : _unit(unit), _line(line), _column(column), _scope(0) {}

    Scope *operator()(Symbol *symbol)
    {
        accept(symbol);
        return _scope;
    }

protected:
    bool process(Scope *symbol)
    {
        if (! _scope) {
            Scope *scope = symbol;

            for (unsigned i = 0; i < scope->memberCount(); ++i) {
                accept(scope->memberAt(i));

                if (_scope)
                    return false;
            }

            unsigned startLine, startColumn;
            _unit->getPosition(scope->startOffset(), &startLine, &startColumn);

            if (_line > startLine || (_line == startLine && _column >= startColumn)) {
                unsigned endLine, endColumn;
                _unit->getPosition(scope->endOffset(), &endLine, &endColumn);

                if (_line < endLine || (_line == endLine && _column < endColumn))
                    _scope = scope;
            }
        }

        return false;
    }

    using SymbolVisitor::visit;

    virtual bool preVisit(Symbol *)
    { return ! _scope; }

    virtual bool visit(UsingNamespaceDirective *) { return false; }
    virtual bool visit(UsingDeclaration *) { return false; }
    virtual bool visit(NamespaceAlias *) { return false; }
    virtual bool visit(Declaration *) { return false; }
    virtual bool visit(Argument *) { return false; }
    virtual bool visit(TypenameArgument *) { return false; }
    virtual bool visit(BaseClass *) { return false; }
    virtual bool visit(ForwardClassDeclaration *) { return false; }

    virtual bool visit(Enum *symbol)
    { return process(symbol); }

    virtual bool visit(Function *symbol)
    { return process(symbol); }

    virtual bool visit(Namespace *symbol)
    { return process(symbol); }

    virtual bool visit(Class *symbol)
    { return process(symbol); }

    virtual bool visit(Block *symbol)
    { return process(symbol); }

    // Objective-C
    virtual bool visit(ObjCBaseClass *) { return false; }
    virtual bool visit(ObjCBaseProtocol *) { return false; }
    virtual bool visit(ObjCForwardClassDeclaration *) { return false; }
    virtual bool visit(ObjCForwardProtocolDeclaration *) { return false; }
    virtual bool visit(ObjCPropertyDeclaration *) { return false; }

    virtual bool visit(ObjCClass *symbol)
    { return process(symbol); }

    virtual bool visit(ObjCProtocol *symbol)
    { return process(symbol); }

    virtual bool visit(ObjCMethod *symbol)
    { return process(symbol); }
};


#define DO_NOT_DUMP_ALL_PARSER_ERRORS

class DocumentDiagnosticClient : public DiagnosticClient
{
    enum { MAX_MESSAGE_COUNT = 10 };

public:
    DocumentDiagnosticClient(Document *doc, List<Document::DiagnosticMessage> *messages)
        : doc(doc),
          messages(messages),
          errorCount(0)
    { }

    virtual void report(int level,
                        const StringLiteral *fileId,
                        unsigned line, unsigned column,
                        const char *format, va_list ap)
    {
        if (level == Error) {
            ++errorCount;

#ifdef DO_NOT_DUMP_ALL_PARSER_ERRORS
            if (errorCount >= MAX_MESSAGE_COUNT)
                return; // ignore the error
#endif // DO_NOT_DUMP_ALL_PARSER_ERRORS
        }

        const Path fileName(fileId->chars(), fileId->size());

        if (fileName != doc->fileName())
            return;

        String message = String::format<512>(format, ap);

#ifndef DO_NOT_DUMP_ALL_PARSER_ERRORS
        {
            const char *levelStr = "Unknown level";
            if (level == Document::DiagnosticMessage::Warning) levelStr = "Warning";
            if (level == Document::DiagnosticMessage::Error) levelStr = "Error";
            if (level == Document::DiagnosticMessage::Fatal) levelStr = "Fatal";
            qDebug("%s:%u:%u: %s: %s", fileId->chars(), line, column, levelStr, message.toUtf8().constData());
        }
#endif // DO_NOT_DUMP_ALL_PARSER_ERRORS

        Document::DiagnosticMessage m(convertLevel(level), doc->fileName(),
                                      line, column, message);
        messages->append(m);
    }

    static int convertLevel(int level) {
        switch (level) {
            case Warning: return Document::DiagnosticMessage::Warning;
            case Error:   return Document::DiagnosticMessage::Error;
            case Fatal:   return Document::DiagnosticMessage::Fatal;
            default:      return Document::DiagnosticMessage::Error;
        }
    }

private:
    Document *doc;
    List<Document::DiagnosticMessage> *messages;
    int errorCount;
};

} // anonymous namespace


Document::Document(const Path &fileName)
    : _fileName(Path::resolved(fileName)),
      _globalNamespace(0),
      _keepSourceAndASTCount(0),
      _checkMode(0)
{
    _control = new Control();

    _control->setDiagnosticClient(new DocumentDiagnosticClient(this, &_diagnosticMessages));

    const StringLiteral *fileId = _control->stringLiteral(fileName.constData(),
                                                          fileName.size());
    _translationUnit = new TranslationUnit(_control, fileId);
    _translationUnit->setQtMocRunEnabled(true);
    _translationUnit->setCxxOxEnabled(true);
    _translationUnit->setObjCEnabled(true);
    (void) _control->switchTranslationUnit(_translationUnit);
}

Document::~Document()
{
    delete _translationUnit;
    delete _control->diagnosticClient();
    delete _control;
}

Control *Document::control() const
{
    return _control;
}

/*
QDateTime Document::lastModified() const
{
    return _lastModified;
}

void Document::setLastModified(const QDateTime &lastModified)
{
    _lastModified = lastModified;
}
*/

Path Document::fileName() const
{
    return _fileName;
}

List<String> Document::includedFiles() const
{
    List<String> files;
    for (int i = 0; i < _includes.size(); ++i)
        files.append(_includes.at(i).fileName());
    //files.removeDuplicates();
#warning not done
    return files;
}

// This assumes to be called with a QDir::cleanPath cleaned fileName.
void Document::addIncludeFile(const String &fileName, unsigned line)
{
    _includes.append(Include(fileName, line));
}

void Document::appendMacro(const Macro &macro)
{
    _definedMacros.append(macro);
}

void Document::addMacroUse(const Macro &macro, unsigned offset, unsigned length,
                           unsigned beginLine,
                           const List<MacroArgumentReference> &actuals)
{
    MacroUse use(macro, offset, offset + length, beginLine);

    for (int i = 0; i < actuals.size(); ++i) {
        const MacroArgumentReference& actual = actuals.at(i);
        const Block arg(actual.position(), actual.position() + actual.length());

        use.addArgument(arg);
    }

    _macroUses.append(use);
}

void Document::addUndefinedMacroUse(const String &name, unsigned offset)
{
    String copy(name.data(), name.size());
    UndefinedMacroUse use(copy, offset);
    _undefinedMacroUses.append(use);
}

/*!
    \class Document::MacroUse
    \brief Represents the usage of a macro in a \l {Document}.
    \sa Document::UndefinedMacroUse
*/

/*!
    \class Document::UndefinedMacroUse
    \brief Represents a macro that was looked up, but not found.

    Holds data about the reference to a macro in an \tt{#ifdef} or \tt{#ifndef}
    or argument to the \tt{defined} operator inside an \tt{#if} or \tt{#elif} that does
    not exist.

    \sa Document::undefinedMacroUses(), Document::MacroUse, Macro
*/

/*!
    \fn String Document::UndefinedMacroUse::name() const

    Returns the name of the macro that was not found.
*/

/*!
    \fn List<UndefinedMacroUse> Document::undefinedMacroUses() const

    Returns a list of referenced but undefined macros.

    \sa Document::macroUses(), Document::definedMacros(), Macro
*/

/*!
    \fn List<MacroUse> Document::macroUses() const

    Returns a list of macros used.

    \sa Document::undefinedMacroUses(), Document::definedMacros(), Macro
*/

/*!
    \fn List<Macro> Document::definedMacros() const

    Returns the list of macros defined.

    \sa Document::macroUses(), Document::undefinedMacroUses()
*/

TranslationUnit *Document::translationUnit() const
{
    return _translationUnit;
}

bool Document::skipFunctionBody() const
{
    return _translationUnit->skipFunctionBody();
}

void Document::setSkipFunctionBody(bool skipFunctionBody)
{
    _translationUnit->setSkipFunctionBody(skipFunctionBody);
}

unsigned Document::globalSymbolCount() const
{
    if (! _globalNamespace)
        return 0;

    return _globalNamespace->memberCount();
}

Symbol *Document::globalSymbolAt(unsigned index) const
{
    return _globalNamespace->memberAt(index);
}

Namespace *Document::globalNamespace() const
{
    return _globalNamespace;
}

void Document::setGlobalNamespace(Namespace *globalNamespace)
{
    _globalNamespace = globalNamespace;
}

Scope *Document::scopeAt(unsigned line, unsigned column)
{
    FindScopeAt findScopeAt(_translationUnit, line, column);
    if (Scope *scope = findScopeAt(_globalNamespace))
        return scope;
    return globalNamespace();
}

Symbol *Document::lastVisibleSymbolAt(unsigned line, unsigned column) const
{
    LastVisibleSymbolAt lastVisibleSymbolAt(globalNamespace());
    return lastVisibleSymbolAt(line, column);
}

const Macro *Document::findMacroDefinitionAt(unsigned line) const
{
    for (int i = 0; i < _definedMacros.size(); ++i) {
        if (_definedMacros.at(i).line() == line)
            return &_definedMacros.at(i);
    }
    return 0;
}

const Document::MacroUse *Document::findMacroUseAt(unsigned offset) const
{
    for (int i = 0; i < _macroUses.size(); ++i) {
        const Document::MacroUse& use = _macroUses.at(i);
        if (use.contains(offset) && (offset < use.begin() + use.macro().name().length()))
            return &use;
    }
    return 0;
}

const Document::UndefinedMacroUse *Document::findUndefinedMacroUseAt(unsigned offset) const
{
    for (int i = 0; i < _undefinedMacroUses.size(); ++i) {
        const Document::UndefinedMacroUse& use = _undefinedMacroUses.at(i);
        if (use.contains(offset) && (offset < use.begin() + use.name().length()))
            return &use;
    }
    return 0;
}

Document::Ptr Document::create(const String &fileName)
{
    Document::Ptr doc(new Document(fileName));
    return doc;
}

String Document::utf8Source() const
{ return _source; }

void Document::setUtf8Source(const String &source)
{
    _source = source;
    _translationUnit->setSource(_source.constData(), _source.size());
}

void Document::startSkippingBlocks(unsigned start)
{
    _skippedBlocks.append(Block(start, 0));
}

void Document::stopSkippingBlocks(unsigned stop)
{
    if (_skippedBlocks.isEmpty())
        return;

    unsigned start = _skippedBlocks.back().begin();
    if (start > stop)
        _skippedBlocks.removeLast(); // Ignore this block, it's invalid.
    else
        _skippedBlocks.back() = Block(start, stop);
}

bool Document::isTokenized() const
{
    return _translationUnit->isTokenized();
}

void Document::tokenize()
{
    _translationUnit->tokenize();
}

bool Document::isParsed() const
{
    return _translationUnit->isParsed();
}

bool Document::parse(ParseMode mode)
{
    TranslationUnit::ParseMode m = TranslationUnit::ParseTranlationUnit;
    switch (mode) {
    case ParseTranlationUnit:
        m = TranslationUnit::ParseTranlationUnit;
        break;

    case ParseDeclaration:
        m = TranslationUnit::ParseDeclaration;
        break;

    case ParseExpression:
        m = TranslationUnit::ParseExpression;
        break;

    case ParseDeclarator:
        m = TranslationUnit::ParseDeclarator;
        break;

    case ParseStatement:
        m = TranslationUnit::ParseStatement;
        break;

    default:
        break;
    }

    return _translationUnit->parse(m);
}

void Document::check(CheckMode mode)
{
    assert(!_globalNamespace);

    _checkMode = mode;

    if (! isParsed())
        parse();

    _globalNamespace = _control->newNamespace(0);
    Bind semantic(_translationUnit);
    if (mode == FastCheck)
        semantic.setSkipFunctionBodies(true);

    if (! _translationUnit->ast())
        return; // nothing to do.

    if (TranslationUnitAST *ast = _translationUnit->ast()->asTranslationUnit())
        semantic(ast, _globalNamespace);
    else if (StatementAST *ast = _translationUnit->ast()->asStatement())
        semantic(ast, _globalNamespace);
    else if (ExpressionAST *ast = _translationUnit->ast()->asExpression())
        semantic(ast, _globalNamespace);
    else if (DeclarationAST *ast = translationUnit()->ast()->asDeclaration())
        semantic(ast, _globalNamespace);
}

void Document::keepSourceAndAST()
{
#warning not done
    ++_keepSourceAndASTCount;
}

void Document::releaseSourceAndAST()
{
    if (!--_keepSourceAndASTCount) {
        _source.clear();
        _translationUnit->release();
        _control->squeeze();
    }
}

bool Document::DiagnosticMessage::operator==(const Document::DiagnosticMessage &other) const
{
    return
            _line == other._line &&
            _column == other._column &&
            _length == other._length &&
            _level == other._level &&
            _fileName == other._fileName &&
            _text == other._text;
}

bool Document::DiagnosticMessage::operator!=(const Document::DiagnosticMessage &other) const
{
    return !operator==(other);
}

Snapshot::Snapshot()
{
}

Snapshot::~Snapshot()
{
}

int Snapshot::size() const
{
    return _documents.size();
}

bool Snapshot::isEmpty() const
{
    return _documents.isEmpty();
}

Snapshot::const_iterator Snapshot::find(const String &fileName) const
{
    return _documents.find(fileName);
}

void Snapshot::remove(const String &fileName)
{
    _documents.remove(fileName);
}

bool Snapshot::contains(const String &fileName) const
{
    return _documents.contains(fileName);
}

void Snapshot::insert(Document::Ptr doc)
{
    if (doc)
        _documents[doc->fileName()] = doc;
}

Document::Ptr Snapshot::preprocessedDocument(const String &source, const String &fileName) const
{
    Document::Ptr newDoc = Document::create(fileName);
    if (Document::Ptr thisDocument = document(fileName)) {
        //newDoc->_lastModified = thisDocument->_lastModified;
        newDoc->_includes = thisDocument->_includes;
    }

    FastPreprocessor pp(*this);
    const String preprocessedCode = pp.run(newDoc, source);
    newDoc->setUtf8Source(preprocessedCode);
    return newDoc;
}

Document::Ptr Snapshot::documentFromSource(const String &preprocessedCode,
                                           const String &fileName) const
{
    Document::Ptr newDoc = Document::create(fileName);

    if (Document::Ptr thisDocument = document(fileName)) {
        //newDoc->_lastModified = thisDocument->_lastModified;
        newDoc->_includes = thisDocument->_includes;
        newDoc->_definedMacros = thisDocument->_definedMacros;
        newDoc->_macroUses = thisDocument->_macroUses;
    }

    newDoc->setUtf8Source(preprocessedCode);
    return newDoc;
}

Document::Ptr Snapshot::document(const String &fileName) const
{
    return _documents.value(fileName);
}

Snapshot Snapshot::simplified(Document::Ptr doc) const
{
    Snapshot snapshot;
    simplified_helper(doc, &snapshot);
    return snapshot;
}

void Snapshot::simplified_helper(Document::Ptr doc, Snapshot *snapshot) const
{
    if (! doc)
        return;

    if (! snapshot->contains(doc->fileName())) {
        snapshot->insert(doc);

        const List<Document::Include>& includes = doc->includes();
        for (int i = 0; i < includes.size(); ++i) {
            const Document::Include& incl = includes.at(i);
            Document::Ptr includedDoc = document(incl.fileName());
            simplified_helper(includedDoc, snapshot);
        }
    }
}

