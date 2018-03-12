CC=gcc
CXX=g++
CFLAGS=-O3 -std=c++11 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Wall #-DVERBOSE
BIN=../bin

all: scheduler


scheduler: main.c
	$(CXX) $(CFLAGS) log-recorder.cpp main.c common-functions.c io-functions.c sysclock.cpp cluster.cpp node.cpp -lpthread core.cpp job.cpp scheduler.cpp -o $(BIN)/scheduler

clean:
	rm -rf $(BIN)/scheduler
