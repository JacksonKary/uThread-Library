#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"
#include "../lib/SpinLock.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define TOTAL 1000000

//shared variable
static int item_count = 0;
static Lock lock;
static SpinLock spin_lock;
int critical = 0;

void* milkSpin(void* arg)
{
    while(item_count < TOTAL)
    {
        spin_lock.lock();
        for(int i= 0; i< critical;i++){
        }
        item_count ++;
        spin_lock.unlock();
        uthread_yield();
    }
    return NULL;
}

void* milkLock(void* arg)
{
    while(item_count < TOTAL)
    {
        lock.lock();
        for(int i= 0; i< critical;i++){
        }
        item_count ++;
        lock.unlock();
        uthread_yield();
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if (argc != 3)
    {
        cerr << "Usage: ./test <thread number> <critical section>" << endl;
        cerr << "Example: ./uthread-sync-demo  20 0" << endl;
        exit(1);
    }

    int thread_count = atoi(argv[1]);
    critical = atoi(argv[2]);
    auto threadStart = std::chrono::high_resolution_clock::now();
    int ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0)
    {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    int *spin_threads = new int[thread_count];
    for(int i=0;i<thread_count; i++){
        spin_threads[i] = uthread_create(milkSpin, nullptr);
        if(spin_threads[i]<0){
             cerr << "Error: uthread_create error" << endl;
        }
    }

    for(int i=0;i<thread_count; i++){
        int result = uthread_join(spin_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join " << endl;
        }
    }

    auto threadEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time = threadEnd - threadStart;
    cout<<"Performance of spinLock:: "<< time.count()<< endl;

    delete[] spin_threads;



    threadStart = std::chrono::high_resolution_clock::now();
    ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0)
    {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    int* threads = new int[thread_count];
    for(int i=0;i<thread_count; i++){
        threads[i] = uthread_create(milkLock, nullptr);
        if(threads[i]<0){
             cerr << "Error: uthread_create error" << endl;
        }
    }

    for(int i=0;i<thread_count; i++){
        int result = uthread_join(threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join " << endl;
        }
    }
    threadEnd = std::chrono::high_resolution_clock::now();
   time = threadEnd - threadStart;
    cout<<"Performance of Lock:: "<< time.count()<< endl;
    delete[] threads;



    return 0;

}