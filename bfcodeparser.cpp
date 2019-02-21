#include "bfcodeparser.hpp"

#include <stack>
#include <cstddef>

BfCodeParser::BfCodeParser(std::istream& input) : input(input) {}

BfCodeParser::~BfCodeParser() {}

BfCode BfCodeParser::parse()
{
    std::stack<size_t> jumps;
    BfCode result;
    size_t ip = 0;
    while(this->input)
    {
        char c = this->input.get();
        switch(c)
        {
            case '[':
                jumps.push(ip);
                break;
            case ']':
            {
                size_t jump_from = jumps.top();
                jumps.pop();
                result.jumps[jump_from] = ip;
                result.jumps[ip] = jump_from;
            }
                break;
        }

        switch(c)
        {
            case '[':
            case ']':
            case '+':
            case '-':
            case '.':
            case ',':
            case '<':
            case '>':
                result.code.push_back(c);
                ++ip;
                break;
        }
    }
    return result;
}
