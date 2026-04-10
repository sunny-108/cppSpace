CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -pthread
LDFLAGS := -pthread

SRC := src/main.cpp src/OperationThread.cpp src/Operation.cpp src/BackupOperation.cpp src/RestoreOperation.cpp
TARGET := sliding_pool_demo

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
