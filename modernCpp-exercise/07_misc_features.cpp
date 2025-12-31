/**
 * Modern C++ Exercise 07: Miscellaneous Modern Features
 * 
 * Topics covered:
 * - Auto and decltype
 * - Range-based for loops
 * - Structured bindings (C++17)
 * - std::optional, std::variant, std::any (C++17)
 * - if/switch with initializers (C++17)
 * - constexpr and consteval (C++20)
 * - std::span (C++20)
 * - Three-way comparison / spaceship operator (C++20)
 * - Designated initializers (C++20)
 * 
 * Compile: g++ -std=c++20 -Wall -Wextra -o misc_features 07_misc_features.cpp
 */

#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <any>
#include <string>
#include <span>
#include <compare>

// ============================================================================
// TASK 1: Auto and decltype
// ============================================================================

void demonstrateAutoAndDecltype() {
    std::cout << "\n=== Task 1: Auto and Decltype ===\n";
    
    // TODO: Use auto for type deduction
    
    // TODO: Use auto with iterators
    
    // TODO: Use auto with lambda
    
    // TODO: Use decltype to deduce type from expression
    
    // TODO: Use decltype(auto) for perfect return type deduction
    
    // TODO: Demonstrate trailing return type
    
    // Your code here
}

// ============================================================================
// TASK 2: Range-based for loops
// ============================================================================

void demonstrateRangeFor() {
    std::cout << "\n=== Task 2: Range-Based For Loops ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // TODO: Iterate by value
    
    // TODO: Iterate by const reference
    
    // TODO: Iterate by reference to modify elements
    
    // TODO: Use structured binding with map
    std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}};
    
    // Your code here
}

// ============================================================================
// TASK 3: Structured bindings (C++17)
// ============================================================================

struct Point {
    double x, y, z;
};

std::pair<int, std::string> getIdAndName() {
    return {42, "John"};
}

void demonstrateStructuredBindings() {
    std::cout << "\n=== Task 3: Structured Bindings ===\n";
    
    // TODO: Destructure pair
    
    // TODO: Destructure tuple
    
    // TODO: Destructure struct
    
    // TODO: Use with map iteration
    
    // TODO: Use with array
    
    // Your code here
}

// ============================================================================
// TASK 4: std::optional (C++17)
// ============================================================================

// TODO: Implement a function that returns optional value
std::optional<int> findIndex(const std::vector<int>& vec, int target) {
    // Your code here
    return std::nullopt;
}

// TODO: Implement a function that parses integer from string
std::optional<int> parseInteger(const std::string& str) {
    // Your code here - return empty optional on failure
    return std::nullopt;
}

class User {
public:
    std::string name;
    std::optional<std::string> email;  // Email is optional
    std::optional<int> age;
    
    User(const std::string& n) : name(n) {}
};

void demonstrateOptional() {
    std::cout << "\n=== Task 4: std::optional ===\n";
    
    // TODO: Create optional values
    
    // TODO: Check if optional has value
    
    // TODO: Access value safely
    
    // TODO: Use value_or for default value
    
    // TODO: Use optional in if statement
    
    // TODO: Chain optional operations
    
    // Your code here
}

// ============================================================================
// TASK 5: std::variant (C++17)
// ============================================================================

using Number = std::variant<int, double, std::string>;

// TODO: Implement visitor for variant
struct NumberPrinter {
    void operator()(int i) const {
        std::cout << "Integer: " << i << "\n";
    }
    
    void operator()(double d) const {
        std::cout << "Double: " << d << "\n";
    }
    
    void operator()(const std::string& s) const {
        std::cout << "String: " << s << "\n";
    }
};

// TODO: Implement a function that processes variant
void processNumber(const Number& num) {
    // Your code here - use std::visit
}

void demonstrateVariant() {
    std::cout << "\n=== Task 5: std::variant ===\n";
    
    // TODO: Create variants with different types
    
    // TODO: Access variant value using get
    
    // TODO: Check variant type using holds_alternative
    
    // TODO: Use std::visit with visitor
    
    // TODO: Use std::visit with lambda
    
    // Your code here
}

// ============================================================================
// TASK 6: std::any (C++17)
// ============================================================================

void demonstrateAny() {
    std::cout << "\n=== Task 6: std::any ===\n";
    
    // TODO: Create any values
    
    // TODO: Check type using type()
    
    // TODO: Extract value using any_cast
    
    // TODO: Handle bad_any_cast exception
    
    // TODO: Reset any to empty
    
    // Your code here
}

// ============================================================================
// TASK 7: if and switch with initializers (C++17)
// ============================================================================

void demonstrateInitializers() {
    std::cout << "\n=== Task 7: if/switch with Initializers ===\n";
    
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};
    
    // TODO: Use if with initializer
    // if (auto it = scores.find("Alice"); it != scores.end()) { ... }
    
    // TODO: Use if with initializer and structured binding
    
    // TODO: Use switch with initializer
    
    // Your code here
}

// ============================================================================
// TASK 8: constexpr and consteval (C++20)
// ============================================================================

// TODO: Implement constexpr factorial
constexpr int factorial(int n) {
    // Your code here
}

// TODO: Implement constexpr fibonacci
constexpr int fibonacci(int n) {
    // Your code here
}

// TODO: Implement consteval function (must be evaluated at compile time)
consteval int square(int n) {
    return n * n;
}

// TODO: Implement constexpr class
class ConstexprVector {
private:
    int x_, y_;
    
public:
    constexpr ConstexprVector(int x, int y) : x_(x), y_(y) {}
    
    constexpr int dot(const ConstexprVector& other) const {
        // Your code here
    }
    
    constexpr int lengthSquared() const {
        // Your code here
    }
};

