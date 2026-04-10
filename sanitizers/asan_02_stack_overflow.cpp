// AddressSanitizer Example 2: Stack Buffer Overflow
// Compile: clang++ -fsanitize=address -g -O1 asan_02_stack_overflow.cpp -o asan_02
// Run: ./asan_02

#include <iostream>
#include <cstring>

void vulnerable_function() {
    char buffer[10];
    
    // BUG: strcpy doesn't check bounds
    // This string is longer than buffer size
    const char* input = "This is a very long string that will overflow";
    strcpy(buffer, input);  // BUG: Buffer overflow!
    
    std::cout << "Buffer content: " << buffer << std::endl;
}

int main() {
    std::cout << "=== ASan Example 2: Stack Buffer Overflow ===" << std::endl;
    
    vulnerable_function();
    
    return 0;
}

/* EXERCISE:
 * 1. Compile with ASan: clang++ -fsanitize=address -g -O1 asan_02_stack_overflow.cpp -o asan_02
 * 2. Run: ./asan_02
 * 3. Observe the stack-buffer-overflow error
 * 4. Fix the bug by:
 *    - Using strncpy with size limit
 *    - Or using std::string instead
 *    - Or increasing buffer size
 * 5. Verify the fix
 * 
 * SAFE ALTERNATIVES:
 * - strncpy(buffer, input, sizeof(buffer) - 1); buffer[9] = '\0';
 * - std::string buffer = input;
 * - char buffer[50];  // Larger buffer
 */
