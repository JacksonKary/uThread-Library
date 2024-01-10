CC = g++
CFLAGS = -g -lrt --std=c++14
# Remove lrt for MacOS
DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h async_io.h
OBJ = ./lib/TCB.o ./lib/uthread.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o
MAIN_OBJ_UTHRAD_SYNC = ./tests/uthread_sync_demo.o
MAIN_OBJ_TEST = ./tests/test.o
MAIN_OBJ_WR = ./tests/aio_test.o
MAIN_OBJ_WL = ./tests/aio_test_workload.o


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: uthread-sync-demo test aio_test aio_work

uthread-sync-demo: $(OBJ) $(MAIN_OBJ_UTHRAD_SYNC)
	$(CC) -o $@ $^ $(CFLAGS)

test: $(OBJ) $(MAIN_OBJ_TEST)
	$(CC) -o $@ $^ $(CFLAGS)

aio_test: $(OBJ) $(MAIN_OBJ_WR)
	$(CC) -o $@ $^ $(CFLAGS)

aio_work: $(OBJ) $(MAIN_OBJ_WL)
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f ./lib/*.o
	rm -f ./tests/*.o
	rm -f *.o uthread-sync-demo
	rm -f *.o test
	rm -f *.o aio_test
	rm -f *.o aio_work
	rm -f ./tests/copy.txt
	rm -f ./tests/copy2.txt
	rm -f ./tests/aio_copy.txt
	rm -f ./tests/aio_copy2.txt
