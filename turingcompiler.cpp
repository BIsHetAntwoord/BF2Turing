#include "turingcompiler.hpp"
#include "bfcode.hpp"

#include <sstream>
#include <string>

std::string to_letter(size_t alphabet)
{
    switch(alphabet)
    {
        case ALPHABET_ALL:
            return "*";
        case ALPHABET_DEFAULT:
            return "_";
        case ALPHABET_PLACEHOLDER_1:
            return "A";
        case ALPHABET_PLACEHOLDER_2:
            return "B";
        case ALPHABET_WRITE_HEAD:
            return "WH";
        default:
        {
            std::stringstream ss;
            ss << alphabet;
            return ss.str();
        }
            break;
    }
}

std::string to_shift_symbol(TransitionShift shift)
{
    switch(shift)
    {
        case TransitionShift::KEEP:
            return "!";
        case TransitionShift::SHIFT_LEFT:
            return "<";
        case TransitionShift::SHIFT_RIGHT:
            return ">";
    }
    return "";
}

State::State() {}

void State::addTransition(size_t next_state, size_t alphabet, size_t writeback, TransitionShift transition)
{
    this->transitions.emplace_back(next_state, alphabet, writeback, transition);
}

void State::print(std::ostream& os, TuringCompiler& compiler)
{
    os << "q" << this->state_no << " {" << std::endl;
    for(auto& transition : this->transitions)
    {
        os << "    " << to_letter(transition.alphabet) << " -> q";
        if(transition.next_state == STATE_ACCEPT)
            os << "A";
        else if(transition.next_state == STATE_REJECT)
            os << "R";
        else
            os << compiler.resolveState(transition.next_state).getStateNo();
        os << " " << to_letter(transition.writeback) << " " << to_shift_symbol(transition.transition) << std::endl;
    }
    os << "}" << std::endl << std::endl;
}

TuringCompiler::TuringCompiler(BfCode& code) : code(code) {}
TuringCompiler::~TuringCompiler() {}

void TuringCompiler::genPlus()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    for(size_t i = 0; i < 256; ++i)
        state.addTransition(next_state, i, (i+1) % 256, TransitionShift::KEEP);
    state.addTransition(STATE_REJECT, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
}

void TuringCompiler::genMin()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    for(size_t i = 0; i < 256; ++i)
        state.addTransition(next_state, i, (i+255) % 256, TransitionShift::KEEP);
    state.addTransition(STATE_REJECT, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
}

void TuringCompiler::genRShift()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    this->states.emplace_back();
    State& extra_state = this->states[this->states.size() - 1];

    state.addTransition(this->states.size() - 1, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_RIGHT);
    extra_state.addTransition(next_state, ALPHABET_DEFAULT, 0, TransitionShift::KEEP);
    extra_state.addTransition(next_state, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
}

void TuringCompiler::genLShift()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    state.addTransition(next_state, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_LEFT);
}

void TuringCompiler::genOutput()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    size_t base_state = this->states.size();

    for(size_t i = 0; i < 256; ++i)
        state.addTransition(base_state + i, i, ALPHABET_PLACEHOLDER_2, TransitionShift::KEEP);
    state.addTransition(STATE_REJECT, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);

    for(size_t i = 0; i < 256; ++i)
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 1];
        state.addTransition(base_state + 256 + i, ALPHABET_WRITE_HEAD, i, TransitionShift::SHIFT_RIGHT);
        state.addTransition(base_state + i, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_LEFT);
    }

    for(size_t i = 0; i < 256; ++i)
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 1];
        state.addTransition(base_state + 512 + i, ALPHABET_PLACEHOLDER_1, ALPHABET_WRITE_HEAD, TransitionShift::SHIFT_RIGHT);
        state.addTransition(STATE_REJECT, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
    }

    for(size_t i = 0; i < 256; ++i)
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 1];
        state.addTransition(next_state, ALPHABET_PLACEHOLDER_2, i, TransitionShift::KEEP);
        state.addTransition(base_state + 512 + i, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_RIGHT);
    }
}

