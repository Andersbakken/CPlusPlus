#include <QStack>
#include <cppmodelmanager.h>
#include <LookupContext.h>
#include <FindUsages.h>
#include <ASTPath.h>
#include <QStringList>
#include <QElapsedTimer>
#include <assert.h>
#include <stdio.h>

using namespace CPlusPlus;
using namespace CppTools::Internal;

class ReallyFindScopeAt: protected SymbolVisitor
{
    TranslationUnit *_unit;
    unsigned _line;
    unsigned _column;
    Scope *_scope;
    unsigned _foundStart;
    unsigned _foundEnd;

public:
    /** line and column should be 1-based */
    ReallyFindScopeAt(TranslationUnit *unit, unsigned line, unsigned column)
        : _unit(unit), _line(line), _column(column), _scope(0),
          _foundStart(0), _foundEnd(0)
    {
    }

    Scope *operator()(Symbol *symbol)
    {
        accept(symbol);
        return _scope;
    }

protected:
    bool process(Scope *symbol)
    {
        Scope *scope = symbol;

        for (unsigned i = 0; i < scope->memberCount(); ++i) {
            accept(scope->memberAt(i));
        }

        unsigned startLine, startColumn;
        _unit->getPosition(scope->startOffset(), &startLine, &startColumn);

        if (_line > startLine || (_line == startLine && _column >= startColumn)) {
            unsigned endLine, endColumn;
            _unit->getPosition(scope->endOffset(), &endLine, &endColumn);

            if (_line < endLine || (_line == endLine && _column < endColumn)) {
                if (!_scope || (scope->startOffset() >= _foundStart &&
                                scope->endOffset() <= _foundEnd)) {
                    _foundStart = scope->startOffset();
                    _foundEnd = scope->endOffset();
                    _scope = scope;
                }
            }
        }

        return false;
    }

    using SymbolVisitor::visit;

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

class Visitor : public SymbolVisitor
{
public:
    Visitor(QPointer<CppModelManager> mgr, LookupContext& ctx)
        : SymbolVisitor(), symbolCount(0), refCount(0), useCount(0), manager(mgr), lookup(ctx)
    {}

    enum Type {
        Type_UsingNamespaceDirective,
        Type_UsingDeclaration,
        Type_NamespaceAlias,
        Type_Declaration,
        Type_Argument,
        Type_TypenameArgument,
        Type_BaseClass,
        Type_Enum,
        Type_Function,
        Type_Namespace,
        Type_Template,
        Type_Class,
        Type_Block,
        Type_ForwardClassDeclaration,
        Type_QtPropertyDeclaration,
        Type_QtEnum,
        Type_ObjCBaseClass,
        Type_ObjCBaseProtocol,
        Type_ObjCClass,
        Type_ObjCForwardClassDeclaration,
        Type_ObjCProtocol,
        Type_ObjCForwardProtocolDeclaration,
        Type_ObjCMethod,
        Type_ObjCPropertyDeclaration
    };

    bool visitSymbol(Type type, Symbol* symbol);

    virtual bool visit(UsingNamespaceDirective *symbol) { return visitSymbol(Type_UsingNamespaceDirective, symbol); }
    virtual bool visit(UsingDeclaration *symbol) { return visitSymbol(Type_UsingDeclaration, symbol); }
    virtual bool visit(NamespaceAlias *symbol) { return visitSymbol(Type_NamespaceAlias, symbol); }
    virtual bool visit(Declaration *symbol) { return visitSymbol(Type_Declaration, symbol); }
    virtual bool visit(Argument *symbol) { return visitSymbol(Type_Argument, symbol); }
    virtual bool visit(TypenameArgument *symbol) { return visitSymbol(Type_TypenameArgument, symbol); }
    virtual bool visit(BaseClass *symbol) { return visitSymbol(Type_BaseClass, symbol); }
    virtual bool visit(Enum *symbol) { return visitSymbol(Type_Enum, symbol); }
    virtual bool visit(Function *symbol) { return visitSymbol(Type_Function, symbol); }
    virtual bool visit(Namespace *symbol) { return visitSymbol(Type_Namespace, symbol); }
    virtual bool visit(Template *symbol) { return visitSymbol(Type_Template, symbol); }
    virtual bool visit(Class *symbol) { return visitSymbol(Type_Class, symbol); }
    virtual bool visit(Block *symbol) { return visitSymbol(Type_Block, symbol); }
    virtual bool visit(ForwardClassDeclaration *symbol) { return visitSymbol(Type_ForwardClassDeclaration, symbol); }

