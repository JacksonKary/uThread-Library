#include "TCB.h"
#include <cassert>

TCB::TCB(int tid, Priority pr, void *(*start_routine)(void *arg), void *arg, State state) : _tid(tid), _pr(pr), _quantum(0), _state(state), _lock_count(0)
{
    // init
    _tid = tid;
    _pr = pr;
    _quantum = 0;
    _state = state;
    _stack = new char[STACK_SIZE];

    // Get the current context
    getcontext(&_context); 
    // Modify stack
    _context.uc_stack.ss_sp = _stack;
    _context.uc_stack.ss_size = STACK_SIZE;
    _context.uc_stack.ss_flags = 0;

    // Set up to run with stub
    makecontext(&_context, (void(*)())stub, 2, start_routine, arg);
}

TCB::~TCB()
{
    free(_stack);
}

void TCB::setState(State state)
{
    _state = state;
}

State TCB::getState() const
{
    return _state;
}

int TCB::getId() const
{
    return _tid;
}

Priority TCB::getPriority() const
{
    return _pr;
}

void TCB::increaseQuantum()
{
    _quantum++;
}

int TCB::getQuantum() const
{
    return _quantum;
}

void TCB::increaseLockCount()
{
    _lock_count++;
}

void TCB::decreaseLockCount()
{
    _lock_count--;
}

int TCB::getLockCount()
{
    return _lock_count; 
}

void TCB::increasePriority()
{
    // TODO
}

void TCB::decreasePriority()
{
    // TODO
}
