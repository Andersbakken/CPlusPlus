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
    preprocessor.run("main.cpp");

    return 0;
}