    // Qt
    virtual bool visit(QtPropertyDeclaration *symbol) { return visitSymbol(Type_QtPropertyDeclaration, symbol); }
    virtual bool visit(QtEnum *symbol) { return visitSymbol(Type_QtEnum, symbol); }

    // Objective-C
    virtual bool visit(ObjCBaseClass *symbol) { return visitSymbol(Type_ObjCBaseClass, symbol); }
    virtual bool visit(ObjCBaseProtocol *symbol) { return visitSymbol(Type_ObjCBaseProtocol, symbol); }
    virtual bool visit(ObjCClass *symbol) { return visitSymbol(Type_ObjCClass, symbol); }
    virtual bool visit(ObjCForwardClassDeclaration *symbol) { return visitSymbol(Type_ObjCForwardClassDeclaration, symbol); }
    virtual bool visit(ObjCProtocol *symbol) { return visitSymbol(Type_ObjCProtocol, symbol); }
    virtual bool visit(ObjCForwardProtocolDeclaration *symbol) { return visitSymbol(Type_ObjCForwardProtocolDeclaration, symbol); }
    virtual bool visit(ObjCMethod *symbol) { return visitSymbol(Type_ObjCMethod, symbol); }
    virtual bool visit(ObjCPropertyDeclaration *symbol) { return visitSymbol(Type_ObjCPropertyDeclaration, symbol); }

    int symbolCount;
    int refCount;
    int useCount;
    QPointer<CppModelManager> manager;
    LookupContext& lookup;
};

inline bool Visitor::visitSymbol(Type type, Symbol* symbol)
{
    if (!symbol)
        return false;

    qDebug("visiting %d", type);

    const Identifier* id = symbol->identifier();
    if (!id)
        return true;

    qDebug("  name %s", id->chars());

    return true;
}

static Symbol *canonicalSymbol(Scope *scope, const QString &code, TypeOfExpression &typeOfExpression)
{
    const QList<LookupItem> results =
        typeOfExpression(code.toUtf8(), scope, TypeOfExpression::Preprocess);

    for (int i = results.size() - 1; i != -1; --i) {
        const LookupItem &r = results.at(i);
        Symbol *decl = r.declaration();

        if (! (decl && decl->enclosingScope()))
            break;

        if (Class *classScope = r.declaration()->enclosingScope()->asClass()) {
            const Identifier *declId = decl->identifier();
            const Identifier *classId = classScope->identifier();

            if (classId && classId->isEqualTo(declId))
                continue; // skip it, it's a ctor or a dtor.

            else if (Function *funTy = r.declaration()->type()->asFunctionType()) {
                if (funTy->isVirtual())
                    return r.declaration();
            }
        }
    }

    for (int i = 0; i < results.size(); ++i) {
        const LookupItem &r = results.at(i);

        if (r.declaration())
            return r.declaration();
    }

    return 0;
}

class DocumentParser : public QObject
{
    Q_OBJECT
public:
    DocumentParser(QPointer<CppModelManager> mgr, QObject* parent = 0)
        : QObject(parent), symbolCount(0), manager(mgr)
    {
    }
    ~DocumentParser()
    {
        const Snapshot& snapshot = manager->snapshot();
        Snapshot::iterator it = snapshot.begin();
        const Snapshot::const_iterator end = snapshot.end();
        while (it != end) {
            it.value()->releaseSourceAndAST();
            ++it;
        }
    }

    QByteArray tokenForAst(AST* ast, TranslationUnit* unit, const QByteArray& src)
    {
        const Token& start = unit->tokenAt(ast->firstToken());
        const Token& last = unit->tokenAt(ast->lastToken() - 1);
        return src.mid(start.begin(), last.end() - start.begin());
    }

    QByteArray debugScope(Scope* scope, const QByteArray& src)
    {
        return src.mid(scope->startOffset(), scope->endOffset() - scope->startOffset());
    }

