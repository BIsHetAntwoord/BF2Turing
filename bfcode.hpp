#ifndef BFCODE_HPP_INCLUDED
#define BFCODE_HPP_INCLUDED

#include <string>
#include <map>
#include <cstddef>

struct BfCode
{
    std::string code;
    std::map<size_t, size_t> jumps;
};

#endif // BFCODE_HPP_INCLUDED
