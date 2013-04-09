cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_CURRENT_LIST_DIR} cplusplus src pp)
add_definitions(-DCPLUSPLUS_WITHOUT_QT)
include(${CMAKE_CURRENT_LIST_DIR}/rct/rct.cmake)
add_executable(rparser
               ${RCT_SOURCES}
               src/main.cpp
               src/CppPreprocessor.cpp
               cplusplus/QtContextKeywords.cpp
               cplusplus/Type.cpp
               cplusplus/Token.cpp
               cplusplus/Templates.cpp
               cplusplus/Symbols.cpp
               cplusplus/Parser.cpp
               cplusplus/DiagnosticClient.cpp
               cplusplus/ASTVisit.cpp
               cplusplus/AST.cpp
               cplusplus/Scope.cpp
               cplusplus/TranslationUnit.cpp
               cplusplus/NameVisitor.cpp
               cplusplus/ObjectiveCAtKeywords.cpp
               cplusplus/TypeMatcher.cpp
               cplusplus/LiteralTable.cpp
               cplusplus/TypeVisitor.cpp
               cplusplus/FullySpecifiedType.cpp
               cplusplus/ASTVisitor.cpp
               cplusplus/ASTMatcher.cpp
               cplusplus/Keywords.cpp
               cplusplus/ASTPatternBuilder.cpp
               cplusplus/Bind.cpp
               cplusplus/ASTClone.cpp
               cplusplus/Literals.cpp
               cplusplus/Name.cpp
               cplusplus/Symbol.cpp
               cplusplus/CoreTypes.cpp
               cplusplus/Control.cpp
               cplusplus/ObjectiveCTypeQualifiers.cpp
               cplusplus/Lexer.cpp
               cplusplus/SymbolVisitor.cpp
               cplusplus/MemoryPool.cpp
               cplusplus/Names.cpp
               cplusplus/ASTMatch0.cpp
               pp/PPToken.cpp
               pp/Macro.cpp
               pp/PreprocessorClient.cpp
               pp/PreprocessorEnvironment.cpp
               pp/pp-scanner.cpp
               pp/pp-engine.cpp)

target_link_libraries(rparser crypto pthread)

