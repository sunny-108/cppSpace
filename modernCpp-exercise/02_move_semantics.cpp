/**
 * Modern C++ Exercise 02: Move Semantics
 * 
 * Topics covered:
 * - Lvalue vs Rvalue references
 * - Move constructors and move assignment operators
 * - std::move
 * - Perfect forwarding with std::forward
 * - Return value optimization (RVO)
 * - Rule of Five
 * 
 * Compile: g++ -std=c++17 -Wall -Wextra -o move_semantics 02_move_semantics.cpp
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <cstring>

// ============================================================================
// TASK 1: Understanding lvalues and rvalues
// ============================================================================

void demonstrateReferences() {
    std::cout << "\n=== Task 1: Lvalue vs Rvalue References ===\n";
    
    int x = 10;
    int& lref = x;              // Lvalue reference
    // int& lref2 = 20;         // ERROR: cannot bind lvalue ref to rvalue
    
    int&& rref = 20;            // Rvalue reference
    // int&& rref2 = x;         // ERROR: cannot bind rvalue ref to lvalue
    int&& rref3 = std::move(x); // OK: std::move casts lvalue to rvalue
    
    std::cout << "x = " << x << ", lref = " << lref << "\n";
    std::cout << "rref = " << rref << ", rref3 = " << rref3 << "\n";
    
    // TODO: Experiment with these concepts
    // 1. Modify x and observe lref
    // 2. Modify rref and see what happens
    // 3. Try to understand why we need rvalue references
}

// ============================================================================
// TASK 2: Implementing move semantics for a custom class
// ============================================================================

class DynamicArray {
private:
    int* data_;
    size_t size_;
    
public:
    // Constructor
    DynamicArray(size_t size = 0) : data_(nullptr), size_(size) {
        if (size_ > 0) {
            data_ = new int[size_];
            std::fill(data_, data_ + size_, 0);
        }
        std::cout << "Constructor: Created array of size " << size_ << "\n";
    }
    
    // Copy constructor (deep copy)
    DynamicArray(const DynamicArray& other) : size_(other.size_) {
        std::cout << "Copy constructor: Copying array of size " << size_ << "\n";
        if (size_ > 0) {
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        } else {
            data_ = nullptr;
        }
    }
    
    // TODO: Implement move constructor
    // 1. Steal the resources from 'other'
    // 2. Set 'other' to a valid but empty state
    // 3. Add a debug message
    DynamicArray(DynamicArray&& other) noexcept {
        // Your code here
    }
    
    // Copy assignment operator
    DynamicArray& operator=(const DynamicArray& other) {
        std::cout << "Copy assignment: Copying array of size " << other.size_ << "\n";
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            if (size_ > 0) {
                data_ = new int[size_];
                std::copy(other.data_, other.data_ + size_, data_);
            } else {
                data_ = nullptr;
            }
        }
        return *this;
    }
    
    // TODO: Implement move assignment operator
    // 1. Check for self-assignment
    // 2. Delete current resources
    // 3. Steal resources from 'other'
    // 4. Set 'other' to valid but empty state
    // 5. Add a debug message
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        // Your code here
        return *this;
    }
    
    // Destructor
    ~DynamicArray() {
        std::cout << "Destructor: Deleting array of size " << size_ << "\n";
        delete[] data_;
    }
    
    // Utility methods
    size_t size() const { return size_; }
    
    int& operator[](size_t index) { return data_[index]; }
    const int& operator[](size_t index) const { return data_[index]; }
    
    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < size_; ++i) {
            std::cout << data_[i];
            if (i < size_ - 1) std::cout << ", ";
        }
        std::cout << "]\n";
    }
};

void demonstrateMoveSemantics() {
    std::cout << "\n=== Task 2: Move Semantics Demo ===\n";
    
    // TODO: Complete the following:
    // 1. Create a DynamicArray and initialize it
    // 2. Copy it to another array (observe copy constructor)
    // 3. Move it to another array using std::move (observe move constructor)
    // 4. Use move assignment operator
    // 5. Compare the performance implications
    
    // Your code here
}

// ============================================================================
// TASK 3: Move-only types
// ============================================================================

class FileHandle {
private:
    std::string filename_;
    bool is_open_;
    
public:
    explicit FileHandle(const std::string& name) 
        : filename_(name), is_open_(true) {
        std::cout << "Opening file: " << filename_ << "\n";
    }
    
    // Delete copy operations - this is a move-only type
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // TODO: Implement move constructor
    FileHandle(FileHandle&& other) noexcept {
        // Your code here
    }
    
    // TODO: Implement move assignment operator  
    FileHandle& operator=(FileHandle&& other) noexcept {
        // Your code here
        return *this;
    }
    
    ~FileHandle() {
        if (is_open_) {
            std::cout << "Closing file: " << filename_ << "\n";
        }
    }
    
    void close() {
        if (is_open_) {
            std::cout << "Manually closing: " << filename_ << "\n";
            is_open_ = false;
        }
    }
    
    bool isOpen() const { return is_open_; }
    std::string getFilename() const { return filename_; }
};

// TODO: Complete this function
// Return a FileHandle by value (this will use move semantics)
FileHandle openFile(const std::string& name) {
    // Your code here
    return FileHandle(name);
}

void demonstrateMoveOnlyType() {
    std::cout << "\n=== Task 3: Move-Only Types ===\n";
    
    // TODO:
    // 1. Create a FileHandle
    // 2. Try to copy it (should fail to compile)
    // 3. Move it to another FileHandle
    // 4. Store FileHandles in a vector using emplace_back
    
    // Your code here
}

// ============================================================================
// TASK 4: Perfect forwarding
// ============================================================================

class Widget {
private:
    std::string name_;
    int id_;
    
public:
    // Constructor
    Widget(const std::string& name, int id) : name_(name), id_(id) {
        std::cout << "Widget created: " << name_ << " (copy)\n";
    }
    
    Widget(std::string&& name, int id) : name_(std::move(name)), id_(id) {
        std::cout << "Widget created: " << name_ << " (move)\n";
    }
    
    void print() const {
        std::cout << "Widget: " << name_ << " [" << id_ << "]\n";
    }
};

// TODO: Implement a factory function using perfect forwarding
// This function should forward arguments perfectly to Widget constructor
// Use std::forward to preserve lvalue/rvalue-ness
template<typename T>
Widget createWidget(T&& name, int id) {
    // Your code here
    return Widget(std::forward<T>(name), id);
}

// TODO: Implement a variadic template factory with perfect forwarding
template<typename... Args>
Widget makeWidget(Args&&... args) {
    // Your code here - forward all arguments
    return Widget(std::forward<Args>(args)...);
}

void demonstratePerfectForwarding() {
    std::cout << "\n=== Task 4: Perfect Forwarding ===\n";
    
    // TODO:
    // 1. Create widgets with lvalue and rvalue arguments
    // 2. Observe which constructor gets called
    // 3. Test your variadic template factory
    
    // Your code here
}

// ============================================================================
// TASK 5: std::move and std::forward in practice
// ============================================================================

// TODO: Implement a function that processes a vector
// If the vector is an rvalue, move its contents
// If it's an lvalue, copy its contents
template<typename T>
std::vector<T> processVector(std::vector<T> vec) {
    std::cout << "Processing vector of size " << vec.size() << "\n";
    // Do some processing...
    for (auto& item : vec) {
        item *= 2;  // Example: double each element
    }
    return vec;  // RVO or move will happen here
}

void demonstrateMoveInPractice() {
    std::cout << "\n=== Task 5: std::move in Practice ===\n";
    
    // TODO:
    // 1. Create a large vector
    // 2. Pass it to processVector as an rvalue (using std::move)
    // 3. Observe that the original vector is now empty
    // 4. Create another vector and pass it as lvalue
    // 5. Observe that the original vector is still valid
    
    // Your code here
}

// ============================================================================
// TASK 6: Return Value Optimization (RVO)
// ============================================================================

class Heavy {
private:
    std::vector<int> data_;
    std::string name_;
    
public:
    Heavy(const std::string& name, size_t size) 
        : data_(size, 42), name_(name) {
        std::cout << "Heavy object '" << name_ << "' constructed (size: " << size << ")\n";
    }
    
    Heavy(const Heavy& other) : data_(other.data_), name_(other.name_) {
        std::cout << "Heavy object '" << name_ << "' COPIED\n";
    }
    
    Heavy(Heavy&& other) noexcept 
        : data_(std::move(other.data_)), name_(std::move(other.name_)) {
        std::cout << "Heavy object '" << name_ << "' MOVED\n";
    }
    
    size_t size() const { return data_.size(); }
};

// TODO: Create functions that demonstrate RVO
// 1. This should trigger RVO (no copy, no move)
Heavy createHeavy1() {
    return Heavy("RVO", 1000);
}

// 2. This might prevent RVO (depends on compiler)
Heavy createHeavy2(bool flag) {
    Heavy h1("NRVO1", 1000);
    Heavy h2("NRVO2", 2000);
    return flag ? h1 : h2;  // Named RVO might not work here
}

// 3. This should use move semantics
Heavy createHeavy3() {
    Heavy h("Move", 1000);
    // Some processing...
    return std::move(h);  // Explicit move (usually unnecessary!)
}

void demonstrateRVO() {
    std::cout << "\n=== Task 6: Return Value Optimization ===\n";
    
    // TODO:
    // 1. Call each createHeavy function
    // 2. Observe which one copies, moves, or uses RVO
    // 3. Understand when to use std::move and when not to
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Implementing swap with move semantics
// ============================================================================

// TODO: Implement an efficient swap function using move semantics
template<typename T>
void efficientSwap(T& a, T& b) {
    // Your code here - use std::move to avoid copies
}

void demonstrateBonusTask() {
    std::cout << "\n=== Bonus Task: Efficient Swap ===\n";
    
    // TODO: Test your swap implementation with DynamicArray
    // Compare with std::swap
    
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Move Semantics Exercise\n";
    std::cout << "===================================\n";
    
    demonstrateReferences();
    demonstrateMoveSemantics();
    demonstrateMoveOnlyType();
    demonstratePerfectForwarding();
    demonstrateMoveInPractice();
    demonstrateRVO();
    demonstrateBonusTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. Move semantics transfer ownership instead of copying
 * 2. std::move casts an lvalue to an rvalue reference
 * 3. Move constructors/assignment should be noexcept
 * 4. After moving, source object should be in valid but unspecified state
 * 5. Perfect forwarding preserves value category (lvalue/rvalue)
 * 6. RVO eliminates copies/moves entirely when possible
 * 7. Don't use std::move on return values (prevents RVO)
 * 8. Rule of Five: If you define any of destructor, copy constructor,
 *    copy assignment, move constructor, or move assignment, define all five
 */
