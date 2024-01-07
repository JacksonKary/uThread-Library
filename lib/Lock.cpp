#include "Lock.h"
#include "uthread_private.h"
#include <cassert>
using namespace std;
Lock::Lock() : held(false)
{
    // Nothing to do
}

// Attempt to acquire lock. Grab lock if available, otherwise thread is
// blocked until the lock becomes available
void Lock::lock()
{
    // Implementation is adapted from OSPP textbook figure 5.15 (https://ospp.cs.washington.edu/figures/Synchronization/UniLock.cc)
    disableInterrupts();
    // If lock is already held
    if (held == true) {
        // Push the calling thread onto Lock's waiting queue
        entrance_queue.push(running); // 'running' is a global from uthread.cpp                   
        // Change the calling thread's state to READY prior to switching threads
        running->setState(READY);
        // Run the next thread in uthread ready queue
        switchThreads();
        // Set new running thread's state to RUNNING
        running->setState(RUNNING);
    } else {
        // If Lock initially free, mark it as held
        held = true;
        // Proceed to critical section, holding Lock
    }
    enableInterrupts();
}

// Unlock the lock. Wake up a blocked thread if any is waiting to acquire the
// lock and hand off the lock
void Lock::unlock()
{
    // Implementation is adapted from OSPP textbook figure 5.15 (https://ospp.cs.washington.edu/figures/Synchronization/UniLock.cc)
    disableInterrupts();
    // If there are threads waiting on Lock, wake the first thread
    if (!entrance_queue.empty()) {
        // Move one TCB from waiting to ready
        TCB *next = entrance_queue.front();
        entrance_queue.pop();
        // Possibly redundant, but ensure next's state is READY when it's added to ready queue
        next->setState(READY);
        // Add next thread to the uthread ready queue
        addToReady(next);
    } else {
        // If no thread is waiting on Lock, mark it as free
        held = false;
    }
    enableInterrupts();
}

// Unlock the lock while interrupts have already been disabled
// NOTE: This function should NOT be used by user code. It is only to be used
//       by uthread library code
void Lock::_unlock()
{
    // Same as unlock, but without disabling interrupts
    if (!entrance_queue.empty()) {
        TCB *next = entrance_queue.front();
        entrance_queue.pop();

        next->setState(READY);
        addToReady(next);
    } else {
        held = false;
    }

}

// Let the lock know that it should switch to this thread after the lock has
// been released (following Mesa semantics)
void Lock::_signal(TCB *tcb)
{
    signaled_queue.push(tcb);
}