void demonstrateConstexpr() {
    std::cout << "\n=== Task 8: constexpr and consteval ===\n";
    
    // TODO: Use constexpr values
    constexpr int fact5 = factorial(5);
    
    // TODO: Use consteval function
    
    // TODO: Use constexpr class
    
    // TODO: Demonstrate compile-time vs runtime evaluation
    
    // Your code here
}

// ============================================================================
// TASK 9: std::span (C++20)
// ============================================================================

// TODO: Implement function that uses span
void printSpan(std::span<const int> data) {
    // Your code here
}

// TODO: Implement function that modifies through span
void doubleValues(std::span<int> data) {
    // Your code here
}

void demonstrateSpan() {
    std::cout << "\n=== Task 9: std::span ===\n";
    
    int arr[] = {1, 2, 3, 4, 5};
    std::vector<int> vec = {10, 20, 30, 40, 50};
    
    // TODO: Create spans from array and vector
    
    // TODO: Use subspan
    
    // TODO: Pass spans to functions
    
    // Your code here
}

// ============================================================================
// TASK 10: Three-way comparison / spaceship operator (C++20)
// ============================================================================

class Point3D {
private:
    int x_, y_, z_;
    
public:
    Point3D(int x, int y, int z) : x_(x), y_(y), z_(z) {}
    
    // TODO: Implement spaceship operator
    auto operator<=>(const Point3D&) const = default;
    
    // This automatically generates <, <=, >, >=, ==, !=
    
    void print() const {
        std::cout << "(" << x_ << ", " << y_ << ", " << z_ << ")\n";
    }
};

class Version {
private:
    int major_, minor_, patch_;
    
public:
    Version(int major, int minor, int patch) 
        : major_(major), minor_(minor), patch_(patch) {}
    
    // TODO: Implement custom spaceship operator
    std::strong_ordering operator<=>(const Version& other) const {
        // Your code here
    }
    
    bool operator==(const Version& other) const = default;
    
    void print() const {
        std::cout << major_ << "." << minor_ << "." << patch_;
    }
};

void demonstrateSpaceship() {
    std::cout << "\n=== Task 10: Spaceship Operator ===\n";
    
    // TODO: Compare Point3D objects
    
    // TODO: Compare Version objects
    
    // TODO: Use in sorting
    
    // Your code here
}

// ============================================================================
// TASK 11: Designated initializers (C++20)
// ============================================================================

struct Config {
    std::string host = "localhost";
    int port = 8080;
    bool ssl = false;
    int timeout = 30;
};

void demonstrateDesignatedInitializers() {
    std::cout << "\n=== Task 11: Designated Initializers ===\n";
    
    // TODO: Use designated initializers
    // Config cfg = {.host = "example.com", .port = 443, .ssl = true};
    
    // TODO: Partial initialization (uses defaults for others)
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Combining multiple features
// ============================================================================

// A practical example combining optional, variant, and structured bindings
using JsonValue = std::variant<
    int,
    double,
    std::string,
    bool,
    std::nullptr_t
>;

class JsonObject {
private:
    std::map<std::string, JsonValue> data_;
    
public:
    // TODO: Implement methods to get values with optional return
    std::optional<int> getInt(const std::string& key) const {
        // Your code here
        return std::nullopt;
    }
    
    std::optional<std::string> getString(const std::string& key) const {
        // Your code here
        return std::nullopt;
    }
    
    void set(const std::string& key, JsonValue value) {
        data_[key] = value;
    }
    
    void print() const {
        for (const auto& [key, value] : data_) {
            std::cout << key << ": ";
            std::visit([](const auto& v) {
                std::cout << v;
            }, value);
            std::cout << "\n";
        }
    }
};

void demonstrateBonusTask() {
    std::cout << "\n=== Bonus: Combining Features ===\n";
    
    // TODO: Create JsonObject and use modern features
    
    // Your code here
}

// ============================================================================
// PRACTICAL TASK: Configuration system
// ============================================================================

class ConfigManager {
private:
    std::map<std::string, std::any> settings_;
    
public:
    // TODO: Implement methods using modern features
    template<typename T>
    void set(const std::string& key, T value) {
        // Your code here
    }
    
    template<typename T>
    std::optional<T> get(const std::string& key) const {
        // Your code here
        return std::nullopt;
    }
    
    template<typename T>
    T getOr(const std::string& key, T defaultValue) const {
        // Your code here
        return defaultValue;
    }
    
    void print() const {
        // Your code here
    }
};

void demonstratePracticalTask() {
    std::cout << "\n=== Practical Task: Configuration System ===\n";
    
    ConfigManager config;
    
    // TODO: Set various configuration values
    
    // TODO: Retrieve with optional
    
    // TODO: Retrieve with default value
    
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Miscellaneous Features Exercise\n";
    std::cout << "===========================================\n";
    
    demonstrateAutoAndDecltype();
    demonstrateRangeFor();
    demonstrateStructuredBindings();
    demonstrateOptional();
    demonstrateVariant();
    demonstrateAny();
    demonstrateInitializers();
    demonstrateConstexpr();
    demonstrateSpan();
    demonstrateSpaceship();
    demonstrateDesignatedInitializers();
    demonstrateBonusTask();
    demonstratePracticalTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. auto deduces type, decltype gets type of expression
 * 2. Structured bindings unpack tuples, pairs, and structs
 * 3. std::optional for values that may not exist
 * 4. std::variant for type-safe unions
 * 5. std::any for type-erased storage (use sparingly)
 * 6. constexpr functions can run at compile-time
 * 7. std::span provides non-owning view of contiguous data
 * 8. Spaceship operator generates all comparison operators
 * 9. Use modern features to write more expressive code
 * 10. Combine features for powerful abstractions
 */
