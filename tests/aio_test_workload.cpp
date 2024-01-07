#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"
#include "../lib/SpinLock.h"
#include "../lib/async_io.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h> 
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>

using namespace std;
#define SHARED_BUFFER_SIZE 128
#define UTHREAD_TIME_QUANTUM 10000

static int full= 0;
static int write_count = 1000;
static int r_offset = 0;
static int w_offset = 0;

int workload = 2000;
int item = 10000;

static Lock lock;
static CondVar need_space_cv;
static CondVar need_item_cv;
char buffer[SHARED_BUFFER_SIZE];

int rfd;
int wfd;

void* normal_read(void* p){
    if(rfd < 0){
        cout<<"invalid read fd"<<endl;
    }
    while (write_count>0){
        int res;
        lock.lock();
        while(full == 1){
            need_space_cv.wait(lock);
        }
    
        if (res = read(rfd, buffer, sizeof(buffer)) < 0){
            perror("read error");
            write_count =0;
        }

        for(int i= 0; i< workload;i++){
            item -= 3;
            item += 1;
            item /= -1;
        }

        full = 1;
        need_item_cv.signal();

        lock.unlock();

        uthread_yield();
    }

    return nullptr;
}

void* normal_write(void* p){
    if(wfd<0){
        cout<<"invalid write fd"<<endl;
    }
    while(write_count>0){
        lock.lock();
        while(full == 0){
            need_item_cv.wait(lock);
        }
        //cout<<"Buffer: "<<buffer<<endl;
        write(wfd,buffer,sizeof(buffer));
        for(int i= 0; i< workload;i++){
            item -= 3;
            item += 1;
            item /= -1;
        }
        write_count-=1;
        full = 0;
        need_space_cv.signal();
        lock.unlock();

        uthread_yield();
    }
    return nullptr;
}

void* aio_read(void* p){
    while(write_count > 0){
        int ret = async_read(rfd, &buffer, sizeof(buffer), r_offset);
        if(ret == -1){
            write_count = 0;
            break;
        }
        for(int i= 0; i< workload;i++){
            item -= 3;
            item += 1;
            item /= -1;
        }
        r_offset+= SHARED_BUFFER_SIZE;
    }

    return nullptr;
}
void* aio_write(void* p){
    
    while(write_count > 0){
        async_write(wfd, &buffer, sizeof(buffer), w_offset);
        w_offset+= SHARED_BUFFER_SIZE;
        write_count --;
    }
    for(int i= 0; i< workload;i++){
        item -= 3;
        item += 1;
        item /= -1;
    }

    return nullptr;
}

int main(int argc, char* argv[]){
    if (argc != 3)
    {
        cerr << "Usage: ./aio <num_read> <num_write>" << endl;
        cerr << "Example: ./aio 20 20" << endl;
        exit(1);
    }

    int reader = atoi(argv[1]);
    int writer = atoi(argv[2]);

    /*-----------------------
    SYSCALL TESTING
    ----------------------------
    */
    auto threadStart = std::chrono::high_resolution_clock::now();
    if(rfd = open("tests/test.txt", O_RDONLY) <=0){
        perror("Cannot read the file");
    }
    else rfd = open("tests/test.txt", O_RDONLY);

    if(wfd = open("tests/copy2.txt", O_WRONLY|O_APPEND|O_CREAT,0666) <=0){
        perror("Cannot write to the file");
    }
    else wfd = open("tests/copy2.txt", O_WRONLY|O_CREAT,0666);

    int thread_count = 1;

    int ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0)
    {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    int *read_threads = new int[reader];
    for(int i=0; i< reader; i++){
        read_threads[i] = uthread_create(normal_read, nullptr);
        if (read_threads[i] < 0)
        {
            cerr << "Error: Read uthread_create producer" << endl;
        }
    }

    int *write_threads = new int[writer];
    for(int i =0; i< writer;i++){
        write_threads[i] = uthread_create(normal_write, nullptr);
        if (write_threads[i] < 0)
        {
            cerr << "Error: Write uthread_create producer" << endl;
        }
    }

    for(int i =0; i< reader;i++){
        int result = uthread_join(read_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join reader" << endl;
        }
    }
    
    for (int i = 0; i < writer; i++)
    {
        int result = uthread_join(write_threads[i], nullptr);

        if (result < 0)
        {
            cerr << "Error: uthread_join writer" << endl;
        }
    }
    auto threadEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time = threadEnd - threadStart;
    cout<<"Performance of Syscall io:: "<< time.count()<< endl;
    close(rfd);
    close(wfd);
    delete[] read_threads;
    delete[] write_threads;

    /*-----------------------
    AIO TESTING
    ----------------------------
    */
    // reset 
    write_count = 1000;
    memset(buffer, 0, sizeof(buffer));

    threadStart = std::chrono::high_resolution_clock::now();
    if(rfd = open("tests/test2.txt", O_RDONLY) <=0){
        perror("Cannot read the file");
    }
    else rfd = open("tests/test2.txt", O_RDONLY);

    if(wfd = open("tests/aio_copy2.txt", O_WRONLY|O_APPEND|O_CREAT,0666) <=0){
        perror("Cannot write to the file");
    }
    else wfd = open("tests/aio_copy2.txt", O_WRONLY|O_CREAT,0666);

    ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0)
    {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    int *aio_read_threads = new int[reader];
    for(int i=0; i< reader; i++){
        aio_read_threads[i] = uthread_create(aio_read, nullptr);
        if (aio_read_threads[i] < 0)
        {
            cerr << "Error: Read uthread_create producer" << endl;
        }
    }

    int *aio_write_threads = new int[writer];
    for(int i =0; i< writer;i++){
       aio_write_threads[i] = uthread_create(aio_write, nullptr);
        if (aio_write_threads[i] < 0)
        {
            cerr << "Error: AIO Write uthread_create producer" << endl;
        }
    }

        for(int i =0; i< reader;i++){
        int result = uthread_join(aio_read_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "AIO Error: uthread_join reader" << endl;
        }
    }
    
    for (int i = 0; i < writer; i++)
    {
        int result = uthread_join(aio_write_threads[i], nullptr);

        if (result < 0)
        {
            cerr << "AIO Error: uthread_join writer" << endl;
        }
    }
    threadEnd = std::chrono::high_resolution_clock::now();
    time = threadEnd - threadStart;
    cout<<"Performance of Aio:: "<< time.count()<< endl;
    close(rfd);
    close(wfd);
    delete[] aio_read_threads;
    delete[] aio_write_threads;

    return 1;

}