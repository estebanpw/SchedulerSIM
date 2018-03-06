CC=gcc
CXX=g++
CFLAGS=-g -std=c++11 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Wall #-DVERBOSE
BIN=../bin

all: scheduler


scheduler: main.c
	$(CXX) $(CFLAGS) main.c common-functions.c io-functions.c sysclock.cpp cluster.cpp node.cpp core.cpp job.cpp log.cpp scheduler.cpp -o $(BIN)/scheduler

clean:
	rm -rf $(BIN)/CHROMEISTER
