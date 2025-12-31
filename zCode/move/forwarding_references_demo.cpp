#include <iostream>
#include <string>
#include <utility>

// --- Understanding the Syntax ---
// T&&  can mean TWO different things depending on context:
//
// 1. If T is a CONCRETE type (like std::string&&), it's an RVALUE REFERENCE (only binds to rvalues)
// 2. If T is a TEMPLATE TYPE (like template<typename T>), it's a FORWARDING REFERENCE (binds to both lvalues and rvalues)

// Example of a regular rvalue reference (NOT a forwarding reference)
void regularRvalueRef(std::string&& s) {
    std::cout << "Regular rvalue reference (only accepts temporaries): " << s << std::endl;
}

// Example of a FORWARDING REFERENCE (Universal Reference)
// The key: T&& where T is deduced by the template
template<typename T>
void forwardingRef(T&& param) {
    // param is a forwarding reference
    // It can accept BOTH lvalues and rvalues
    std::cout << "Forwarding reference received: " << param << std::endl;
}

// --- Why do we need them? Perfect Forwarding! ---
// We want to write wrapper functions that pass arguments to other functions
// while preserving whether they were lvalues or rvalues.

void process(const std::string& s) {
    std::cout << "  -> Called with LVALUE: " << s << std::endl;
}

void process(std::string&& s) {
    std::cout << "  -> Called with RVALUE: " << s << std::endl;
}

// BAD: Without forwarding references - loses information
template<typename T>
void badWrapper(T param) {
    // Always copies! param is always an lvalue inside this function.
    process(param); // Always calls process(const std::string&)
}

// GOOD: With forwarding references + std::forward
template<typename T>
void goodWrapper(T&& param) {
    // std::forward preserves the "lvalue-ness" or "rvalue-ness"
    process(std::forward<T>(param));
}

// --- How does T&& deduce the type? ---
// RULE 1: If you pass an LVALUE, T deduces to "Type&" (lvalue reference)
//         So T&& becomes "Type& &&" which collapses to "Type&"
//
// RULE 2: If you pass an RVALUE, T deduces to "Type" (no reference)
//         So T&& becomes "Type&&" (rvalue reference)
//
// This is called "Reference Collapsing"

template<typename T>
void showType(T&& param) {
    std::cout << "Type T deduced as: " << typeid(T).name() << std::endl;
}

int main() {
    std::string name = "Alice";
    
    std::cout << "=== Regular Rvalue Reference ===\n";
    // regularRvalueRef(name); // ERROR! Cannot bind lvalue to rvalue reference
    regularRvalueRef("Bob");   // OK: string literal creates temporary
    regularRvalueRef(std::move(name)); // OK: std::move casts to rvalue
    name = "Alice"; // Reset
    
    std::cout << "\n=== Forwarding Reference (accepts both!) ===\n";
    forwardingRef(name);           // OK: accepts lvalue
    forwardingRef("Charlie");      // OK: accepts rvalue
    forwardingRef(std::move(name)); // OK: accepts rvalue
    name = "Alice"; // Reset
    
    std::cout << "\n=== Bad Wrapper (loses rvalue information) ===\n";
    badWrapper(name);              // Calls process(const std::string&)
    badWrapper(std::string("David")); // Still calls process(const std::string&) - INEFFICIENT!
    
    std::cout << "\n=== Good Wrapper (perfect forwarding) ===\n";
    goodWrapper(name);              // Calls process(const std::string&)
    goodWrapper(std::string("Eve")); // Calls process(std::string&&) - EFFICIENT!
    
    std::cout << "\n=== Type Deduction Examples ===\n";
    int x = 42;
    showType(x);           // T = int&    (lvalue passed)
    showType(42);          // T = int     (rvalue passed)
    showType(std::move(x)); // T = int     (rvalue passed)
    
    std::cout << "\n=== Key Takeaway ===\n";
    std::cout << "Forwarding reference (T&&) where T is a template parameter:\n";
    std::cout << "  - Binds to BOTH lvalues and rvalues\n";
    std::cout << "  - Used with std::forward for perfect forwarding\n";
    std::cout << "  - Preserves the value category of arguments\n";
    
    return 0;
}
