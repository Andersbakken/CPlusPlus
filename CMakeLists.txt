cmake_minimum_required(VERSION 2.8)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

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
            src/libs/3rdparty/cplusplus/ASTPatternBuilder.cpp
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
            src/libs/3rdparty/cplusplus/LiteralTable.cpp
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


# all sources for utils
            # src/libs/utils/annotateditemdelegate.cpp
            # src/libs/utils/appmainwindow.cpp
            # src/libs/utils/basetreeview.cpp
            # src/libs/utils/basevalidatinglineedit.cpp
            # src/libs/utils/bracematcher.cpp
            # src/libs/utils/buildablehelperlibrary.cpp
            # src/libs/utils/changeset.cpp
            # src/libs/utils/checkablemessagebox.cpp
            # src/libs/utils/classnamevalidatinglineedit.cpp
            # src/libs/utils/codegeneration.cpp
            # src/libs/utils/completingtextedit.cpp
            # src/libs/utils/consoleprocess.cpp
            # src/libs/utils/consoleprocess_unix.cpp
            # src/libs/utils/consoleprocess_win.cpp
            # src/libs/utils/crumblepath.cpp
            # src/libs/utils/detailsbutton.cpp
            # src/libs/utils/detailswidget.cpp
            # src/libs/utils/elfreader.cpp
            # src/libs/utils/elidinglabel.cpp
            # src/libs/utils/environment.cpp
            # src/libs/utils/environmentmodel.cpp
            # src/libs/utils/faketooltip.cpp
            # src/libs/utils/fancylineedit.cpp
            # src/libs/utils/fancymainwindow.cpp
            # src/libs/utils/fileinprojectfinder.cpp
            # src/libs/utils/filenamevalidatinglineedit.cpp
            # src/libs/utils/filesearch.cpp
            # src/libs/utils/filesystemwatcher.cpp
            # src/libs/utils/fileutils.cpp
            # src/libs/utils/filewizarddialog.cpp
            # src/libs/utils/filewizardpage.cpp
            # src/libs/utils/filterlineedit.cpp
            # src/libs/utils/flowlayout.cpp
            # src/libs/utils/headerviewstretcher.cpp
            # src/libs/utils/historycompleter.cpp
            # src/libs/utils/hostosinfo.cpp
            # src/libs/utils/htmldocextractor.cpp
            # src/libs/utils/ipaddresslineedit.cpp
            # src/libs/utils/iwelcomepage.cpp
            # src/libs/utils/json.cpp
            # src/libs/utils/linecolumnlabel.cpp
            # src/libs/utils/navigationtreeview.cpp
            # src/libs/utils/networkaccessmanager.cpp
            # src/libs/utils/newclasswidget.cpp
            # src/libs/utils/outputformatter.cpp
            # src/libs/utils/parameteraction.cpp
            # src/libs/utils/pathchooser.cpp
            # src/libs/utils/pathlisteditor.cpp
            # src/libs/utils/persistentsettings.cpp
            # src/libs/utils/portlist.cpp
            # src/libs/utils/process_ctrlc_stub.cpp
            # src/libs/utils/process_stub_unix.c
            # src/libs/utils/process_stub_win.c
            # src/libs/utils/projectintropage.cpp
            # src/libs/utils/projectnamevalidatinglineedit.cpp
            # src/libs/utils/proxyaction.cpp
            # src/libs/utils/qtcassert.cpp
            # src/libs/utils/qtcolorbutton.cpp
            # src/libs/utils/qtcprocess.cpp
            # src/libs/utils/reloadpromptutils.cpp
            # src/libs/utils/savedaction.cpp
            # src/libs/utils/savefile.cpp
            # src/libs/utils/settingsselector.cpp
            # src/libs/utils/statuslabel.cpp
            # src/libs/utils/stringutils.cpp
            # src/libs/utils/styledbar.cpp
            # src/libs/utils/stylehelper.cpp
            # src/libs/utils/synchronousprocess.cpp
            # src/libs/utils/tcpportsgatherer.cpp
            # src/libs/utils/tooltip/tipcontents.cpp
            # src/libs/utils/tooltip/tips.cpp
            # src/libs/utils/tooltip/tooltip.cpp
            # src/libs/utils/uncommentselection.cpp
            # src/libs/utils/unixutils.cpp
            # src/libs/utils/winutils.cpp
            # src/libs/utils/wizard.cpp

# mocables in src/libs/utils
# src/libs/utils/appmainwindow.h
# src/libs/utils/basetreeview.h
# src/libs/utils/basevalidatinglineedit.h
# src/libs/utils/checkablemessagebox.h
# src/libs/utils/classnamevalidatinglineedit.h
# src/libs/utils/completingtextedit.h
# src/libs/utils/consoleprocess.h
# src/libs/utils/crumblepath.cpp
# src/libs/utils/crumblepath.h
# src/libs/utils/detailsbutton.h
# src/libs/utils/detailswidget.h
# src/libs/utils/elidinglabel.h
# src/libs/utils/environmentmodel.h
# src/libs/utils/faketooltip.h
# src/libs/utils/fancylineedit.h
# src/libs/utils/fancymainwindow.h
# src/libs/utils/filenamevalidatinglineedit.h
# src/libs/utils/filesystemwatcher.h
# src/libs/utils/filewizarddialog.h
# src/libs/utils/filewizardpage.h
# src/libs/utils/filterlineedit.h
# src/libs/utils/historycompleter.h
# src/libs/utils/ipaddresslineedit.h
# src/libs/utils/iwelcomepage.h
# src/libs/utils/linecolumnlabel.h
# src/libs/utils/multitask.h
# src/libs/utils/navigationtreeview.h
# src/libs/utils/networkaccessmanager.h
# src/libs/utils/newclasswidget.h
# src/libs/utils/outputformatter.h
# src/libs/utils/parameteraction.h
# src/libs/utils/pathchooser.h
# src/libs/utils/pathlisteditor.h
# src/libs/utils/projectintropage.h
# src/libs/utils/projectnamevalidatinglineedit.h
# src/libs/utils/proxyaction.h
# src/libs/utils/qtcolorbutton.cpp
# src/libs/utils/qtcolorbutton.h
# src/libs/utils/qtcprocess.h
# src/libs/utils/savedaction.h
# src/libs/utils/settingsselector.h
# src/libs/utils/statuslabel.h
# src/libs/utils/styledbar.h
# src/libs/utils/synchronousprocess.h
# src/libs/utils/tooltip/tips.h
# src/libs/utils/tooltip/tooltip.h
# src/libs/utils/wizard.cpp
# src/libs/utils/wizard.h
