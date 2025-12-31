/**
 * Modern C++ Exercise 03: Lambda Expressions
 * 
 * Topics covered:
 * - Lambda syntax and basics
 * - Capture clauses ([=], [&], [this])
 * - Mutable lambdas
 * - Generic lambdas (C++14)
 * - Lambda with auto parameters
 * - std::function and lambda types
 * - Lambdas in algorithms
 * - IIFE (Immediately Invoked Function Expression)
 * 
 * Compile: g++ -std=c++17 -Wall -Wextra -o lambda 03_lambda_expressions.cpp
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <numeric>
#include <map>

// ============================================================================
// TASK 1: Basic lambda syntax
// ============================================================================

void demonstrateBasicLambda() {
    std::cout << "\n=== Task 1: Basic Lambda Syntax ===\n";
    
    // Simple lambda with no capture, no parameters
    auto greet = []() {
        std::cout << "Hello from lambda!\n";
    };
    greet();
    
    // Lambda with parameters
    auto add = [](int a, int b) {
        return a + b;
    };
    std::cout << "5 + 3 = " << add(5, 3) << "\n";
    
    // Lambda with explicit return type
    auto divide = [](double a, double b) -> double {
        return (b != 0) ? a / b : 0.0;
    };
    std::cout << "10 / 3 = " << divide(10.0, 3.0) << "\n";
    
    // TODO: Create your own lambdas:
    // 1. A lambda that takes a string and returns its length
    // 2. A lambda that checks if a number is even
    // 3. A lambda that computes factorial recursively (hint: use std::function)
    
    // Your code here
}

// ============================================================================
// TASK 2: Capture clauses
// ============================================================================

void demonstrateCaptures() {
    std::cout << "\n=== Task 2: Capture Clauses ===\n";
    
    int x = 10;
    int y = 20;
    
    // Capture by value
    auto captureByValue = [x, y]() {
        std::cout << "Captured by value: x=" << x << ", y=" << y << "\n";
        // x++; // ERROR: cannot modify captured by value
    };
    captureByValue();
    
    // Capture by reference
    auto captureByRef = [&x, &y]() {
        std::cout << "Captured by reference: x=" << x << ", y=" << y << "\n";
        x++;  // OK: can modify
        y++;
    };
    captureByRef();
    std::cout << "After reference capture: x=" << x << ", y=" << y << "\n";
    
    // Capture all by value
    auto captureAllValue = [=]() {
        std::cout << "Capture all by value: x=" << x << ", y=" << y << "\n";
    };
    captureAllValue();
    
    // Capture all by reference
    auto captureAllRef = [&]() {
        x += 10;
        y += 10;
        std::cout << "Modified via [&]: x=" << x << ", y=" << y << "\n";
    };
    captureAllRef();
    
    // Mixed capture
    int z = 30;
    auto mixedCapture = [=, &z]() {  // All by value except z by reference
        std::cout << "Mixed: x=" << x << ", y=" << y << ", z=" << z << "\n";
        z += 5;
    };
    mixedCapture();
    std::cout << "After mixed capture: z=" << z << "\n";
    
    // TODO: Experiment with captures:
    // 1. Create a lambda that captures only specific variables
    // 2. Try to modify a value-captured variable (won't compile)
    // 3. Use [=, &var] and [&, var] patterns
    // 4. Understand the difference between capture and parameter
    
    // Your code here
}

// ============================================================================
// TASK 3: Mutable lambdas
// ============================================================================

void demonstrateMutableLambda() {
    std::cout << "\n=== Task 3: Mutable Lambdas ===\n";
    
    int counter = 0;
    
    // Non-mutable lambda with value capture
    auto increment1 = [counter]() {
        // counter++; // ERROR: cannot modify value-captured variable
        return counter + 1;
    };
    
    // Mutable lambda - can modify value-captured variables
    auto increment2 = [counter]() mutable {
        counter++;  // OK with mutable
        return counter;
    };
    
    std::cout << "Original counter: " << counter << "\n";
    std::cout << "increment1(): " << increment1() << "\n";
    std::cout << "increment2(): " << increment2() << "\n";
    std::cout << "increment2(): " << increment2() << "\n";
    std::cout << "Original counter still: " << counter << "\n";
    
    // TODO: Create a mutable lambda that:
    // 1. Captures a counter by value
    // 2. Increments it each time it's called
    // 3. Returns the current count
    // 4. Call it multiple times and observe the behavior
    
    // Your code here
}

// ============================================================================
// TASK 4: Generic lambdas (C++14)
// ============================================================================

void demonstrateGenericLambda() {
    std::cout << "\n=== Task 4: Generic Lambdas ===\n";
    
    // Generic lambda with auto parameters
    auto print = [](const auto& value) {
        std::cout << value << "\n";
    };
    
    print(42);
    print(3.14);
    print("Hello");
    print(std::string("World"));
    
    // Generic lambda with multiple auto parameters
    auto add = [](auto a, auto b) {
        return a + b;
    };
    
    std::cout << "5 + 3 = " << add(5, 3) << "\n";
    std::cout << "2.5 + 1.5 = " << add(2.5, 1.5) << "\n";
    std::cout << "str1 + str2 = " << add(std::string("Hello"), std::string(" World")) << "\n";
    
    // TODO: Create generic lambdas for:
    // 1. A comparator that works with any type
    // 2. A function that prints container elements (works with vector, list, etc.)
    // 3. A function that multiplies two values of any compatible types
    
    // Your code here
}

// ============================================================================
// TASK 5: Lambdas with STL algorithms
// ============================================================================

void demonstrateLambdaWithAlgorithms() {
    std::cout << "\n=== Task 5: Lambdas with STL Algorithms ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // std::for_each
    std::cout << "Numbers: ";
    std::for_each(numbers.begin(), numbers.end(), [](int n) {
        std::cout << n << " ";
    });
    std::cout << "\n";
    
    // std::count_if
    int evenCount = std::count_if(numbers.begin(), numbers.end(), [](int n) {
        return n % 2 == 0;
    });
    std::cout << "Even numbers: " << evenCount << "\n";
    
    // std::find_if
    auto it = std::find_if(numbers.begin(), numbers.end(), [](int n) {
        return n > 5;
    });
    if (it != numbers.end()) {
        std::cout << "First number > 5: " << *it << "\n";
    }
    
    // std::transform
    std::vector<int> squared(numbers.size());
    std::transform(numbers.begin(), numbers.end(), squared.begin(), [](int n) {
        return n * n;
    });
    std::cout << "Squared: ";
    for (int n : squared) std::cout << n << " ";
    std::cout << "\n";
    
    // TODO: Use lambdas with:
    // 1. std::sort to sort in descending order
    // 2. std::remove_if to remove odd numbers
    // 3. std::accumulate with a custom operation
    // 4. std::partition to separate even and odd numbers
    
    // Your code here
}

// ============================================================================
// TASK 6: std::function and storing lambdas
// ============================================================================

void demonstrateStdFunction() {
    std::cout << "\n=== Task 6: std::function and Lambda Storage ===\n";
    
    // std::function can store lambdas
    std::function<int(int, int)> operation;
    
    operation = [](int a, int b) { return a + b; };
    std::cout << "5 + 3 = " << operation(5, 3) << "\n";
    
    operation = [](int a, int b) { return a * b; };
    std::cout << "5 * 3 = " << operation(5, 3) << "\n";
    
    // Vector of functions
    std::vector<std::function<void()>> tasks;
    
    for (int i = 0; i < 5; ++i) {
        tasks.push_back([i]() {
            std::cout << "Task " << i << " executed\n";
        });
    }
    
    std::cout << "Executing tasks:\n";
    for (auto& task : tasks) {
        task();
    }
    
    // TODO: Create a calculator using std::function:
    // 1. Map of operations ("+", "-", "*", "/")
    // 2. Each operation is a lambda stored in std::function
    // 3. Allow user to select operation and compute result
    
    // Your code here
}

// ============================================================================
// TASK 7: Lambda as callback
// ============================================================================

class Button {
private:
    std::string label_;
    std::function<void()> onClick_;
    
public:
    Button(const std::string& label) : label_(label) {}
    
    void setOnClick(std::function<void()> callback) {
        onClick_ = callback;
    }
    
    void click() {
        std::cout << "Button '" << label_ << "' clicked!\n";
        if (onClick_) {
            onClick_();
        }
    }
};

void demonstrateLambdaCallback() {
    std::cout << "\n=== Task 7: Lambda as Callback ===\n";
    
    Button okButton("OK");
    Button cancelButton("Cancel");
    
    int clickCount = 0;
    
    okButton.setOnClick([&clickCount]() {
        clickCount++;
        std::cout << "OK handler executed (click #" << clickCount << ")\n";
    });
    
    cancelButton.setOnClick([]() {
        std::cout << "Cancel handler executed\n";
    });
    
    okButton.click();
    okButton.click();
    cancelButton.click();
    
    // TODO: Enhance this example:
    // 1. Add more button events (onHover, onDoubleClick)
    // 2. Create a button that maintains its own click count
    // 3. Implement event chaining (multiple callbacks per event)
    
    // Your code here
}

// ============================================================================
// TASK 8: IIFE - Immediately Invoked Function Expression
// ============================================================================

void demonstrateIIFE() {
    std::cout << "\n=== Task 8: IIFE ===\n";
    
    // Use IIFE to initialize a const variable with complex logic
    const int result = []() {
        int sum = 0;
        for (int i = 1; i <= 10; ++i) {
            sum += i;
        }
        return sum;
    }();
    
    std::cout << "Sum of 1-10: " << result << "\n";
    
    // IIFE for conditional initialization
    const std::string message = [](bool condition) {
        if (condition) {
            return std::string("Condition is true");
        } else {
            return std::string("Condition is false");
        }
    }(true);
    
    std::cout << message << "\n";
    
    // TODO: Use IIFE for:
    // 1. Complex const variable initialization
    // 2. Platform-specific code selection
    // 3. Initialization of a const vector with filtered values
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Advanced lambda techniques
// ============================================================================

void demonstrateBonusTask() {
    std::cout << "\n=== Bonus Task: Advanced Techniques ===\n";
    
    // 1. Recursive lambda using std::function
    std::function<int(int)> fibonacci = [&fibonacci](int n) -> int {
        if (n <= 1) return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
    };
    
    std::cout << "Fibonacci(10) = " << fibonacci(10) << "\n";
    
    // TODO: Implement:
    // 1. A lambda that returns another lambda (lambda factory)
    // 2. Recursive lambda for tree traversal
    // 3. Lambda with variadic parameters (C++14)
    // 4. Use lambda to implement Y-combinator
    
    // Example: Lambda factory
    auto makeMultiplier = [](int factor) {
        return [factor](int value) {
            return value * factor;
        };
    };
    
    auto times2 = makeMultiplier(2);
    auto times10 = makeMultiplier(10);
    
    std::cout << "5 * 2 = " << times2(5) << "\n";
    std::cout << "5 * 10 = " << times10(5) << "\n";
    
    // Your code here for additional implementations
}

// ============================================================================
// PRACTICAL TASK: Build a simple event system
// ============================================================================

class EventSystem {
private:
    std::map<std::string, std::vector<std::function<void()>>> listeners_;
    
public:
    // TODO: Implement these methods
    void addEventListener(const std::string& event, std::function<void()> callback) {
        // Your code here
    }
    
    void triggerEvent(const std::string& event) {
        // Your code here
    }
    
    void removeAllListeners(const std::string& event) {
        // Your code here
    }
};

void demonstratePracticalTask() {
    std::cout << "\n=== Practical Task: Event System ===\n";
    
    // TODO:
    // 1. Complete the EventSystem class
    // 2. Create an event system
    // 3. Add multiple listeners to events
    // 4. Trigger events
    // 5. Test with different lambda captures
    
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Lambda Expressions Exercise\n";
    std::cout << "=======================================\n";
    
    demonstrateBasicLambda();
    demonstrateCaptures();
    demonstrateMutableLambda();
    demonstrateGenericLambda();
    demonstrateLambdaWithAlgorithms();
    demonstrateStdFunction();
    demonstrateLambdaCallback();
    demonstrateIIFE();
    demonstrateBonusTask();
    demonstratePracticalTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. Lambda syntax: [capture](parameters) -> return_type { body }
 * 2. Capture by value [=] copies, capture by reference [&] references
 * 3. Use mutable keyword to modify value-captured variables
 * 4. Generic lambdas (auto parameters) work with any type
 * 5. std::function can store lambdas with matching signature
 * 6. Lambdas are great for algorithms, callbacks, and IIFE
 * 7. Capture [this] to access member variables in methods
 * 8. Each lambda has a unique type (even if identical code)
 */
