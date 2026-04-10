// AddressSanitizer Example 4: Double Free
// Compile: clang++ -fsanitize=address -g -O1 asan_04_double_free.cpp -o asan_04
// Run: ./asan_04

#include <iostream>

int main() {
    std::cout << "=== ASan Example 4: Double Free ===" << std::endl;
    
    int* ptr = new int(42);
    std::cout << "Value: " << *ptr << std::endl;
    
    // First delete - OK
    delete ptr;
    std::cout << "First delete completed" << std::endl;
    
    // BUG: Deleting the same pointer again!
    delete ptr;  // Double free - undefined behavior!
    std::cout << "Second delete completed" << std::endl;
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with ASan and run
 * 2. Observe "attempting double-free" error
 * 3. Fix the bug by:
 *    - Setting ptr = nullptr after first delete
 *    - Only deleting once
 *    - Using smart pointers
 * 4. Verify the fix
 * 
 * WHY IT'S DANGEROUS:
 * - Corrupts heap metadata
 * - Can cause crashes in unrelated code
 * - Security vulnerability (exploitable)
 * 
 * SAFE PATTERN:
 * delete ptr;
 * ptr = nullptr;  // Deleting nullptr is safe
 */
