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
        std::string path;
        uint32_t offset;
    };
    struct Symbol {
        std::string symbolName;
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
    virtual void visitor(const Location &/*location*/, const Symbol &/*symbol*/) {}
private:
    RParserPrivate *mData;
};

#endif
