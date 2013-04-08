#include <Parser.h>
#include <TranslationUnit.h>
#include <ASTVisitor.h>
#include <AST.h>
#include <Control.h>
#include <string.h>
#include <stdio.h>

class Visitor : public CPlusPlus::ASTVisitor
{
public:
    Visitor(CPlusPlus::TranslationUnit *unit)
        : CPlusPlus::ASTVisitor(unit), mIndent(0)
    {}
    virtual bool preVisit(CPlusPlus::AST *ast)
    {
        ++mIndent;
        dump(ast);
        return true;
    }
    
    virtual void postVisit(CPlusPlus::AST *ast)
    {
        --mIndent;
    }
    void dump(CPlusPlus::AST *ast)
    {
        for (int i=0; i<mIndent; ++i) {
            printf("  ");
        }

        for (int i=ast->firstToken(); i<=ast->lastToken(); ++i) {
            printf("%s ", spell(i));
        }
        // printf("\n");
        // return;

        if (ast->asAccessDeclaration()) {
            printf("asAccessDeclaration\n");
            return;
        }
        if (ast->asAliasDeclaration()) {
            printf("asAliasDeclaration\n");
            return;
        }
        if (ast->asAlignofExpression()) {
            printf("asAlignofExpression\n");
            return;
        }
        if (ast->asArrayAccess()) {
            printf("asArrayAccess\n");
            return;
        }
        if (ast->asArrayDeclarator()) {
            printf("asArrayDeclarator\n");
            return;
        }
        if (ast->asArrayInitializer()) {
            printf("asArrayInitializer\n");
            return;
        }
        if (ast->asAsmDefinition()) {
            printf("asAsmDefinition\n");
            return;
        }
        if (ast->asAttribute()) {
            printf("asAttribute\n");
            return;
        }
        if (ast->asAttributeSpecifier()) {
            printf("asAttributeSpecifier\n");
            return;
        }
        if (ast->asBaseSpecifier()) {
            printf("asBaseSpecifier\n");
            return;
        }
        if (ast->asBinaryExpression()) {
            printf("asBinaryExpression\n");
            return;
        }
        if (ast->asBoolLiteral()) {
            printf("asBoolLiteral\n");
            return;
        }
        if (ast->asBracedInitializer()) {
            printf("asBracedInitializer\n");
            return;
        }
        if (ast->asBreakStatement()) {
            printf("asBreakStatement\n");
            return;
        }
        if (ast->asCall()) {
            printf("asCall\n");
            return;
        }
        if (ast->asCapture()) {
            printf("asCapture\n");
            return;
        }
        if (ast->asCaseStatement()) {
            printf("asCaseStatement\n");
            return;
        }
        if (ast->asCastExpression()) {
            printf("asCastExpression\n");
            return;
        }
        if (ast->asCatchClause()) {
            printf("asCatchClause\n");
            return;
        }
        if (ast->asClassSpecifier()) {
            printf("asClassSpecifier\n");
            return;
        }
        if (ast->asCompoundExpression()) {
            printf("asCompoundExpression\n");
            return;
        }
        if (ast->asCompoundLiteral()) {
            printf("asCompoundLiteral\n");
            return;
        }
        if (ast->asCompoundStatement()) {
            printf("asCompoundStatement\n");
            return;
        }
        if (ast->asCondition()) {
            printf("asCondition\n");
            return;
        }
        if (ast->asConditionalExpression()) {
            printf("asConditionalExpression\n");
            return;
        }
        if (ast->asContinueStatement()) {
            printf("asContinueStatement\n");
            return;
        }
        if (ast->asConversionFunctionId()) {
            printf("asConversionFunctionId\n");
            return;
        }
        if (ast->asCoreDeclarator()) {
            printf("asCoreDeclarator\n");
            return;
        }
        if (ast->asCppCastExpression()) {
            printf("asCppCastExpression\n");
            return;
        }
        if (ast->asCtorInitializer()) {
            printf("asCtorInitializer\n");
            return;
        }
        if (ast->asDeclaration()) {
            printf("asDeclaration\n");
            return;
        }
        if (ast->asDeclarationStatement()) {
            printf("asDeclarationStatement\n");
            return;
        }
        if (ast->asDeclarator()) {
            printf("asDeclarator\n");
            return;
        }
        if (ast->asDeclaratorId()) {
            printf("asDeclaratorId\n");
            return;
        }
        if (ast->asDecltypeSpecifier()) {
            printf("asDecltypeSpecifier\n");
            return;
        }
        if (ast->asDeleteExpression()) {
            printf("asDeleteExpression\n");
            return;
        }
        if (ast->asDestructorName()) {
            printf("asDestructorName\n");
            return;
        }
        if (ast->asDoStatement()) {
            printf("asDoStatement\n");
            return;
        }
        if (ast->asDynamicExceptionSpecification()) {
            printf("asDynamicExceptionSpecification\n");
            return;
        }
        if (ast->asElaboratedTypeSpecifier()) {
            printf("asElaboratedTypeSpecifier\n");
            return;
        }
        if (ast->asEmptyDeclaration()) {
            printf("asEmptyDeclaration\n");
            return;
        }
        if (ast->asEnumSpecifier()) {
            printf("asEnumSpecifier\n");
            return;
        }
        if (ast->asEnumerator()) {
            printf("asEnumerator\n");
            return;
        }
        if (ast->asExceptionDeclaration()) {
            printf("asExceptionDeclaration\n");
            return;
        }
        if (ast->asExceptionSpecification()) {
            printf("asExceptionSpecification\n");
            return;
        }
        if (ast->asExpression()) {
            printf("asExpression\n");
            return;
        }
        if (ast->asExpressionListParen()) {
            printf("asExpressionListParen\n");
            return;
        }
        if (ast->asExpressionOrDeclarationStatement()) {
            printf("asExpressionOrDeclarationStatement\n");
            return;
        }
        if (ast->asExpressionStatement()) {
            printf("asExpressionStatement\n");
            return;
        }
        if (ast->asForStatement()) {
            printf("asForStatement\n");
            return;
        }
        if (ast->asForeachStatement()) {
            printf("asForeachStatement\n");
            return;
        }
        if (ast->asFunctionDeclarator()) {
            printf("asFunctionDeclarator\n");
            return;
        }
        if (ast->asFunctionDefinition()) {
            printf("asFunctionDefinition\n");
            return;
        }
        if (ast->asGotoStatement()) {
            printf("asGotoStatement\n");
            return;
        }
        if (ast->asIdExpression()) {
            printf("asIdExpression\n");
            return;
        }
        if (ast->asIfStatement()) {
            printf("asIfStatement\n");
            return;
        }
        if (ast->asLabeledStatement()) {
            printf("asLabeledStatement\n");
            return;
        }
        if (ast->asLambdaCapture()) {
            printf("asLambdaCapture\n");
            return;
        }
        if (ast->asLambdaDeclarator()) {
            printf("asLambdaDeclarator\n");
            return;
        }
        if (ast->asLambdaExpression()) {
            printf("asLambdaExpression\n");
            return;
        }
        if (ast->asLambdaIntroducer()) {
            printf("asLambdaIntroducer\n");
            return;
        }
        if (ast->asLinkageBody()) {
            printf("asLinkageBody\n");
            return;
        }
        if (ast->asLinkageSpecification()) {
            printf("asLinkageSpecification\n");
            return;
        }
        if (ast->asMemInitializer()) {
            printf("asMemInitializer\n");
            return;
        }
        if (ast->asMemberAccess()) {
            printf("asMemberAccess\n");
            return;
        }
        if (ast->asName()) {
            printf("asName\n");
            return;
        }
        if (ast->asNamedTypeSpecifier()) {
            printf("asNamedTypeSpecifier\n");
            return;
        }
        if (ast->asNamespace()) {
            printf("asNamespace\n");
            return;
        }
        if (ast->asNamespaceAliasDefinition()) {
            printf("asNamespaceAliasDefinition\n");
            return;
        }
        if (ast->asNestedDeclarator()) {
            printf("asNestedDeclarator\n");
            return;
        }
        if (ast->asNestedExpression()) {
            printf("asNestedExpression\n");
            return;
        }
        if (ast->asNestedNameSpecifier()) {
            printf("asNestedNameSpecifier\n");
            return;
        }
        if (ast->asNewArrayDeclarator()) {
            printf("asNewArrayDeclarator\n");
            return;
        }
        if (ast->asNewExpression()) {
            printf("asNewExpression\n");
            return;
        }
        if (ast->asNewTypeId()) {
            printf("asNewTypeId\n");
            return;
        }
        if (ast->asNoExceptSpecification()) {
            printf("asNoExceptSpecification\n");
            return;
        }
        if (ast->asNumericLiteral()) {
            printf("asNumericLiteral\n");
            return;
        }
        if (ast->asObjCClassDeclaration()) {
            printf("asObjCClassDeclaration\n");
            return;
        }
        if (ast->asObjCClassForwardDeclaration()) {
            printf("asObjCClassForwardDeclaration\n");
            return;
        }
        if (ast->asObjCDynamicPropertiesDeclaration()) {
            printf("asObjCDynamicPropertiesDeclaration\n");
            return;
        }
        if (ast->asObjCEncodeExpression()) {
            printf("asObjCEncodeExpression\n");
            return;
        }
        if (ast->asObjCFastEnumeration()) {
            printf("asObjCFastEnumeration\n");
            return;
        }
        if (ast->asObjCInstanceVariablesDeclaration()) {
            printf("asObjCInstanceVariablesDeclaration\n");
            return;
        }
        if (ast->asObjCMessageArgument()) {
            printf("asObjCMessageArgument\n");
            return;
        }
        if (ast->asObjCMessageArgumentDeclaration()) {
            printf("asObjCMessageArgumentDeclaration\n");
            return;
        }
        if (ast->asObjCMessageExpression()) {
            printf("asObjCMessageExpression\n");
            return;
        }
        if (ast->asObjCMethodDeclaration()) {
            printf("asObjCMethodDeclaration\n");
            return;
        }
        if (ast->asObjCMethodPrototype()) {
            printf("asObjCMethodPrototype\n");
            return;
        }
        if (ast->asObjCPropertyAttribute()) {
            printf("asObjCPropertyAttribute\n");
            return;
        }
        if (ast->asObjCPropertyDeclaration()) {
            printf("asObjCPropertyDeclaration\n");
            return;
        }
        if (ast->asObjCProtocolDeclaration()) {
            printf("asObjCProtocolDeclaration\n");
            return;
        }
        if (ast->asObjCProtocolExpression()) {
            printf("asObjCProtocolExpression\n");
            return;
        }
        if (ast->asObjCProtocolForwardDeclaration()) {
            printf("asObjCProtocolForwardDeclaration\n");
            return;
        }
        if (ast->asObjCProtocolRefs()) {
            printf("asObjCProtocolRefs\n");
            return;
        }
        if (ast->asObjCSelector()) {
            printf("asObjCSelector\n");
            return;
        }
        if (ast->asObjCSelectorArgument()) {
            printf("asObjCSelectorArgument\n");
            return;
        }
        if (ast->asObjCSelectorExpression()) {
            printf("asObjCSelectorExpression\n");
            return;
        }
        if (ast->asObjCSynchronizedStatement()) {
            printf("asObjCSynchronizedStatement\n");
            return;
        }
        if (ast->asObjCSynthesizedPropertiesDeclaration()) {
            printf("asObjCSynthesizedPropertiesDeclaration\n");
            return;
        }
        if (ast->asObjCSynthesizedProperty()) {
            printf("asObjCSynthesizedProperty\n");
            return;
        }
        if (ast->asObjCTypeName()) {
            printf("asObjCTypeName\n");
            return;
        }
        if (ast->asObjCVisibilityDeclaration()) {
            printf("asObjCVisibilityDeclaration\n");
            return;
        }
        if (ast->asOperator()) {
            printf("asOperator\n");
            return;
        }
        if (ast->asOperatorFunctionId()) {
            printf("asOperatorFunctionId\n");
            return;
        }
        if (ast->asParameterDeclaration()) {
            printf("asParameterDeclaration\n");
            return;
        }
        if (ast->asParameterDeclarationClause()) {
            printf("asParameterDeclarationClause\n");
            return;
        }
        if (ast->asPointer()) {
            printf("asPointer\n");
            return;
        }
        if (ast->asPointerLiteral()) {
            printf("asPointerLiteral\n");
            return;
        }
        if (ast->asPointerToMember()) {
            printf("asPointerToMember\n");
            return;
        }
        if (ast->asPostIncrDecr()) {
            printf("asPostIncrDecr\n");
            return;
        }
        if (ast->asPostfix()) {
            printf("asPostfix\n");
            return;
        }
        if (ast->asPostfixDeclarator()) {
            printf("asPostfixDeclarator\n");
            return;
        }
        if (ast->asPtrOperator()) {
            printf("asPtrOperator\n");
            return;
        }
        if (ast->asQtEnumDeclaration()) {
            printf("asQtEnumDeclaration\n");
            return;
        }
        if (ast->asQtFlagsDeclaration()) {
            printf("asQtFlagsDeclaration\n");
            return;
        }
        if (ast->asQtInterfaceName()) {
            printf("asQtInterfaceName\n");
            return;
        }
        if (ast->asQtInterfacesDeclaration()) {
            printf("asQtInterfacesDeclaration\n");
            return;
        }
        if (ast->asQtMemberDeclaration()) {
            printf("asQtMemberDeclaration\n");
            return;
        }
        if (ast->asQtMethod()) {
            printf("asQtMethod\n");
            return;
        }
        if (ast->asQtObjectTag()) {
            printf("asQtObjectTag\n");
            return;
        }
        if (ast->asQtPrivateSlot()) {
            printf("asQtPrivateSlot\n");
            return;
        }
        if (ast->asQtPropertyDeclaration()) {
            printf("asQtPropertyDeclaration\n");
            return;
        }
        if (ast->asQtPropertyDeclarationItem()) {
            printf("asQtPropertyDeclarationItem\n");
            return;
        }
        if (ast->asQualifiedName()) {
            printf("asQualifiedName\n");
            return;
        }
        if (ast->asRangeBasedForStatement()) {
            printf("asRangeBasedForStatement\n");
            return;
        }
        if (ast->asReference()) {
            printf("asReference\n");
            return;
        }
        if (ast->asReturnStatement()) {
            printf("asReturnStatement\n");
            return;
        }
        if (ast->asSimpleDeclaration()) {
            printf("asSimpleDeclaration\n");
            return;
        }
        if (ast->asSimpleName()) {
            printf("asSimpleName\n");
            return;
        }
        if (ast->asSimpleSpecifier()) {
            printf("asSimpleSpecifier\n");
            return;
        }
        if (ast->asSizeofExpression()) {
            printf("asSizeofExpression\n");
            return;
        }
        if (ast->asSpecifier()) {
            printf("asSpecifier\n");
            return;
        }
        if (ast->asStatement()) {
            printf("asStatement\n");
            return;
        }
        if (ast->asStaticAssertDeclaration()) {
            printf("asStaticAssertDeclaration\n");
            return;
        }
        if (ast->asStringLiteral()) {
            printf("asStringLiteral\n");
            return;
        }
        if (ast->asSwitchStatement()) {
            printf("asSwitchStatement\n");
            return;
        }
        if (ast->asTemplateDeclaration()) {
            printf("asTemplateDeclaration\n");
            return;
        }
        if (ast->asTemplateId()) {
            printf("asTemplateId\n");
            return;
        }
        if (ast->asTemplateTypeParameter()) {
            printf("asTemplateTypeParameter\n");
            return;
        }
        if (ast->asThisExpression()) {
            printf("asThisExpression\n");
            return;
        }
        if (ast->asThrowExpression()) {
            printf("asThrowExpression\n");
            return;
        }
        if (ast->asTrailingReturnType()) {
            printf("asTrailingReturnType\n");
            return;
        }
        if (ast->asTranslationUnit()) {
            printf("asTranslationUnit\n");
            return;
        }
        if (ast->asTryBlockStatement()) {
            printf("asTryBlockStatement\n");
            return;
        }
        if (ast->asTypeConstructorCall()) {
            printf("asTypeConstructorCall\n");
            return;
        }
        if (ast->asTypeId()) {
            printf("asTypeId\n");
            return;
        }
        if (ast->asTypeidExpression()) {
            printf("asTypeidExpression\n");
            return;
        }
        if (ast->asTypenameCallExpression()) {
            printf("asTypenameCallExpression\n");
            return;
        }
        if (ast->asTypenameTypeParameter()) {
            printf("asTypenameTypeParameter\n");
            return;
        }
        if (ast->asTypeofSpecifier()) {
            printf("asTypeofSpecifier\n");
            return;
        }
        if (ast->asUnaryExpression()) {
            printf("asUnaryExpression\n");
            return;
        }
        if (ast->asUsing()) {
            printf("asUsing\n");
            return;
        }
        if (ast->asUsingDirective()) {
            printf("asUsingDirective\n");
            return;
        }
        if (ast->asWhileStatement()) {
            printf("asWhileStatement\n");
            return;
        }

        printf("Node\n");
    }
private:
    int mIndent;
};

