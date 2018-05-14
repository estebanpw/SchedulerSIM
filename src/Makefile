CC=gcc
CXX=g++
CFLAGS=-O3 -std=c++11 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -Wall #-DVERBOSE
BIN=../bin

all: scheduler metrics

scheduler: main.cpp
	$(CXX) $(CFLAGS) log-recorder.cpp main.cpp common-functions.cpp io-functions.cpp sysclock.cpp cluster.cpp node.cpp -lpthread core.cpp job.cpp scheduler.cpp policy.cpp -o $(BIN)/scheduler

metrics: log_analyzer.cpp
	$(CXX) $(CFLAGS) log_analyzer.cpp common-functions.cpp -o $(BIN)/log_analyzer 

clean:
	rm -rf $(BIN)/scheduler