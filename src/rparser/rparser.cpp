#include "rparser.h"
#include <assert.h>
#include <PreprocessorClient.h>
#include <CppDocument.h>
#include <Bind.h>
#include <Control.h>
#include <Symbols.h>
#include <Literals.h>
#include <QMap>

struct RParserPrivate
{
    RParserPrivate(RParser *parser) : rparser(parser), preprocessor(0) {}
    ~RParserPrivate() { delete preprocessor; }

    inline uint32_t fileId(const QString &file)
    {
        uint32_t ret = fileIds.value(file, INT_MAX);
        if (ret == INT_MAX) {
            // resolve paths here?
            ret = rparser->fileId(file.toLocal8Bit().constData());
            fileIds[file] = ret;
        }
        return ret;
    }

    RParser *rparser;
    CPlusPlus::Snapshot snapshot;
    CPlusPlus::Client *preprocessor;
    QMap<QString, uint32_t> fileIds;


    // CppPreprocessor *preprocessor;
};

class Visitor : public CPlusPlus::ASTVisitor
{
public:
    Visitor(CPlusPlus::TranslationUnit *unit, RParserPrivate *priv, uint32_t fileId)
        : CPlusPlus::ASTVisitor(unit), mPrivate(priv), mFileId(fileId)
    {}
    bool visitSymbol(CPlusPlus::Symbol */*symbol*/) { return true; }
    virtual bool visit(CPlusPlus::BaseSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::CatchClauseAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ClassSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::CompoundStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::EnumSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ForStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ForeachStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::FunctionDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::FunctionDefinitionAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::IfStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::NamespaceAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCFastEnumerationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCMethodPrototypeAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCProtocolDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::SwitchStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TemplateDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TemplateTypeParameterAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TypenameTypeParameterAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::UsingAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::UsingDirectiveAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::WhileStatementAST *ast) { return visitSymbol(ast->symbol); }
    
private:
    RParserPrivate *mPrivate;
    const uint32_t mFileId;
};

RParser::RParser()
    : mData(new RParserPrivate(this))
{
    // List<Path> includes;
    // includes << "cplusplus" << "pp" << "src" << "doc" << "rct/src";
    // preprocessor.setIncludePaths(includes);

    // Path main = "src/main.cpp";

    // preprocessor.run(main);

    // CPlusPlus::Snapshot::const_iterator doc = snapshot.begin();
    // const CPlusPlus::Snapshot::const_iterator end = snapshot.end();
    // while (doc != end) {
    //     error() << "Found document" << doc->first;
    //     ++doc;
    // }

    // main.resolve();

    // CPlusPlus::Document::Ptr mainDoc = snapshot.document(main);
    // if (mainDoc) {
    //     CPlusPlus::TranslationUnit* tu = mainDoc->translationUnit();
    //     error("Found main.cpp: %p", tu);
    // }
}

RParser::~RParser()
{
    delete mData;
}

bool RParser::parse(const std::string &/* sourceFile */,
                    const std::vector<std::string> &/* includePaths */,
                    const std::vector<std::string> &/* defines */)
{
    assert(!mData->preprocessor);
    // mData->preprocessor = new
    // mData->preprocessor->run(sourceFile);


    return false;
}

void RParser::visit()
{
    assert(mData->preprocessor);
    CPlusPlus::Control control;
    CPlusPlus::Namespace *globalNamespace = control.newNamespace(0);

    // const CPlusPlus::StringLiteral *fileId = control.stringLiteral(file, strlen(file));
    // CPlusPlus::TranslationUnit translationUnit(&control, fileId);
    // translationUnit.setQtMocRunEnabled(true);
    // translationUnit.setCxxOxEnabled(true);
    // translationUnit.setObjCEnabled(true);
    // control.switchTranslationUnit(&translationUnit);
    // const String contents = mSourceInformation.sourceFile.readAll();
    // translationUnit.setSource(contents.constData(), contents.size());
    // translationUnit.tokenize();
    // CPlusPlus::Parser parser(&translationUnit);
    // CPlusPlus::TranslationUnitAST *ast = 0;

    // if (!parser.parseTranslationUnit(ast))
    //     return false;
    // int elapsed = watch.elapsed();
    // error() << elapsed;
    // Visitor visitor(&translationUnit, this);
    // visitor.accept(ast);
    // return true;
    CPlusPlus::Snapshot::const_iterator doc = mData->snapshot.begin();
    const CPlusPlus::Snapshot::const_iterator end = mData->snapshot.end();


    while (doc != end) {
        const uint32_t fd = mData->fileId(doc.key());
        if (fd) {
            CPlusPlus::TranslationUnit *translationUnit = doc.value()->translationUnit();
            control.switchTranslationUnit(translationUnit);
            CPlusPlus::Parser parser(translationUnit);
            CPlusPlus::TranslationUnitAST *ast = 0;

            if (parser.parseTranslationUnit(ast)) {
                CPlusPlus::Bind bind(translationUnit);
                bind(ast, globalNamespace);
                Visitor visitor(translationUnit, mData, fd);
                visitor.accept(ast);
            }
        }
        ++doc;
    }
}
