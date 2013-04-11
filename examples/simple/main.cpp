#include <cppmodelmanager.h>
#include <LookupContext.h>
#include <FindUsages.h>
#include <QStringList>
#include <QElapsedTimer>
#include <assert.h>

using namespace CPlusPlus;
using namespace CppTools::Internal;

int bind = 0;
int visit = 0;
int parse = 0;
int findRef = 0;

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

static inline QByteArray fullyQualifiedNameString(Symbol* symbol, LookupContext& lookup)
{
    const QList<const Name*> name = lookup.fullyQualifiedName(symbol);

    QByteArray ret;
    QList<const Name*>::const_iterator it = name.begin();
    const QList<const Name*>::const_iterator end = name.end();
    while (it != end) {
        const Identifier* id = (*it)->identifier();
        if (!id) {
            qDebug() << "so far" << ret << (it + 1 == end) << symbol->fileName() << symbol->line() << symbol->column();
            if ((*it)->isNameId())
                qDebug() << "isNameId";
            if ((*it)->isTemplateNameId())
                qDebug() << "isTemplateNameId";
            if ((*it)->isDestructorNameId())
                qDebug() << "isDestructorNameId";
            if ((*it)->isOperatorNameId())
                qDebug() << "isOperatorNameId";
            if ((*it)->isConversionNameId())
                qDebug() << "isConversionNameId";
            if ((*it)->isQualifiedNameId())
                qDebug() << "isQualifiedNameId";
            if ((*it)->isSelectorNameId())
                qDebug() << "isSelectorNameId";
            assert(id);
            return QByteArray();
        }
        ret += id->chars();
        ret += "::";
        ++it;
    }
    return ret;
}

inline bool Visitor::visitSymbol(Type type, Symbol* symbol)
{
    if (!symbol)
        return false;

    ++symbolCount;

    qDebug() << "fullyQualifiedNameString" << fullyQualifiedNameString(symbol, lookup);

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
            Visitor visitor(manager, lookup);
            visitor.accept(globalNamespace);
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