void TuringCompiler::genInput()
{
    //TODO, do something
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    State& state = this->getState(this->current_ip);
    size_t base_state = this->states.size();
    state.addTransition(base_state, ALPHABET_ALL, ALPHABET_PLACEHOLDER_2, TransitionShift::SHIFT_LEFT);

    //Go to the start of the read section
    {
        this->states.emplace_back();
        State& new_state = this->states[base_state];
        new_state.addTransition(base_state + 1, ALPHABET_DEFAULT, ALPHABET_DEFAULT, TransitionShift::SHIFT_RIGHT);
        new_state.addTransition(base_state, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_LEFT);
    }

    //Find the first readable character
    {
        this->states.emplace_back();
        State& new_state = this->states[base_state + 1];
        new_state.addTransition(base_state + 1, ALPHABET_PLACEHOLDER_1, ALPHABET_PLACEHOLDER_1, TransitionShift::SHIFT_RIGHT);

        for(size_t i = 0; i < 256; ++i)
            new_state.addTransition(base_state + 2 + i, i, ALPHABET_PLACEHOLDER_1, TransitionShift::SHIFT_RIGHT);

        new_state.addTransition(STATE_REJECT, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
    }

    //Next, put the next readable token at the position of the read token
    for(size_t i = 0; i < 256; ++i)
    {
        this->states.emplace_back();
        State& new_state = this->states[base_state + 2 + i];
        new_state.addTransition(next_state, ALPHABET_PLACEHOLDER_2, i, TransitionShift::KEEP);
        new_state.addTransition(base_state + 2 + i, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_RIGHT);
    }
}

void TuringCompiler::genJumpForward()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    size_t jmp_target = this->getStateIndex(this->code.jumps[this->current_ip]);

    State& state = this->getState(this->current_ip);

    state.addTransition(jmp_target, 0, 0, TransitionShift::KEEP);
    state.addTransition(next_state, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
}

void TuringCompiler::genJumpBackward()
{
    size_t next_state = this->getStateIndex(this->current_ip + 1);
    size_t jmp_target = this->getStateIndex(this->code.jumps[this->current_ip]);

    State& state = this->getState(this->current_ip);

    state.addTransition(next_state, 0, 0, TransitionShift::KEEP);
    state.addTransition(jmp_target, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::KEEP);
}

void TuringCompiler::genPrologue()
{
    this->states.emplace_back();

    for(size_t i = 0; i < INPUT_SIZE; ++i)
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 2];
        state.addTransition(this->states.size() - 1, ALPHABET_DEFAULT, ALPHABET_PLACEHOLDER_1, TransitionShift::SHIFT_RIGHT);
        state.addTransition(this->states.size() - 1, ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_RIGHT);
    }

    //First, set the write head
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 2];
        state.addTransition(this->states.size() - 1, ALPHABET_ALL, ALPHABET_WRITE_HEAD, TransitionShift::SHIFT_RIGHT);
    }

    //Initialize memory
    for(size_t i = 1; i < OUTPUT_SIZE - 1; ++i)
    {
        this->states.emplace_back();
        State& state = this->states[this->states.size() - 2];
        state.addTransition(this->states.size() - 1, ALPHABET_ALL, ALPHABET_PLACEHOLDER_1, TransitionShift::SHIFT_RIGHT);
    }

    //Setup jump to start of code
    this->states.emplace_back();
    size_t current_state = this->states.size() - 2;
    size_t int_state = this->states.size() - 1;
    size_t next_state = this->getStateIndex(0);
    State& state = this->states[current_state];
    state.addTransition(int_state, ALPHABET_ALL, ALPHABET_PLACEHOLDER_1, TransitionShift::SHIFT_RIGHT);
    State& int_state_ref = this->states[int_state];
    int_state_ref.addTransition(next_state, ALPHABET_ALL, 0, TransitionShift::KEEP);
}

void TuringCompiler::genEpilogue()
{
    State& state = this->getState(this->current_ip);
    state.addTransition(this->getStateIndex(this->current_ip), ALPHABET_ALL, ALPHABET_ALL, TransitionShift::SHIFT_LEFT);
    state.addTransition(STATE_ACCEPT, ALPHABET_DEFAULT, ALPHABET_DEFAULT, TransitionShift::SHIFT_RIGHT);
}

State& TuringCompiler::getState(size_t ip)
{
    if(this->state_map.count(ip) > 0)
        return this->states[this->state_map[ip]];
    else
    {
        this->states.emplace_back();
        this->state_map[ip] = this->states.size() - 1;
        return this->states[this->state_map[ip]];
    }
}

size_t TuringCompiler::getStateIndex(size_t ip)
{
    State& state = this->getState(ip);

    for(size_t i = 0; i < this->states.size(); ++i)
    {
        if(&state == &this->states[i])
            return i;
    }
    return 0;
}

void TuringCompiler::run()
{
    this->genPrologue();

    for(this->current_ip = 0; this->current_ip < this->code.code.size(); ++this->current_ip)
    {
        switch(this->code.code[this->current_ip])
        {
            case '+':
                this->genPlus();
                break;
            case '-':
                this->genMin();
                break;
            case '>':
                this->genRShift();
                break;
            case '<':
                this->genLShift();
                break;
            case '.':
                this->genOutput();
                break;
            case ',':
                this->genInput();
                break;
            case '[':
                this->genJumpForward();
                break;
            case ']':
                this->genJumpBackward();
                break;
        }
    }
    this->genEpilogue();
}

void TuringCompiler::print(std::ostream& os)
{
    for(size_t i = 0; i < this->states.size(); ++i)
        this->states[i].setState(i);

    os << "alphabet: ";
    for(size_t i = 0; i < 256; ++i)
        os << i << ", ";
    os << "A, B, WH;" << std::endl
        << "accept: qA;" << std::endl
        << "reject: qR;" << std::endl;

    for(size_t i = 0; i < this->states.size(); ++i)
        this->states[i].print(os, *this);
}
