# uThread Library (C++)

### This is a library for user-level threads with synchronization. It mimics the POSIX thread (pthread) library.

## Description
### Synchronization
uThread mimics the interface of pthread library and runs in user space. This library assumes a uniprocessor. It uses synchronization by the means of:
- Locks
- Spinlocks
- Condition Variables

### Asynchronous I/O
uThread also supports Asynchronous I/O, which prevents blocking of the entire process (all of the other user threads). Our async_read() and async_write() functions provide a mechanism for
supporting asynchronous I/O (refer to async_io.h).
- <code>ssize_t async_read(int fd, void *buf, size_t count, int offset)</code>
- <code>ssize_t async_write(int fd, void *buf, size_t count, int offset)</code>

These functions will initiate an I/O request with the operating system and then switch to another uThread thread until successfully polling for completion.
This means that the asynchronous I/O functions will be synchronous with respect to the calling thread but asynchronous in terms of letting other threads complete work in the meantime.
We make use of the aio_read(), aio_write(), aio_error(), and aio_return() system calls for carrying out asynchronous I/O requests.

### Priority Inversion
With the addition of synchronization mechanisms, the current uThread implementation is susceptible to priority inversion. We plan to implement a solution used by Microsoft
Windows (https://docs.microsoft.com/en-us/windows/win32/procthread/priority-inversion), called
Priority Boosting. Priority Boosting addresses the problem by periodically increasing the priority of low priority threads that are currently holding locks so that they will eventually be scheduled and hopefully release the lock.

We have a priority scheduler and some helper functions in place to do this, but have yet to implement the helpers. The priority scheduler has three priority levels: RED, ORANGE, and GREEN
(RED is highest priority and GREEN is lowest). The scheduler strictly follows priorities by scheduling a thread with the highest priority currently in the ready queue.

Helper functions for modifying the priority of a thread have also been provided (yet to be implemented) through the public functions:
- uthread_increase_priority()
- uthread_decrease_priority()

and private functions:
- \_uthread_increase_priority()
- \_uthread_decrease_priority()
  
in uthread.cpp. Additionally, since we're following the Priority Boosting methodology, the following methods have been added to the TCB (thread control block) class for keeping track of how many Locks a thread is currently holding:
- increaseLockCount()
- decreaseLockCount()
- getLockCount()

## uThread API
- <b>pthread equivalents.</b> Each API provides the same functionality as its pthread counterpart, except that tid is represented as an integer (See uthread.h).  
	- <code>int uthread_create(void *(*start_routine)(void *), void *arg)</code>
	- <code>int uthread_yield(void)</code>
	- <code>int uthread_self(void)</code>
	- <code>int uthread_join(int tid, void **retval)</code>
- <b>uthread control.</b> These APIs allow application developers to have more fine-grained control of thread execution (See uthread.h).  
	- <code>int uthread_init(int time_slice)</code>
	- <code>int uthread_exit(void *retval)</code>
	- <code>int uthread_suspend(int tid)</code>
	- <code>int uthread_resume(int tid)</code>
- <b>Thread Quantum Info</b> Rough equivalent to individual and total thread turn count (See uthread.h).
  - <code>int uthread_get_quantums(int tid)</code>
  - <code>int uthread_get_total_quantums()</code>
- <b>Priority</b> These APIs allow application developers to control priority level of individual threads (See uthread.h).
  - <code>int uthread_increase_priority(int tid)</code> (TODO)
  - <code>int uthread_decrease_priority(int tid)</code> (TODO)
- <b>Synchronization</b> These APIs allow application developers to control the use of locks, spinlocks, and condition variables.
  - <b>Locks</b> (See Lock.h)
    - <code>Lock()</code> (Constructor)
    - <code>void lock()</code>
    - <code>void unlock()</code>
  - <b>SpinLocks</b> (See SpinLock.h)
    - <code>SpinLock()</code> (Constructor)
    - <code>void lock()</code>
    - <code>void unlock()</code>
  - <b>Condition Variables</b> (See CondVar.h)
    - <code>CondVar()</code> (Constructor)
    - <code>void wait(Lock &lock)</code>
    - <code>void signal()</code>
    - <code>void broadcast()</code>

## What's in this directory?
- <code>lib</code> : Folder, which contains the library files
  - <code>CondVar.cpp</code> : Implementation of CondVar class
  - <code>CondVar.h</code> : Header file for CondVar class
  - <code>Lock.cpp</code> : Implementation of Lock class
  - <code>Lock.h</code> : Header file for Lock class
  - <code>SpinLock.cpp</code> : Implementation of SpinLock class
  - <code>SpinLock.h</code> : Header file for SpinLock class
  - <code>TCB.cpp</code> : Implementation of TCB class
  - <code>TCB.h</code> : Header file for TCB class
  - <code>async_io.cpp</code> : Implementation of asynchronous I/O functions
  - <code>async_io.h</code> : Header file for asynchronous I/O functions
  - <code>uthread.cpp</code> : Implementation of uThread library functions
  - <code>uthread.h</code> : Header file for uThread library functions
  - <code>uthread_private.h</code> : Header file for internal uThread library functionality. This should not be accessed directly by user code
- <code>tests</code> : Folder, which contains files for testing
  - <code>aio_test.cpp</code> : Tests synchronous vs asynchronous I/O (Usage: ./aio <num_read> <num_write>)
  - <code>aio_test_workload.cpp</code> : Tests synchronous vs asynchronous I/O across workloads, controlled by global variable (Usage: ./aio <num_read> <num_write>)
  - <code>test.cpp</code> : Tests Lock and Spin Lock (Usage: ./test \<thread number> \<critical section>)
  - <code>test.txt</code> : Text file used by tests to read data from
  - <code>uthread_sync_demo.cpp</code> : Tests Lock, SpinLock, and Condition Variables (Usage: ./uthread-sync-demo <num_producer> <num_consumer>)
- <code>Makefile</code> : Build file to compile library and run test cases. (See all commands below)
    
## Running Tests
The provided make file makes compiling and running tests very simple.
They are also good examples of how to use the uThread library functions.

To run tests, simply open a terminal in the directory containing the Makefile and input the following commands.

To compile all test files at once:

	make

Alternatively, you can compile a specific test file alone with its name:

	make aio_test
	make aio_work
	make test
	make uthread_sync_demo

Now, you can run any of the tests:
- Sync vs Async I/O test:

		./aio <num_read> <num_write>
 
- Sync vs Async (workload):
(Change the global variable <code>workload</code> and remake between tests)

		./aio <num_read> <num_write>  
  
- Lock vs SpinLock:
  
		./test \<thread number> \<critical section>
  
- Lock, SpinLock, Condition Variables:

		./uthread-sync-demo <num_producer> <num_consumer>
