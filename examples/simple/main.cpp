#include <cppmodelmanager.h>
#include <LookupContext.h>
#include <FindUsages.h>
#include <QStringList>
#include <QElapsedTimer>

using namespace CPlusPlus;
using namespace CppTools::Internal;

int bind = 0;
int visit = 0;
int parse = 0;
int findRef = 0;
class Visitor : public ASTVisitor
{
public:
    Visitor(TranslationUnit* unit, QPointer<CppModelManager> mgr, LookupContext& ctx)
        : ASTVisitor(unit), symbolCount(0), refCount(0), useCount(0), manager(mgr), lookup(ctx)
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
    int useCount;
    QPointer<CppModelManager> manager;
    LookupContext& lookup;
};

inline bool Visitor::visitSymbol(Symbol* symbol)
{
    ++symbolCount;

    QElapsedTimer timer;
    timer.start();
    FindUsages findUsages(lookup);
    findUsages(symbol);

    //QList<int> refs = findUsages.references();
    QList<Usage> usages = findUsages.usages();
    findRef += timer.elapsed();

    //refCount += refs.size();
    useCount += usages.size();
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
        QElapsedTimer timer;
        timer.start();
        LookupContext lookup(doc, manager->snapshot());
        TranslationUnit *translationUnit = doc->translationUnit();

        Parser parser(translationUnit);

        TranslationUnitAST *ast = 0;
        Namespace *globalNamespace = doc->globalNamespace();

        if (parser.parseTranslationUnit(ast)) {
            parse += timer.restart();
            Bind bind(translationUnit);
            bind(ast, globalNamespace);
            ::bind += timer.restart();
            Visitor visitor(translationUnit, manager, lookup);
            visitor.accept(ast);
            visit += timer.restart();

            if (visitor.symbolCount)
                qDebug("file '%s', symbols: %d, refs %d", qPrintable(doc->fileName()),
                       visitor.symbolCount, visitor.useCount);
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
        } else if (i > 0 && strcmp("-o", argv[i - 1])) {
            const QString file = QString::fromUtf8(argv[i]);
            if (QFile::exists(file)) {
                // assume input
                if (!inputFile.isEmpty()) {
                    qFatal("Already have an input file, new '%s', old '%s'", argv[i], qPrintable(inputFile));
                }
                inputFile = file;
                qDebug("Using '%s' as input", argv[i]);
            }
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

    printf("bind %d visit %d parse %d findRef %d\n",
           bind, visit, parse, findRef);

    return 0;
}
