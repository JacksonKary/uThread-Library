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
    queue.push(running); //add to waiting list

    lock._unlock(); //release the lock
    enableInterrupts();

    uthread_suspend(uthread_self()); // suspend and switch
    
    lock.lock(); //back from waiting

}

// Wake up a blocked thread if any is waiting
void CondVar::signal()
{
    if (!queue.empty()) {
        // get TCB from waiting list
        TCB* chosenTCB = queue.front();
        queue.pop();

        // notify lock which thread to run
        lock->_signal(chosenTCB);
        // resume lock
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
