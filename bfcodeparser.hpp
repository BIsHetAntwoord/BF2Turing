#ifndef BFCODEPARSER_HPP_INCLUDED
#define BFCODEPARSER_HPP_INCLUDED

#include "bfcode.hpp"

#include <istream>

class BfCodeParser
{
    private:
        std::istream& input;
    public:
        BfCodeParser(std::istream&);
        ~BfCodeParser();

        BfCode parse();
};

#endif
