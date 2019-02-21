#ifndef GENERATOR_HPP_INCLUDED
#define GENERATOR_HPP_INCLUDED

class BfCode;

class TuringGenerator
{
    private:
        BfCode& code;
    public:
        TuringGenerator(BfCode&);
        ~TuringGenerator();
};

#endif // GENERATOR_HPP_INCLUDED
