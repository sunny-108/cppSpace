#include <iostream>
#include <string>

// Function taking an lvalue reference
void process(const std::string& s) {
    std::cout << "Processed lvalue (persistent object): " << s << std::endl;
}

// Function taking an rvalue reference
void process(std::string&& s) {
    std::cout << "Processed rvalue (temporary object): " << s << std::endl;
}

int main() {
    // --- Lvalues ---
    // An "lvalue" (locator value) represents an object that occupies some identifiable location in memory (i.e., has an address).
    // You can think of it as something that appears on the LEFT side of an assignment.
    
    std::string firstName = "John"; // 'firstName' is an lvalue
    std::string lastName = "Doe";   // 'lastName' is an lvalue

    std::cout << "--- Lvalue Examples ---" << std::endl;
    process(firstName); // Calls process(const std::string&)
    process(lastName);  // Calls process(const std::string&)

    // You can take the address of an lvalue
    std::string* ptr = &firstName; 
    std::cout << "Address of firstName: " << ptr << std::endl;


    // --- Rvalues ---
    // An "rvalue" is everything that is not an lvalue. 
    // It typically represents a temporary value that does not persist beyond the expression that uses it.
    // You can think of it as something that appears on the RIGHT side of an assignment (and usually cannot be on the left).

    std::cout << "\n--- Rvalue Examples ---" << std::endl;
    
    // 1. Literals are rvalues (except string literals, which are lvalues arrays, but std::string("...") is an rvalue)
    process("Hello World"); // Implicitly creates a temporary std::string, which is an rvalue. Calls process(std::string&&)

    // 2. Temporary objects returned by functions or operators
    process(firstName + " " + lastName); // The result of concatenation is a temporary string (rvalue). Calls process(std::string&&)

    // 3. Explicitly casted rvalues using std::move
    process(std::move(firstName)); // std::move casts the lvalue 'firstName' to an rvalue. Calls process(std::string&&)

    // Note: You generally cannot take the address of an rvalue
    // std::string* ptr2 = &("Hello"); // Error (conceptually, though string literals are special in C++)
    // int* i = &5; // Error

    return 0;
}
