#ifndef rparser_h
#define rparser_h

#include <string>
#include <vector>
#include <set>
#include <stdint.h>

struct RParserPrivate;
class RParser
{
public:
    RParser();
    virtual ~RParser();
    bool parse(const std::string &sourceFile,
               const std::vector<std::string> &includePaths,
               const std::vector<std::string> &defines);

    void visit();
    struct Location {
        uint32_t fileId;
        uint32_t offset;
    };
    struct Symbol {
        const char *symbolName;
        enum Kind {
            FunctionDefinition,
            FunctionDeclaration,
            Variable,
            Field,
            Argument,
            Enum,
            EnumValue
        } kind;
        Location target;
        std::set<Location> references;
    };
    virtual uint32_t fileId(const char *fileName) = 0; // 0 means don't visit symbol
    virtual bool visitor(const Location &/*location*/, const Symbol &/*symbol*/) { return true; }
private:
    RParserPrivate *mData;
};

#endif
