#ifndef TCB_H
#define TCB_H
// Define _XOPEN_SOURCE for MacOS
// #define _XOPEN_SOURCE

#include "uthread.h"
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>

extern void stub(void *(*start_routine)(void *), void *arg);

enum State
{
    READY,
    RUNNING,
    BLOCK
};

/*
 * The thread
 */
class TCB
{

public:
    /**
     * Constructor for TCB. Allocate a thread stack and setup the thread
     * context to call the stub function
     * @param tid id for the new thread
     * @param pr priority for the new thread
     * @param f the thread function that get no args and return nothing
     * @param arg the thread function argument
     * @param state current state for the new thread
     */
    TCB(int tid, Priority pr, void *(*start_routine)(void *arg), void *arg, State state);

    /**
     * thread deconstructor
     */
    ~TCB();

    /**
     * Function to set the thread state
     * @param state the new state for our thread
     */
    void setState(State state);

    /**
     * Function that get the state of the thread
     * @return the current state of the thread
     */
    State getState() const;

    /**
     * Function that get the ID of the thread
     * @return the ID of the thread
     */
    int getId() const;

    /**
     * Function that get the priority of the thread
     * @return the priority of the thread
     */
    Priority getPriority() const;

    /**
     * Function to increase the quantum of the thread
     */
    void increaseQuantum();

    /**
     * Function that get the quantum of the thread
     * @return the current quantum of the thread
     */
    int getQuantum() const;

    /**
     * Function that increments the thread's lock count
     */
    void increaseLockCount();

    /**
     * function that decrements the thread's lock count
     */
    void decreaseLockCount();

    /**
     * Function that returns the number of locks held by this thread
     */
    int getLockCount();

    /**
     * Function that increases the thread's priority by one
     */
    void increasePriority();

    /**
     * Function that decreases the thread's priority by one
     */
    void decreasePriority();

    /**
     * Context of the thread.
     *
     * Context must be public because get and set context must be called within
     * the same function for guaranteed correctness.
     */
    ucontext_t _context; // The thread's saved context

private:
    int _tid;        // The thread id number.
    Priority _pr;    // The priority of the thread (Red, orange or green)
    int _quantum;    // The time interval, as explained in the pdf.
    State _state;    // The state of the thread
    int _lock_count; // The number of locks held by the thread
    char *_stack;    // The thread's stack
};

#endif /* TCB_H */
