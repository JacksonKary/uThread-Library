#include "async_io.h"
#include "uthread.h"
#include <aio.h>
#include <errno.h>

#include <iostream>
#define FAIL -1
#define SUCCESS 0
#define THREAD_ERROR "thread library error: "
#define SYS_ERROR "system error: "

/**
 * Possibly just add these switch cases to the function in uthread.cpp
 */
void aio_printError(int type, std::string pre)
{
    switch (type)
    {
    case EAGAIN:
    {
        std::cerr << pre << "out of resources - there is no data available right now" << std::endl;
        break;
    }
    case EBADF:
    {
        std::cerr << pre << "aio_filedes is not a valid file descriptor open for reading/writing" << std::endl;
        break;
    }
    case EFBIG:
    {
        std::cerr << pre << "starting position is at or beyond the max offset for this file" << std::endl;
        break;
    }
    case EOVERFLOW:
    {
        std::cerr << pre << "starting position is past the max offset for this file" << std::endl;
        break;
    }
    case EINVAL:
    {
        std::cerr << pre << "One or more of aio_offset, aio_reqprio, aio_nbytes are invalid" << std::endl;
        break;
    }
    case ENOSYS: // This case should never really occur
    {
        std::cerr << SYS_ERROR << "One of aio_return, aio_read, aio_write is not implemented" << std::endl;
        break;
    }
    case ECANCELED:
    {
        std::cerr << pre << "the read request was canceled" << std::endl;
        break;
    }
    default:
        break;
    }
}

// Carry out an asynchronous read request where this thread will be blocked
// while servicing the read but other ready threads will be scheduled
// Input:
// - fd: File descriptor
// - buf: Buffer to store read in
// - count: Number of bytes to read
// - offset: File offset to start at
// Output:
// - Number of bytes read on success, -1 on failure
ssize_t async_read(int fd, void *buf, size_t count, int offset)
{
    /* Consider how this should be used */
    struct aiocb async_read_req = {
        .aio_fildes = fd,
        .aio_buf = buf,
        .aio_nbytes = count,
        .aio_offset = offset
    };
    // Call aio_read and check for error
    if (aio_read(&async_read_req) == FAIL) {
        aio_printError(errno, THREAD_ERROR);
        return FAIL;
    }
    // Poll for completion
    int aio_err;
    while ((aio_err = aio_error(&async_read_req)) == EINPROGRESS) {
        // Avoid busy-waiting
        uthread_yield();
    }

    // Check aio_err for errors
    if (aio_err == ECANCELED) {
        aio_printError(ECANCELED, SYS_ERROR);
        return FAIL;
    } else if (aio_err > 0) {
        std::cerr << SYS_ERROR << "asyncronous I/O operation failed" << std::endl;
        return FAIL;
    }

    // aio_error will now have returned 0
    int bytes_read = aio_return(&async_read_req);
    if (bytes_read == FAIL) {
        aio_printError(errno, THREAD_ERROR);
        return FAIL;
    }

    return bytes_read;
}

// Carry out an asynchronous write request where this thread will be blocked
// while servicing the write but other ready threads will be scheduled
// Input:
// - fd: File descriptor
// - buf: Buffer containing data to write to file
// - count: Number of bytes to write
// - offset: File offset to start at
// Output:
// - Number of bytes written on success, -1 on failure
ssize_t async_write(int fd, void *buf, size_t count, int offset)
{
    struct aiocb async_write_req = {
        .aio_fildes = fd,
        .aio_buf = buf,
        .aio_nbytes = count,
        .aio_offset = offset
    };
    // Call aio_write and check for error
    if (aio_write(&async_write_req) == FAIL) {
        aio_printError(errno, THREAD_ERROR);
        return FAIL;
    }
    // Poll for completion
    int aio_err;
    while ((aio_err = aio_error(&async_write_req)) == EINPROGRESS) {
        // Avoid busy-waiting
        uthread_yield();
    }

    // Check aio_err for errors
    if (aio_err == ECANCELED) {
        aio_printError(ECANCELED, SYS_ERROR);
        return FAIL;
    } else if (aio_err > 0) {
        std::cerr << SYS_ERROR << "asyncronous I/O operation failed" << std::endl;
        return FAIL;
    }

    // aio_error will now have returned 0
    int bytes_written = aio_return(&async_write_req);
    if (bytes_written == FAIL) {
        aio_printError(errno, THREAD_ERROR);
        return FAIL;
    }

    return bytes_written;
}
