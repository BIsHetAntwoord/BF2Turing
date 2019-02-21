#ifndef TURINGCOMPILER_HPP_INCLUDED
#define TURINGCOMPILER_HPP_INCLUDED

#include <ostream>
#include <map>
#include <vector>
#include <limits>

class BfCode;

enum class TransitionShift
{
    SHIFT_LEFT,
    SHIFT_RIGHT,
    KEEP
};

const size_t ALPHABET_DEFAULT = 256;
const size_t ALPHABET_ALL = 257;
const size_t ALPHABET_PLACEHOLDER_1 = 258;
const size_t ALPHABET_PLACEHOLDER_2 = 259;
const size_t ALPHABET_WRITE_HEAD = 260;

const size_t STATE_ACCEPT = std::numeric_limits<size_t>::max();
const size_t STATE_REJECT = std::numeric_limits<size_t>::max() - 1;

const size_t INPUT_SIZE = 16;
const size_t OUTPUT_SIZE = 16;

class TuringCompiler;

class State
{
    private:
        struct Transition
        {
            size_t next_state;
            size_t alphabet;
            size_t writeback;
            TransitionShift transition;

            inline Transition(size_t next_state, size_t alphabet, size_t writeback, TransitionShift transition) : next_state(next_state),
                alphabet(alphabet), writeback(writeback), transition(transition) {}
        };

        size_t state_no;
        std::vector<Transition> transitions;
    public:
        State();
        ~State() = default;

        inline void setState(size_t no)
        {
            this->state_no = no;
        }
        inline size_t getStateNo() const
        {
            return this->state_no;
        }
        void addTransition(size_t, size_t, size_t, TransitionShift);
        void print(std::ostream&, TuringCompiler&);
};

class TuringCompiler
{
    private:
        BfCode& code;
        std::map<size_t, size_t> state_map;
        std::vector<State> states;
        size_t current_ip = 0;

        void genPlus();
        void genMin();
        void genRShift();
        void genLShift();
        void genInput();
        void genOutput();
        void genJumpForward();
        void genJumpBackward();

        void genPrologue();
        void genEpilogue();

        size_t getStateIndex(size_t);
        State& getState(size_t);
    public:
        TuringCompiler(BfCode&);
        ~TuringCompiler();

        void run();
        void print(std::ostream&);

        inline State& resolveState(size_t no)
        {
            return this->states[no];
        }
};

#endif // TURINGCOMPILER_HPP_INCLUDED
