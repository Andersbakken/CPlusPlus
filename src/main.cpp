#include <CppPreprocessor.h>
#include <rct/Path.h>
#include <rct/StopWatch.h>
#include <rct/Log.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    CPlusPlus::Snapshot snapshot;
    CppPreprocessor preprocessor(Path::pwd(), snapshot);
    List<Path> includes;
    includes << "cplusplus" << "pp" << "src" << "doc" << "rct/src";
    preprocessor.setIncludePaths(includes);

    Path main = "src/main.cpp";

    preprocessor.run(main);

    CPlusPlus::Snapshot::const_iterator doc = snapshot.begin();
    const CPlusPlus::Snapshot::const_iterator end = snapshot.end();
    while (doc != end) {
        error() << "Found document" << doc->first;
        ++doc;
    }

    main.resolve();

    CPlusPlus::Document::Ptr mainDoc = snapshot.document(main);
    if (mainDoc) {
        CPlusPlus::TranslationUnit* tu = mainDoc->translationUnit();
        error("Found main.cpp: %p", tu);
    }

    return 0;
}
