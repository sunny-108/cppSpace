// AddressSanitizer Example 3: Use-After-Free
// Compile: clang++ -fsanitize=address -g -O1 asan_03_use_after_free.cpp -o asan_03
// Run: ./asan_03

#include <iostream>

struct Data {
    int value;
    Data(int v) : value(v) {
        std::cout << "Data created with value: " << value << std::endl;
    }
    ~Data() {
        std::cout << "Data destroyed" << std::endl;
    }
};

int main() {
    std::cout << "=== ASan Example 3: Use-After-Free ===" << std::endl;
    
    Data* ptr = new Data(42);
    
    std::cout << "Value before delete: " << ptr->value << std::endl;
    
    // Free the memory
    delete ptr;
    
    // BUG: Accessing memory after it's been freed!
    std::cout << "Value after delete: " << ptr->value << std::endl;  // Use-after-free!
    
    // BUG: Even worse - modifying freed memory
    ptr->value = 100;  // Use-after-free write!
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with ASan and run
 * 2. Observe "heap-use-after-free" error
 * 3. Fix the bug by:
 *    - Setting ptr = nullptr after delete
 *    - Not accessing ptr after delete
 *    - Using smart pointers (std::unique_ptr, std::shared_ptr)
 * 4. Verify the fix
 * 
 * SAFE ALTERNATIVES:
 * - After delete: ptr = nullptr;
 * - Check: if (ptr) { ... }
 * - Use: std::unique_ptr<Data> ptr = std::make_unique<Data>(42);
 */
