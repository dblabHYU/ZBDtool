CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -Wall -I/home/rocksdb
CFLAGS = -std=c11 -Wall -I/home/rocksdb
LDFLAGS = -lrocksdb -lzbd

CPP_SRCS := $(wildcard src/*.cpp)
C_SRCS := $(wildcard src/*.c)
CPP_OBJS := $(CPP_SRCS:src/%.cpp=builds/%.o)
C_OBJS := $(C_SRCS:src/%.c=builds/%.o)
OBJS := $(CPP_OBJS) $(C_OBJS)
TARGETS := $(CPP_SRCS:src/%.cpp=%) $(C_SRCS:src/%.c=%)

all: $(TARGETS)

print-sources:
		@echo 'print all info'
		@echo CPP Sources: $(CPP_SRCS)
		@echo C Sources: $(C_SRCS)
		@echo Objects: $(OBJS)
		@echo Targets: $(TARGETS)

%: builds/%.o
		$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

builds/%.o: src/%.cpp
		$(CXX) $(CXXFLAGS) -c $< -o $@

builds/%.o: src/%.c
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -f $(OBJS) $(TARGETS)

.PHONY: all print-sources