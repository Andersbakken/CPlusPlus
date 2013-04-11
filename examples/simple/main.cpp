#include <cppmodelmanager.h>
#include <QStringList>

using namespace CPlusPlus;
using namespace CppTools::Internal;

int main(int argc, char** argv)
{
    QString inputFile;
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
        } else {
            // assume input
            if (!inputFile.isEmpty()) {
                qFatal("Already have an input file, new '%s', old '%s'", argv[i], qPrintable(inputFile));
            }
            inputFile = QString::fromUtf8(argv[i]);
            qDebug("Using '%s' as input", argv[i]);
        }
    }

    QPointer<CppModelManager> manager;
    CppPreprocessor preprocessor(manager);
    preprocessor.setIncludePaths(incs);
    preprocessor.addDefinitions(defs);
    preprocessor.run(inputFile);

    CPlusPlus::Snapshot::const_iterator doc = manager->snapshot().begin();
    const CPlusPlus::Snapshot::const_iterator end = manager->snapshot().end();
    while (doc != end) {
        CPlusPlus::TranslationUnit *translationUnit = doc.value()->translationUnit();
        qDebug("Got doc '%s' with unit %p", qPrintable(doc.key()), translationUnit);
        ++doc;
    }

    return 0;
}
