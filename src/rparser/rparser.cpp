#include "rparser.h"
struct RParserPrivate
{



};

RParser::RParser()
    : mData(new RParserPrivate)
{
    
}

RParser::~RParser()
{
    delete mData;
}

bool RParser::parse(const std::string &sourceFile,
                    const std::vector<std::string> &includePaths,
                    const std::vector<std::string> &defines)
{

    return false;
}

void RParser::visit()
{

}
