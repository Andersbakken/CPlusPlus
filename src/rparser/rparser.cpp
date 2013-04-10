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
    // virtual bool visit(CPlusPlus::AccessDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::AliasDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::AlignofExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ArrayAccessAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ArrayDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ArrayInitializerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::AsmDefinitionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::AttributeAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::AttributeSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::BaseSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::BinaryExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::BoolLiteralAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::BracedInitializerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::BreakStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CallAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CaptureAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CaseStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CastExpressionAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::CatchClauseAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ClassSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CompoundExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CompoundLiteralAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::CompoundStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ConditionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ConditionalExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ContinueStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ConversionFunctionIdAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CppCastExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::CtorInitializerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DeclarationStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DeclaratorIdAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DecltypeSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DeleteExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DestructorNameAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DoStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::DynamicExceptionSpecificationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ElaboratedTypeSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::EmptyDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::EnumSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::EnumeratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ExceptionDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ExpressionListParenAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ExpressionOrDeclarationStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ExpressionStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ForStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ForeachStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::FunctionDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::FunctionDefinitionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::GotoStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::IdExpressionAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::IfStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LabeledStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LambdaCaptureAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LambdaDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LambdaExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LambdaIntroducerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LinkageBodyAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::LinkageSpecificationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::MemInitializerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::MemberAccessAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NamedTypeSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::NamespaceAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NamespaceAliasDefinitionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NestedDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NestedExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NestedNameSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NewArrayDeclaratorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NewExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NewTypeIdAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NoExceptSpecificationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::NumericLiteralAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCClassDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCClassForwardDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCDynamicPropertiesDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCEncodeExpressionAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCFastEnumerationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCInstanceVariablesDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCMessageArgumentAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCMessageArgumentDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCMessageExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCMethodDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCMethodPrototypeAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCPropertyAttributeAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCPropertyDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::ObjCProtocolDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCProtocolExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCProtocolForwardDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCProtocolRefsAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSelectorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSelectorArgumentAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSelectorExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSynchronizedStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSynthesizedPropertiesDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCSynthesizedPropertyAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCTypeNameAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ObjCVisibilityDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::OperatorAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::OperatorFunctionIdAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ParameterDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ParameterDeclarationClauseAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::PointerAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::PointerLiteralAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::PointerToMemberAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::PostIncrDecrAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtEnumDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtFlagsDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtInterfaceNameAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtInterfacesDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtMemberDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtMethodAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtObjectTagAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtPrivateSlotAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtPropertyDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QtPropertyDeclarationItemAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::QualifiedNameAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::RangeBasedForStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ReferenceAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ReturnStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::SimpleDeclarationAST *ast) { return visitSymbol(ast->symbol); } // this one has a list
    // virtual bool visit(CPlusPlus::SimpleNameAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::SimpleSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::SizeofExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::StaticAssertDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::StringLiteralAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::SwitchStatementAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TemplateDeclarationAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TemplateIdAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TemplateTypeParameterAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ThisExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::ThrowExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TrailingReturnTypeAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TranslationUnitAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TryBlockStatementAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TypeConstructorCallAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TypeIdAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TypeidExpressionAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TypenameCallExpressionAST *ast) { return visitSymbol(ast->symbol); }
    virtual bool visit(CPlusPlus::TypenameTypeParameterAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::TypeofSpecifierAST *ast) { return visitSymbol(ast->symbol); }
    // virtual bool visit(CPlusPlus::UnaryExpressionAST *ast) { return visitSymbol(ast->symbol); }
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
