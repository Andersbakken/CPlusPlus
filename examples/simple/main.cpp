#include <cppmodelmanager.h>
#include <LookupContext.h>
#include <QStringList>

using namespace CPlusPlus;
using namespace CppTools::Internal;

class Visitor : public ASTVisitor
{
public:
    Visitor(TranslationUnit* unit, QPointer<CppModelManager> mgr, LookupContext& ctx)
        : ASTVisitor(unit), symbolCount(0), refCount(0), manager(mgr), lookup(ctx)
    {}
    bool visitSymbol(Symbol* symbol);
    virtual bool visit(BaseSpecifierAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CatchClauseAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ClassSpecifierAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CompoundStatementAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(EnumSpecifierAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ForStatementAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ForeachStatementAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(FunctionDeclaratorAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(FunctionDefinitionAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(IfStatementAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(NamespaceAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ObjCFastEnumerationAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ObjCMethodPrototypeAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(ObjCProtocolDeclarationAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(SwitchStatementAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(TemplateDeclarationAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(TemplateTypeParameterAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(TypenameTypeParameterAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(UsingAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(UsingDirectiveAST* ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(WhileStatementAST* ast) { return visitSymbol(ast->symbol); }

    int symbolCount;
    int refCount;
    QPointer<CppModelManager> manager;
    LookupContext& lookup;
};

inline bool Visitor::visitSymbol(Symbol* symbol)
{
    ++symbolCount;
    QList<int> refs = manager->references(symbol, lookup);
    refCount += refs.size();
    return true;
}

class DocumentParser : public QObject
{
    Q_OBJECT
public:
    DocumentParser(QPointer<CppModelManager> mgr, QObject* parent = 0)
        : QObject(parent), manager(mgr)
    {
    }
    ~DocumentParser()
    {
    }

private slots:
    void onDocumentUpdated(CPlusPlus::Document::Ptr doc)
    {
        LookupContext lookup(doc, manager->snapshot());
        TranslationUnit *translationUnit = doc->translationUnit();

        Parser parser(translationUnit);

        TranslationUnitAST *ast = 0;
        Namespace *globalNamespace = doc->globalNamespace();

        if (parser.parseTranslationUnit(ast)) {
            Bind bind(translationUnit);
            bind(ast, globalNamespace);
            Visitor visitor(translationUnit, manager, lookup);
            visitor.accept(ast);

            if (visitor.symbolCount)
                qDebug("file '%s', symbols: %d, refs %d", qPrintable(doc->fileName()),
                       visitor.symbolCount, visitor.refCount);
        }
    }

public:
    QPointer<CppModelManager> manager;
};

#include "main.moc"

int main(int argc, char** argv)
{
    QString inputFile;
    QStringList incs, defs;
    int arglen;
    for (int i = 1; i < argc; ++i) {
        if (!strncmp(argv[i], "-I", 2)) {
            arglen = strlen(argv[i]);
            if (arglen == 2) {
                if (i + 1 < argc) {
                    incs << QString::fromUtf8(argv[i + 1]);
                    ++i;
                } else {
                    qFatal("Missing include");
                }
            } else {
                incs << QString::fromUtf8(argv[i] + 2, arglen - 2);
            }
        } else if (!strncmp(argv[i], "-D", 2)) {
            arglen = strlen(argv[i]);
            if (arglen == 2) {
                if (i + 1 < argc) {
                    defs << QString::fromUtf8(argv[i + 1]);
                    ++i;
                } else {
                    qFatal("Missing include");
                }
            } else {
                defs << QString::fromUtf8(argv[i] + 2, arglen - 2);
            }
        } else {
            // assume input
            if (!inputFile.isEmpty()) {
                qFatal("Already have an input file, new '%s', old '%s'", argv[i], qPrintable(inputFile));
            }
            inputFile = QString::fromUtf8(argv[i]);
            qDebug("Using '%s' as input", argv[i]);
        }
    }

    if (inputFile.isEmpty()) {
        qFatal("No input file");
    }

    QPointer<CppModelManager> manager(new CppModelManager);

    DocumentParser parser(manager);
    QObject::connect(manager.data(), SIGNAL(documentUpdated(CPlusPlus::Document::Ptr)),
                     &parser, SLOT(onDocumentUpdated(CPlusPlus::Document::Ptr)), Qt::DirectConnection);

    CppPreprocessor preprocessor(manager);
    preprocessor.setIncludePaths(incs);
    preprocessor.addDefinitions(defs);
    preprocessor.run(inputFile);

    return 0;
}
