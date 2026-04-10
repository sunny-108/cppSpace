// AddressSanitizer Example 1: Heap Buffer Overflow
// Compile: clang++ -fsanitize=address -g -O1 asan_01_buffer_overflow.cpp -o asan_01
// Run: ./asan_01

#include <iostream>

int main() {
    std::cout << "=== ASan Example 1: Heap Buffer Overflow ===" << std::endl;
    
    // BUG: Allocate array of 10 integers
    int* arr = new int[10];
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    
    // BUG: Access beyond array bounds (index 10 is out of bounds)
    // Valid indices are 0-9
    std::cout << "Accessing arr[10]: " << arr[10] << std::endl;
    
    // Also try writing out of bounds
    arr[15] = 999;  // BUG: Even worse - writing out of bounds
    
    delete[] arr;
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with: clang++ -fsanitize=address -g -O1 asan_01_buffer_overflow.cpp -o asan_01
 * 2. Run: ./asan_01
 * 3. Observe the ASan error report
 * 4. Fix the bugs by:
 *    - Only accessing valid indices (0-9)
 *    - Or increasing array size if needed
 * 5. Recompile and verify it's fixed
 * 
 * EXPECTED ASan OUTPUT:
 * - Should report "heap-buffer-overflow"
 * - Shows the exact line where the overflow occurred
 * - Shows stack trace
 * - Shows memory layout around the corrupted area
 */
