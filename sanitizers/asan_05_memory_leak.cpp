// AddressSanitizer Example 5: Memory Leak
// Compile: clang++ -fsanitize=address -g -O1 asan_05_memory_leak.cpp -o asan_05
// Run: ./asan_05
// Or: ASAN_OPTIONS=detect_leaks=1 ./asan_05

#include <iostream>

void leak_memory() {
    // BUG: Allocate but never free
    int* leaked = new int[100];
    for (int i = 0; i < 100; i++) {
        leaked[i] = i;
    }
    // Missing: delete[] leaked;
    
    std::cout << "Allocated 100 integers but didn't free them" << std::endl;
}

void another_leak() {
    // BUG: Multiple allocations without cleanup
    for (int i = 0; i < 10; i++) {
        int* temp = new int(i);
        // Missing: delete temp;
    }
    std::cout << "Leaked 10 more integers" << std::endl;
}

int main() {
    std::cout << "=== ASan Example 5: Memory Leak ===" << std::endl;
    
    leak_memory();
    another_leak();
    
    std::cout << "Program ending - check for leak report" << std::endl;
    return 0;
}

/* EXERCISE:
 * 1. Compile with ASan: clang++ -fsanitize=address -g -O1 asan_05_memory_leak.cpp -o asan_05
 * 2. Run: ./asan_05 (LSan is enabled by default on Linux, check on macOS)
 * 3. Observe memory leak report at program exit
 * 4. Fix the bugs by:
 *    - Adding appropriate delete/delete[] calls
 *    - Using smart pointers
 *    - Using RAII containers (std::vector, std::unique_ptr)
 * 5. Verify no leaks reported
 * 
 * LEAK SANITIZER (LSan):
 * - Part of ASan on Linux (default enabled)
 * - On macOS: may need explicit ASAN_OPTIONS=detect_leaks=1
 * - Reports leaks at program termination
 * - Shows allocation stack trace
 * 
 * SAFE ALTERNATIVES:
 * - std::vector<int> vec(100);
 * - std::unique_ptr<int[]> ptr(new int[100]);
 * - Always pair new with delete, new[] with delete[]
 */
