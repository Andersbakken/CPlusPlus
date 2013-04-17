cmake_minimum_required(VERSION 2.8)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS} -DNDEBUG)

set(CPLUSPLUS_MOCABLE_HEADERS
    src/libs/cplusplus/OverviewModel.h
    src/plugins/cpptools/ModelManagerInterface.h
    src/plugins/cpptools/cppfindreferences.h
    src/plugins/cpptools/cppindexingsupport.h
    src/plugins/cpptools/cppmodelmanager.h)

qt4_wrap_cpp(CPLUSPLUS_MOCS ${CPLUSPLUS_MOCABLE_HEADERS})

include_directories(src/libs/3rdparty
                    src/libs/3rdparty/cplusplus
                    src/libs
                    src/plugins/cpptools)

add_library(CPlusPlus STATIC
            src/libs/3rdparty/cplusplus/AST.cpp
            src/libs/3rdparty/cplusplus/ASTClone.cpp
            src/libs/3rdparty/cplusplus/ASTMatch0.cpp
            src/libs/3rdparty/cplusplus/ASTMatcher.cpp
            src/libs/3rdparty/cplusplus/ASTVisit.cpp
            src/libs/3rdparty/cplusplus/ASTVisitor.cpp
            src/libs/3rdparty/cplusplus/Bind.cpp
            src/libs/3rdparty/cplusplus/Control.cpp
            src/libs/3rdparty/cplusplus/CoreTypes.cpp
            src/libs/3rdparty/cplusplus/DiagnosticClient.cpp
            src/libs/3rdparty/cplusplus/FullySpecifiedType.cpp
            src/libs/3rdparty/cplusplus/Keywords.cpp
            src/libs/3rdparty/cplusplus/Lexer.cpp
            src/libs/3rdparty/cplusplus/Literals.cpp
            src/libs/3rdparty/cplusplus/MemoryPool.cpp
            src/libs/3rdparty/cplusplus/Name.cpp
            src/libs/3rdparty/cplusplus/Names.cpp
            src/libs/3rdparty/cplusplus/NameVisitor.cpp
            src/libs/3rdparty/cplusplus/ObjectiveCAtKeywords.cpp
            src/libs/3rdparty/cplusplus/ObjectiveCTypeQualifiers.cpp
            src/libs/3rdparty/cplusplus/Parser.cpp
            src/libs/3rdparty/cplusplus/QtContextKeywords.cpp
            src/libs/3rdparty/cplusplus/Scope.cpp
            src/libs/3rdparty/cplusplus/Symbol.cpp
            src/libs/3rdparty/cplusplus/Symbols.cpp
            src/libs/3rdparty/cplusplus/SymbolVisitor.cpp
            src/libs/3rdparty/cplusplus/Templates.cpp
            src/libs/3rdparty/cplusplus/Token.cpp
            src/libs/3rdparty/cplusplus/TranslationUnit.cpp
            src/libs/3rdparty/cplusplus/Type.cpp
            src/libs/3rdparty/cplusplus/TypeMatcher.cpp
            src/libs/3rdparty/cplusplus/TypeVisitor.cpp
            src/libs/cplusplus/ASTParent.cpp
            src/libs/cplusplus/ASTPath.cpp
            src/libs/cplusplus/BackwardsScanner.cpp
            src/libs/cplusplus/CppDocument.cpp
            src/libs/cplusplus/CppRewriter.cpp
            src/libs/cplusplus/DependencyTable.cpp
            src/libs/cplusplus/DeprecatedGenTemplateInstance.cpp
            src/libs/cplusplus/Dumpers.cpp
            src/libs/cplusplus/ExpressionUnderCursor.cpp
            src/libs/cplusplus/FastPreprocessor.cpp
            src/libs/cplusplus/findcdbbreakpoint.cpp
            src/libs/cplusplus/FindUsages.cpp
            src/libs/cplusplus/Icons.cpp
            src/libs/cplusplus/LookupContext.cpp
            src/libs/cplusplus/LookupItem.cpp
            src/libs/cplusplus/Macro.cpp
            src/libs/cplusplus/MatchingText.cpp
            src/libs/cplusplus/NamePrettyPrinter.cpp
            src/libs/cplusplus/Overview.cpp
            src/libs/cplusplus/OverviewModel.cpp
            src/libs/cplusplus/pp-engine.cpp
            src/libs/cplusplus/pp-scanner.cpp
            src/libs/cplusplus/PPToken.cpp
            src/libs/cplusplus/PreprocessorClient.cpp
            src/libs/cplusplus/PreprocessorEnvironment.cpp
            src/libs/cplusplus/ResolveExpression.cpp
            src/libs/cplusplus/SimpleLexer.cpp
            src/libs/cplusplus/SnapshotSymbolVisitor.cpp
            src/libs/cplusplus/SymbolNameVisitor.cpp
            src/libs/cplusplus/TypeOfExpression.cpp
            src/libs/cplusplus/TypePrettyPrinter.cpp
            src/libs/languageutils/componentversion.cpp
            src/libs/languageutils/fakemetaobject.cpp
            src/plugins/cpptools/builtinindexingsupport.cpp
            src/plugins/cpptools/cppfindreferences.cpp
            src/plugins/cpptools/cppindexingsupport.cpp
            src/plugins/cpptools/cppmodelmanager.cpp
            src/plugins/cpptools/ModelManagerInterface.cpp
            src/plugins/cpptools/searchsymbols.cpp
            src/plugins/cpptools/symbolfinder.cpp
            src/libs/utils/textfileformat.cpp
            src/libs/utils/fileutils.cpp
            src/libs/utils/savefile.cpp
            src/libs/utils/qtcassert.cpp
            ${CPLUSPLUS_MOCS})
