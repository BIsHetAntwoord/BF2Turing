#include <iostream>
#include <fstream>
#include "bfcodeparser.hpp"
#include "turingcompiler.hpp"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "Need a file" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    std::ifstream input(filename);
    BfCodeParser parser(input);
    BfCode code = parser.parse();

    TuringCompiler compiler(code);
    compiler.run();
    compiler.print(std::cout);

    return 0;
}
