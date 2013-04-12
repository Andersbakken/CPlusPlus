#include <QStack>
#include <cppmodelmanager.h>
#include <LookupContext.h>
#include <FindUsages.h>
#include <QStringList>
#include <QElapsedTimer>
#include <assert.h>

using namespace CPlusPlus;
using namespace CppTools::Internal;

int fisk = 0;

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

    virtual void postVisit(Symbol* symbol);

    struct CursorInfo
    {
        CursorInfo(Type t, Symbol* sym, LookupContext& /*lookup*/)
            : type(t), symbol(sym), target(0)
        {
            //static QSet<int> seen;
            
            // fullyQualifiedName = lookup.fullyQualifiedName(symbol);
            /*
            const Identifier* id = sym->identifier();
            assert(id);
            printf("type %d %u id %s (%s:%d:%d)%s\n", type, sym->hashCode(), id->chars(),
                   sym->fileName(), sym->line(), sym->column(), seen.contains(sym->hashCode()) ? " seen" : "");
            seen.insert(sym->hashCode());
            */
        }

        Type type;
        Symbol* symbol;
        QList<const Name*> fullyQualifiedName;
        QList<CursorInfo*> references;
        CursorInfo* target;
    };

    typedef QHash<int, CursorInfo*> SymbolHash;
    typedef QHash<Scope*, SymbolHash> ScopeHash;
    // QMap<QByteArray, CursorInfo*> mSymbolNames;

    ScopeHash::iterator findScope(Scope* scope);

    int symbolCount;
    int refCount;
    int useCount;
    QPointer<CppModelManager> manager;
    LookupContext& lookup;

    ScopeHash symbols;
};

void Visitor::postVisit(Symbol* symbol)
{
    if (!symbol)
        return;
    Scope* scope = symbol->asScope();
    if (!scope)
        return;
    symbols.remove(scope);
}

inline bool Visitor::visitSymbol(Type type, Symbol* symbol)
{
    if (!symbol)
        return false;

    const Identifier* id = symbol->identifier();
    if (!id)
        return true;

    const unsigned symbolHash = symbol->hashCode();

    CursorInfo* info = new CursorInfo(type, symbol, lookup);
    static int iii = 0;
    if (++iii % 10000 && 0) {
        QElapsedTimer timer;
        timer.start();
        FindUsages find(lookup);
        find(symbol);
        qDebug() << "Found" << find.usages().size() << timer.elapsed() << id->chars();
    }
    symbols[symbol->enclosingScope()][symbolHash] = info;

    ++symbolCount;
    // } else {
    //     CursorInfo* decl = 0;
    //     Scope* scope = symbol->enclosingScope();
    //     while (scope) {
    //         ScopeHash::const_iterator s = symbols.find(scope);
    //         if (s != symbols.end()) {
    //             SymbolHash::const_iterator sym = s.value().find(symbolHash);
    //             if (sym != s.value().end() && compareFullyQualifiedName(sym.value()->fullyQualifiedName, info->fullyQualifiedName)) {
    //                 decl = sym.value();
    //                 break;
    //             }
    //         }
    //         scope = scope->enclosingScope();
    //     }
    //     qWarning("is not decl name %s type %d loc %s:%d:%d", id->chars(), type, symbol->fileName(), symbol->line(), symbol->column());
    //     if (!decl) {
    //         qWarning("no decl found");
    //         delete info;
    //     } else {
    //         qWarning("found decl");
    //         info->target = decl;
    //         decl->references.append(info);
    //     }
    // }

    return true;
}

class ReferenceVisitor : public ASTVisitor
{
public:
    ReferenceVisitor(TranslationUnit* unit, const Document::Ptr& doc, LookupContext& ctx)
        : ASTVisitor(unit), mDoc(doc), mSrc(doc->utf8Source()), mLookup(ctx)
    {
    }

    virtual bool visit(SimpleNameAST */*ast*/)
    {
        /*
        const Token& startTok = tokenAt(ast->firstToken());
        const Token& endTok = tokenAt(ast->lastToken() - 1);
        qDebug() << mSrc.mid(startTok.begin(), endTok.end() - startTok.begin());
        */
        //const QList<LookupItem> candidates = mLookup.lookup(ast->name, mScope);
        return true;
    }

    virtual bool preVisit(AST */*ast*/)
    {
        return true;
    }

private:
    QStack<Scope*> scopes;
    const Document::Ptr mDoc;
    const QByteArray mSrc;
    LookupContext& mLookup;
};

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
    }

private slots:
    void onDocumentUpdated(CPlusPlus::Document::Ptr doc)
    {
        const QFileInfo info(doc->fileName());
        const QString canonical = info.canonicalFilePath();

        // if (seen.contains(canonical))
        //     return;
        // seen.insert(canonical);

        QElapsedTimer timer;
        timer.start();
        LookupContext lookup(doc, manager->snapshot());
        TranslationUnit *translationUnit = doc->translationUnit();

        Parser parser(translationUnit);

        TranslationUnitAST *ast = 0;
        Namespace *globalNamespace = doc->globalNamespace();

        if (parser.parseTranslationUnit(ast)) {
            Bind bind(translationUnit);
            bind(ast, globalNamespace);
            Visitor visitor(manager, lookup);
            visitor.accept(globalNamespace);
            qDebug("accepted %s %d", qPrintable(doc->fileName()), visitor.symbolCount);
            symbolCount += visitor.symbolCount;
            if (visitor.symbolCount)
                ++fisk;
            // ReferenceVisitor refVisitor(translationUnit, doc, lookup);
            // refVisitor.accept(ast);
            // visit += timer.restart();

            /*
            if (visitor.symbolCount)
                qDebug("file '%s', symbols: %d, refs %d", qPrintable(doc->fileName()),
                       visitor.symbolCount, visitor.useCount);
            */
        }
    }

public:
    int symbolCount;
    QPointer<CppModelManager> manager;
    QSet<QString> seen;
};

#include "main.moc"

int main(int argc, char** argv)
{
    QList<QString> inputFiles;
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
                inputFiles.append(file);
                qDebug("Using '%s' as input", argv[i]);
            }
        }
    }

    // if (inputFiles.isEmpty()) {
    //     qFatal("No input files");
    // }

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

    printf("time %lld syms %d files %d fisk %d\n",
           timer.elapsed(), parser.symbolCount, count, fisk);

    ++argc;

    return 0;
}