    QList<Usage> findUsages(Symbol* symbol, const QByteArray& unpreprocessedSource)
    {
        const Identifier *symbolId = symbol->identifier();
        const Snapshot& snapshot = manager->snapshot();

        Document::Ptr doc = snapshot.preprocessedDocument(unpreprocessedSource, symbol->fileName());
        doc->tokenize();

        QList<Usage> usages;

        Control *control = doc->control();
        if (control->findIdentifier(symbolId->chars(), symbolId->size()) != 0) {
            //doc->setGlobalNamespace(0);
            //doc->check();
            doc->check();

            FindUsages process(unpreprocessedSource, doc, snapshot);
            process(symbol);

            usages = process.usages();
        }

        return usages;
    }

    void processInput(char* line)
    {
        char* save;
        char* ret = strtok_r(line, ":", &save);

        QString fn;
        unsigned l, c, cnt = 0;

        while (ret) {
            // process token
            switch (cnt++) {
            case 0:
                fn = QString::fromUtf8(ret);
                break;
            case 1:
            case 2: {
                unsigned* x = (cnt == 2) ? &l : &c;
                *x = atoi(ret);
                break; }
            }
            ret = strtok_r(0, ":", &save);
        }

        if (cnt != 3) {
            qWarning("Invalid input %s", line);
            return;
        }

        qDebug("processing %s:%d:%d", qPrintable(fn), l, c);
        Document::Ptr doc = manager->document(fn);
        if (!doc) {
            qWarning("No document for %s", qPrintable(fn));
            return;
        }

        TranslationUnit *translationUnit = doc->translationUnit();
        printf("men... %p\n", doc->globalNamespace());
        LookupContext lookup(doc, manager->snapshot());

        TypeOfExpression typeofExpression;
        typeofExpression.init(doc, manager->snapshot(), lookup.bindings());

        printf("men... 2 %p\n", doc->globalNamespace());
        ReallyFindScopeAt really(translationUnit, l, c);
        Scope* scope = really(doc->globalNamespace());
        if (!scope) {
            qWarning("no scope at %d:%d", l, c);
            return;
        }
        //Scope* scope = doc->scopeAt(l, c);

        printf("men... 3 %p\n", doc->globalNamespace());
        QByteArray src = doc->utf8Source();

        ASTPath path(doc);
        QList<AST*> asts = path(l, c);
        qDebug("asts cnt %d", asts.size());
        /*
          foreach(AST* ast, asts) {
          qDebug("ast %p (expr %p) token %s", ast, ast->asExpression(), qPrintable(tokenForAst(ast, translationUnit, src)));
          }
        */
        if (asts.isEmpty()) {
            qWarning("no ast at %d:%d", l, c);
            return;
        }

        Symbol* decl = 0;
        for (int i = asts.size() - 1; i >= 0; --i) {
            typeofExpression.setExpandTemplates(true);
            QByteArray expression = tokenForAst(asts.at(i), translationUnit, src);
            decl = canonicalSymbol(scope, expression, typeofExpression);
            if (decl)
                break;
        }
        if (decl) {
            qDebug() << "finding refs";

            printf("men... 4 %p\n", doc->globalNamespace());
            const QList<Usage> usages = findUsages(decl, src);
            qDebug() << "found refs" << usages.size();
            foreach(const Usage& usage, usages) {
                qDebug("usage %s:%d:%d (%s)", qPrintable(usage.path), usage.line, usage.col, qPrintable(usage.lineText));

                Document::Ptr doc = manager->document(usage.path);
                if (doc) {
                    Symbol* refsym = doc->lastVisibleSymbolAt(usage.line, usage.col + 1);
                    if (refsym) {
                        if (refsym->line() == static_cast<unsigned>(usage.line) &&
                            refsym->column() == static_cast<unsigned>(usage.col + 1)) {
                            qDebug() << "it's a symbol!";
                        }
                    }
                }
            }
        } else {
            // try to find the symbol outright
            Symbol* sym = doc->lastVisibleSymbolAt(l, c);
            if (sym) {
                const Identifier* id = sym->identifier();
                if (id) {
                    // ### fryktelig
                    if (sym->line() == l && sym->column() <= c &&
                        sym->column() + id->size() >= c) {
                        // yes
                        qDebug("found symbol outright %s at %s:%d:%d", id->chars(), sym->fileName(), sym->line(), sym->column());;

                        qDebug() << "finding refs";

                        const QList<Usage> usages = findUsages(sym, src);
                        foreach(const Usage& usage, usages) {
                            qDebug("usage %s:%d:%d (%s)", qPrintable(usage.path), usage.line, usage.col, qPrintable(usage.lineText));
                        }

                        return;
                    }
                }
                // no
            }
        }
    }

private slots:
    void onDocumentUpdated(CPlusPlus::Document::Ptr doc)
    {
        // seems I need to keep this around
        doc->keepSourceAndAST();
        const QFileInfo info(doc->fileName());
        const QString canonical = info.canonicalFilePath();

        // if (seen.contains(canonical))
        //     return;
        // seen.insert(canonical);

        //QElapsedTimer timer;
        //timer.start();
        //LookupContext lookup(doc, manager->snapshot());
        TranslationUnit *translationUnit = doc->translationUnit();

        Parser parser(translationUnit);

        Namespace *globalNamespace = doc->globalNamespace();

        Bind bind(translationUnit);

        if (!translationUnit->ast())
            return; // nothing to do.

        if (TranslationUnitAST *ast = translationUnit->ast()->asTranslationUnit())
            bind(ast, globalNamespace);
        else if (StatementAST *ast = translationUnit->ast()->asStatement())
            bind(ast, globalNamespace);
        else if (ExpressionAST *ast = translationUnit->ast()->asExpression())
            bind(ast, globalNamespace);
        else if (DeclarationAST *ast = translationUnit->ast()->asDeclaration())
            bind(ast, globalNamespace);
        //bind(ast, globalNamespace);
        //doc->check();
        //Visitor visitor(manager, lookup);
        //visitor.accept(globalNamespace);
        //qDebug("accepted %s %d", qPrintable(doc->fileName()), visitor.symbolCount);
        qDebug("bound %s", qPrintable(doc->fileName()));
        //symbolCount += visitor.symbolCount;
    }

public:
    int symbolCount;
    QPointer<CppModelManager> manager;
    QSet<QString> seen;
};

#include "main.moc"

int main(int /*argc*/, char** /*argv*/)
{
    QList<QString> inputFiles;
    QPointer<CppModelManager> manager(new CppModelManager);

    DocumentParser parser(manager);
    QObject::connect(manager.data(), SIGNAL(documentUpdated(CPlusPlus::Document::Ptr)),
                     &parser, SLOT(onDocumentUpdated(CPlusPlus::Document::Ptr)), Qt::DirectConnection);

    QElapsedTimer timer;
    timer.start();

    QFile f("/tmp/foo");
    f.open(QIODevice::ReadOnly);
    CppPreprocessor preprocessor(manager);
    int count = 0;
    while (!f.atEnd()) {
        QStringList stuff = QString(f.readLine()).split(" ");
        if (!stuff.first().endsWith("++") && !stuff.first().endsWith("cc")) 
            continue;            
        QStringList incs = stuff.filter(QRegExp("^-I"));
        for (int i=0; i<incs.size(); ++i) {
            incs[i].remove(0, 2);
        }
        QStringList defs = stuff.filter(QRegExp("^-D"));
        for (int i=0; i<defs.size(); ++i) {
            defs[i].remove(0, 2);
        }
        incs << "/usr/include/" << "." << "/usr/include/c++/4.6";

        preprocessor.setIncludePaths(incs);
        preprocessor.addDefinitions(defs);
        stuff.last().chop(1);
        preprocessor.run(stuff.last());
        // qDebug() << incs << defs << stuff.last();
        preprocessor.resetEnvironment();
        // break;
        ++count;
    }

    printf("time %lld syms %d files %d\n",
           timer.elapsed(), parser.symbolCount, count);

    for (;;) {
        char* line = 0;
        size_t size;
        ssize_t r = getline(&line, &size, stdin);
        if (r == -1)
            break;
        if (size) {
            assert(r > 1);
            if (!strncmp(line, "quit\n", 5)) {
                free(line);
                break;
            }
            assert(line[r - 1] == '\n');
            // remove the newline
            line[r - 1] = '\0';
            parser.processInput(line);
            free(line);
        }
    }

    return 0;
}
