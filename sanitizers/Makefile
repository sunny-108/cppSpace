# Makefile for Sanitizer Examples
# Usage: make all          - Build all examples
#        make asan         - Build all ASan examples
#        make tsan         - Build all TSan examples
#        make clean        - Remove all binaries
#        make run-asan-01  - Build and run specific example

CXX = clang++
ASAN_FLAGS = -fsanitize=address -g -O1 -fno-omit-frame-pointer
TSAN_FLAGS = -fsanitize=thread -g -O1

# AddressSanitizer targets
ASAN_SOURCES = $(wildcard asan_*.cpp)
ASAN_BINS = $(ASAN_SOURCES:.cpp=)

# ThreadSanitizer targets
TSAN_SOURCES = $(wildcard tsan_*.cpp)
TSAN_BINS = $(TSAN_SOURCES:.cpp=)

.PHONY: all asan tsan clean run-asan run-tsan

all: asan tsan

asan: $(ASAN_BINS)

tsan: $(TSAN_BINS)

# Build ASan examples
asan_%: asan_%.cpp
	$(CXX) $(ASAN_FLAGS) $< -o $@

# Build TSan examples
tsan_%: tsan_%.cpp
	$(CXX) $(TSAN_FLAGS) $< -o $@

# Run specific examples
run-asan-01: asan_01_buffer_overflow
	./asan_01_buffer_overflow

run-asan-02: asan_02_stack_overflow
	./asan_02_stack_overflow

run-asan-03: asan_03_use_after_free
	./asan_03_use_after_free

run-asan-04: asan_04_double_free
	./asan_04_double_free

run-asan-05: asan_05_memory_leak
	./asan_05_memory_leak

run-tsan-01: tsan_01_basic_race
	./tsan_01_basic_race

run-tsan-02: tsan_02_shared_counter
	./tsan_02_shared_counter

run-tsan-03: tsan_03_lazy_init
	./tsan_03_lazy_init

run-tsan-04: tsan_04_vector_race
	./tsan_04_vector_race

clean:
	rm -f $(ASAN_BINS) $(TSAN_BINS)
	rm -f *.dSYM

# Help target
help:
	@echo "Sanitizer Examples Makefile"
	@echo "Usage:"
	@echo "  make all          - Build all examples"
	@echo "  make asan         - Build all ASan examples"
	@echo "  make tsan         - Build all TSan examples"
	@echo "  make run-asan-01  - Build and run ASan example 01"
	@echo "  make run-tsan-01  - Build and run TSan example 01"
	@echo "  make clean        - Remove all binaries"
