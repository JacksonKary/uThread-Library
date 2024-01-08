#include "CondVar.h"
#include "uthread_private.h"
#include <cassert>
using namespace std;

CondVar::CondVar()
{
    // Nothing to do
}

// Release the lock and block this thread atomically. Thread is woken up when
// signalled or broadcasted
void CondVar::wait(Lock &lock) 
{
    disableInterrupts();

    this->lock = &lock; 
    queue.push(running); // Add to waiting list

    lock._unlock(); // Release the lock
    enableInterrupts();

    uthread_suspend(uthread_self()); // Suspend and switch
    
    lock.lock(); // Back from waiting

}

// Wake up a blocked thread if any is waiting
void CondVar::signal()
{
    if (!queue.empty()) {
        // Get TCB from waiting list
        TCB* chosenTCB = queue.front();
        queue.pop();

        // Notify lock which thread to run
        lock->_signal(chosenTCB);
        // Resume lock
        uthread_resume(chosenTCB->getId());
    }
}

// Wake up all blocked threads if any are waiting
void CondVar::broadcast()
{
    // Signal threads until the waiting queue is empty
    while (!queue.empty()) {
        signal();
    }
}
