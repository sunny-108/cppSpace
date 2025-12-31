/**
 * Modern C++ Exercise 01: Smart Pointers
 * 
 * Topics covered:
 * - std::unique_ptr
 * - std::shared_ptr
 * - std::weak_ptr
 * - RAII principles
 * - Custom deleters
 * - make_unique and make_shared
 * 
 * Compile: g++ -std=c++17 -Wall -Wextra -o smart_pointers 01_smart_pointers.cpp
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>

// ============================================================================
// TASK 1: Basic unique_ptr usage
// ============================================================================

class Resource {
private:
    std::string name_;
    int value_;
public:
    Resource(const std::string& name, int value) 
        : name_(name), value_(value) {
        std::cout << "Resource '" << name_ << "' created\n";
    }
    
    ~Resource() {
        std::cout << "Resource '" << name_ << "' destroyed\n";
    }
    
    void use() const {
        std::cout << "Using " << name_ << " (value: " << value_ << ")\n";
    }
    
    std::string getName() const { return name_; }
    int getValue() const { return value_; }
};

// TODO: Complete this function to demonstrate unique_ptr
// 1. Create a unique_ptr to a Resource using make_unique
// 2. Use the resource
// 3. Transfer ownership to another unique_ptr using std::move
// 4. Verify the original pointer is now null
void demonstrateUniquePtr() {
    std::cout << "\n=== Task 1: unique_ptr Demo ===\n";
    
    // Your code here
    
}

// ============================================================================
// TASK 2: unique_ptr with custom deleter
// ============================================================================

// Custom deleter that logs when called
struct CustomDeleter {
    void operator()(Resource* ptr) const {
        std::cout << "Custom deleter called for: " << ptr->getName() << "\n";
        delete ptr;
    }
};

// TODO: Complete this function
// 1. Create a unique_ptr with a custom deleter
// 2. Use a lambda as a custom deleter
void demonstrateCustomDeleter() {
    std::cout << "\n=== Task 2: Custom Deleter ===\n";
    
    // Your code here
    
}

// ============================================================================
// TASK 3: shared_ptr basics
// ============================================================================

class Node {
public:
    int data;
    std::shared_ptr<Node> next;
    
    Node(int d) : data(d), next(nullptr) {
        std::cout << "Node " << data << " created\n";
    }
    
    ~Node() {
        std::cout << "Node " << data << " destroyed\n";
    }
};

// TODO: Complete this function
// 1. Create a shared_ptr using make_shared
// 2. Create multiple shared_ptrs pointing to the same object
// 3. Print the use_count() at different stages
// 4. Show that the object is destroyed only when all shared_ptrs go out of scope
void demonstrateSharedPtr() {
    std::cout << "\n=== Task 3: shared_ptr Demo ===\n";
    
    // Your code here
    
}

// ============================================================================
// TASK 4: weak_ptr to break circular references
// ============================================================================

class Person {
public:
    std::string name;
    std::shared_ptr<Person> partner;  // This can cause circular reference!
    // std::weak_ptr<Person> partner;  // Use this instead
    
    Person(const std::string& n) : name(n) {
        std::cout << name << " created\n";
    }
    
    ~Person() {
        std::cout << name << " destroyed\n";
    }
    
    void setPartner(std::shared_ptr<Person> p) {
        partner = p;
    }
};

// TODO: Complete this function
// 1. Create two Person objects using shared_ptr
// 2. Make them partners (observe the memory leak with shared_ptr)
// 3. Change Person::partner to weak_ptr and fix the circular reference
// 4. Demonstrate using lock() to access the weak_ptr
void demonstrateWeakPtr() {
    std::cout << "\n=== Task 4: weak_ptr Demo ===\n";
    
    // Your code here
    
}

// ============================================================================
// TASK 5: Smart pointers in containers
// ============================================================================

// TODO: Complete this function
// 1. Create a vector of unique_ptr<Resource>
// 2. Add resources to the vector using emplace_back and make_unique
// 3. Access and use resources from the vector
// 4. Demonstrate that you cannot copy the vector (only move)
void demonstrateSmartPtrInContainers() {
    std::cout << "\n=== Task 5: Smart Pointers in Containers ===\n";
    
    // Your code here
    
}

// ============================================================================
// TASK 6: Factory pattern with smart pointers
// ============================================================================

class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() const = 0;
    virtual double area() const = 0;
};

class Circle : public Shape {
    double radius_;
public:
    Circle(double r) : radius_(r) {}
    void draw() const override {
        std::cout << "Drawing Circle with radius " << radius_ << "\n";
    }
    double area() const override {
        return 3.14159 * radius_ * radius_;
    }
};

class Rectangle : public Shape {
    double width_, height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    void draw() const override {
        std::cout << "Drawing Rectangle " << width_ << "x" << height_ << "\n";
    }
    double area() const override {
        return width_ * height_;
    }
};

// TODO: Complete this factory function
// Return a unique_ptr<Shape> based on the type parameter
// type "circle" -> Circle with given radius
// type "rectangle" -> Rectangle with given width and height
std::unique_ptr<Shape> createShape(const std::string& type, double param1, double param2 = 0) {
    // Your code here
    return nullptr;
}

void demonstrateFactory() {
    std::cout << "\n=== Task 6: Factory Pattern ===\n";
    
    // TODO: Use the factory to create shapes and demonstrate polymorphism
    
}

// ============================================================================
// BONUS TASK: Implement a simple smart pointer
// ============================================================================

// TODO: Implement a basic smart pointer class template
// It should:
// 1. Take ownership of a raw pointer in constructor
// 2. Delete the pointer in destructor
// 3. Provide * and -> operators
// 4. Prevent copying but allow moving
// 5. Provide a release() method

template<typename T>
class SimpleUniquePtr {
private:
    T* ptr_;
    
public:
    // Constructor
    explicit SimpleUniquePtr(T* ptr = nullptr) : ptr_(ptr) {}
    
    // Destructor
    ~SimpleUniquePtr() {
        // Your code here
    }
    
    // Delete copy constructor and copy assignment
    SimpleUniquePtr(const SimpleUniquePtr&) = delete;
    SimpleUniquePtr& operator=(const SimpleUniquePtr&) = delete;
    
    // Move constructor
    SimpleUniquePtr(SimpleUniquePtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    // Move assignment
    SimpleUniquePtr& operator=(SimpleUniquePtr&& other) noexcept {
        // Your code here
        return *this;
    }
    
    // Dereference operators
    T& operator*() const { 
        // Your code here
    }
    
    T* operator->() const { 
        // Your code here
    }
    
    // Get raw pointer
    T* get() const { 
        // Your code here
    }
    
    // Release ownership
    T* release() {
        // Your code here
    }
    
    // Check if pointer is valid
    explicit operator bool() const {
        // Your code here
    }
};

void demonstrateBonusTask() {
    std::cout << "\n=== Bonus Task: Custom Smart Pointer ===\n";
    
    // TODO: Test your SimpleUniquePtr implementation
    
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Smart Pointers Exercise\n";
    std::cout << "===================================\n";
    
    demonstrateUniquePtr();
    demonstrateCustomDeleter();
    demonstrateSharedPtr();
    demonstrateWeakPtr();
    demonstrateSmartPtrInContainers();
    demonstrateFactory();
    demonstrateBonusTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * EXPECTED OUTPUT (approximately):
 * 
 * === Task 1: unique_ptr Demo ===
 * Resource 'Task1' created
 * Using Task1 (value: 42)
 * Resource 'Task1' destroyed
 * 
 * === Task 2: Custom Deleter ===
 * Resource 'Custom' created
 * Custom deleter called for: Custom
 * Resource 'Custom' destroyed
 * 
 * === Task 3: shared_ptr Demo ===
 * Node 1 created
 * use_count: 1
 * use_count: 2
 * use_count: 3
 * Node 1 destroyed
 * 
 * === Task 4: weak_ptr Demo ===
 * Alice created
 * Bob created
 * Alice destroyed
 * Bob destroyed
 * 
 * === Task 5: Smart Pointers in Containers ===
 * Resource 'R1' created
 * Resource 'R2' created
 * Using R1 (value: 1)
 * Using R2 (value: 2)
 * Resource 'R2' destroyed
 * Resource 'R1' destroyed
 * 
 * === Task 6: Factory Pattern ===
 * Drawing Circle with radius 5
 * Drawing Rectangle 4x6
 * 
 * === Bonus Task: Custom Smart Pointer ===
 * Resource 'Bonus' created
 * Using Bonus (value: 999)
 * Resource 'Bonus' destroyed
 */
