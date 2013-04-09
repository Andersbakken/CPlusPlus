#ifndef CppPreprocessor_h
#define CppPreprocessor_h

#include <rct/String.h>
#include <rct/Path.h>
#include <rct/List.h>
#include <rct/Set.h>
#include <rct/Map.h>

#include <CppDocument.h>
#include <PreprocessorClient.h>
#include <PreprocessorEnvironment.h>
#include <pp-engine.h>

class CppPreprocessor : public CPlusPlus::Client
{
public:
    CppPreprocessor(const Path &pwd, CPlusPlus::Snapshot& snapshot);
    virtual ~CppPreprocessor();

    void setIncludePaths(const List<Path> &includePaths);
    void setFrameworkPaths(const List<Path> &frameworkPaths);
    void addFrameworkPath(const Path &frameworkPath);
    void setProjectFiles(const List<String> &files);
    void setTodo(const Set<Path> &files);

    void run(const Path &fileName);
    void removeFromCache(const Path &fileName);

    void resetEnvironment();
    static Path cleanPath(const Path &path);

    const Set<Path> &todo() const
    { return mTodo; }

    Path pwd() const { return mPwd; }

protected:
    CPlusPlus::Document::Ptr switchDocument(CPlusPlus::Document::Ptr doc);

    bool includeFile(const Path &absoluteFilePath, String *result);
    String tryIncludeFile(Path &fileName, IncludeType type);
    String tryIncludeFile_helper(Path &fileName, IncludeType type);

    void mergeEnvironment(CPlusPlus::Document::Ptr doc);

    virtual void macroAdded(const CPlusPlus::Macro &macro);
    virtual void passedMacroDefinitionCheck(unsigned offset, unsigned line,
                                            const CPlusPlus::Macro &macro);
    virtual void failedMacroDefinitionCheck(unsigned offset, const CPlusPlus::StringRef &name);
    virtual void notifyMacroReference(unsigned offset, unsigned line,
                                      const CPlusPlus::Macro &macro);
    virtual void startExpandingMacro(unsigned offset,
                                     unsigned line,
                                     const CPlusPlus::Macro &macro,
                                     const List<CPlusPlus::MacroArgumentReference> &actuals);
    virtual void stopExpandingMacro(unsigned offset, const CPlusPlus::Macro &macro);
    virtual void markAsIncludeGuard(const String &macroName);
    virtual void startSkippingBlocks(unsigned offset);
    virtual void stopSkippingBlocks(unsigned offset);
    virtual void sourceNeeded(unsigned line, Path &fileName, IncludeType type);

private:
    CPlusPlus::Snapshot& mSnapshot;
    CPlusPlus::Environment mEnv;
    CPlusPlus::Preprocessor mPreprocess;
    CPlusPlus::Document::Ptr mCurrentDoc;
    List<Path> mIncludePaths;
    List<Path> mFrameworkPaths;
    Set<Path> mIncluded;
    Set<Path> mTodo;
    Set<Path> mProcessed;
    Map<Path, Path> mFileNameCache;
    Map<Path, String> mFileCache;
    const Path mPwd;
};

#endif

