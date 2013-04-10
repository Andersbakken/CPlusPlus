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
    ~RParser();
    bool parse(const std::string &sourceFile,
               const std::vector<std::string> &includePaths,
               const std::vector<std::string> &defines);

    void visit();
    struct Location {
        const char *path;
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
            Reference
        } kind;
        Location target;
        std::set<Location> references;
    };
    enum VisitorResult {
        Break,
        Continue,
        Recurse
    };
    virtual VisitorResult visitor(const Location &/*location*/, const Symbol &/*symbol*/) { return Break; }
private:
    RParserPrivate *mData;
};

#endif