int main(int argc, char **argv)
{
    CPlusPlus::Control control;
    const char *file = "foo.cpp";
    // _control->setDiagnosticClient(new DocumentDiagnosticClient(this, &_diagnosticMessages));

    const CPlusPlus::StringLiteral *fileId = control.stringLiteral(file, strlen(file));
    CPlusPlus::TranslationUnit translationUnit(&control, fileId);
    translationUnit.setQtMocRunEnabled(true);
    translationUnit.setCxxOxEnabled(true);
    translationUnit.setObjCEnabled(true);
    control.switchTranslationUnit(&translationUnit);
    FILE *f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = new char[len + 1];
    fread(buf, 1, len, f);
    buf[len] = '\0';

    translationUnit.setSource(buf, len);
    translationUnit.tokenize();
    CPlusPlus::Parser parser(&translationUnit);
    CPlusPlus::TranslationUnitAST *ast = 0;
    if (parser.parseTranslationUnit(ast)) {
        Visitor visitor(&translationUnit);
        visitor.accept(ast);
        printf("[%s] %s:%d: if (parser.parseTranslationUnit(ast)) { [after]\n", __func__, __FILE__, __LINE__);
    } else {
        printf("[%s] %s:%d: } else { [after]\n", __func__, __FILE__, __LINE__);
    }
    return 0;
}
