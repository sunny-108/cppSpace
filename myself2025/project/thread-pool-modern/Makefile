CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -pthread
LDFLAGS := -pthread

SRC := src/main.cpp src/Command.cpp src/Thread.cpp src/ThreadPoolMgr.cpp
TARGET := thread_pool_demo

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
