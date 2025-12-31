/**
 * Modern C++ Exercise 05: STL Algorithms and Ranges
 * 
 * Topics covered:
 * - Classic STL algorithms (sort, find, transform, etc.)
 * - Algorithm composition
 * - C++17 parallel algorithms
 * - C++20 Ranges library
 * - Range adaptors and views
 * - Range algorithms
 * - Custom ranges
 * 
 * Compile: g++ -std=c++20 -Wall -Wextra -o stl_algo 05_stl_algorithms.cpp
 * For parallel: g++ -std=c++20 -Wall -Wextra -ltbb -o stl_algo 05_stl_algorithms.cpp
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <ranges>
#include <functional>
#include <iterator>
#include <execution>

// ============================================================================
// TASK 1: Classic STL Algorithms - Searching and Finding
// ============================================================================

void demonstrateSearchingAlgorithms() {
    std::cout << "\n=== Task 1: Searching Algorithms ===\n";
    
    std::vector<int> numbers = {1, 5, 3, 8, 2, 9, 4, 7, 6};
    
    // std::find
    auto it = std::find(numbers.begin(), numbers.end(), 8);
    if (it != numbers.end()) {
        std::cout << "Found 8 at index: " << std::distance(numbers.begin(), it) << "\n";
    }
    
    // TODO: Use std::find_if to find first even number
    
    // TODO: Use std::find_if_not to find first number that is not odd
    
    // TODO: Use std::count to count occurrences of a value
    
    // TODO: Use std::count_if to count even numbers
    
    // TODO: Use std::any_of to check if any number is greater than 10
    
    // TODO: Use std::all_of to check if all numbers are positive
    
    // TODO: Use std::none_of to check if no numbers are negative
    
    // Your code here
}

// ============================================================================
// TASK 2: Classic STL Algorithms - Sorting and Ordering
// ============================================================================

void demonstrateSortingAlgorithms() {
    std::cout << "\n=== Task 2: Sorting Algorithms ===\n";
    
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    // TODO: Sort in ascending order
    
    // TODO: Sort in descending order using greater<>
    
    // TODO: Sort with custom comparator (by absolute distance from 5)
    
    // TODO: Use std::partial_sort to get top 3 elements
    
    // TODO: Use std::nth_element to find median
    
    // TODO: Use std::partition to separate even and odd numbers
    
    // TODO: Use std::stable_partition to maintain relative order
    
    // Your code here
    
    // Working with strings
    std::vector<std::string> words = {"apple", "Banana", "cherry", "Date"};
    
    // TODO: Sort case-insensitively
    
    // TODO: Sort by string length
    
    // Your code here
}

// ============================================================================
// TASK 3: Classic STL Algorithms - Transforming and Modifying
// ============================================================================

void demonstrateTransformingAlgorithms() {
    std::cout << "\n=== Task 3: Transforming Algorithms ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::vector<int> squares(numbers.size());
    
    // TODO: Use std::transform to square each number
    
    // TODO: Use std::transform with two input ranges to add corresponding elements
    
    std::vector<int> result;
    // TODO: Use std::copy_if to copy only even numbers
    
    // TODO: Use std::remove_if to remove odd numbers (returns new end iterator)
    
    // TODO: Use std::replace_if to replace values
    
    // TODO: Use std::fill to set all elements to a value
    
    // TODO: Use std::generate to populate with computed values
    
    // Your code here
}

// ============================================================================
// TASK 4: Classic STL Algorithms - Accumulation and Reduction
// ============================================================================

void demonstrateAccumulationAlgorithms() {
    std::cout << "\n=== Task 4: Accumulation Algorithms ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // TODO: Use std::accumulate to sum all elements
    
    // TODO: Use std::accumulate with custom operation (product)
    
    // TODO: Use std::reduce (C++17) - similar to accumulate but parallelizable
    
    // TODO: Use std::transform_reduce to compute sum of squares
    
    std::vector<std::string> words = {"Hello", " ", "World", "!"};
    // TODO: Use std::accumulate to concatenate strings
    
    // Your code here
}

// ============================================================================
// TASK 5: Parallel Algorithms (C++17)
// ============================================================================

void demonstrateParallelAlgorithms() {
    std::cout << "\n=== Task 5: Parallel Algorithms ===\n";
    
    std::vector<int> numbers(1000000);
    std::generate(numbers.begin(), numbers.end(), std::rand);
    
    // TODO: Sort using sequential execution
    auto seq_copy = numbers;
    // Your code here: std::sort with std::execution::seq
    
    // TODO: Sort using parallel execution
    auto par_copy = numbers;
    // Your code here: std::sort with std::execution::par
    
    // TODO: Sort using parallel unsequenced execution
    auto par_unseq_copy = numbers;
    // Your code here: std::sort with std::execution::par_unseq
    
    // TODO: Use parallel std::transform
    
    // TODO: Use parallel std::reduce
    
    // Your code here
}

// ============================================================================
// TASK 6: C++20 Ranges - Basic Views
// ============================================================================

void demonstrateRangesBasics() {
    std::cout << "\n=== Task 6: C++20 Ranges Basics ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // TODO: Use std::ranges::sort (works on ranges, not just iterators)
    
    // TODO: Filter even numbers using views::filter
    // Your code here: auto even = numbers | std::views::filter(...);
    
    // TODO: Transform to squares using views::transform
    
    // TODO: Take first 5 elements using views::take
    
    // TODO: Drop first 3 elements using views::drop
    
    // TODO: Chain multiple views together
    // Example: filter even -> transform to square -> take first 3
    
    // Your code here
}

// ============================================================================
// TASK 7: C++20 Ranges - Advanced Views and Adaptors
// ============================================================================

void demonstrateAdvancedRanges() {
    std::cout << "\n=== Task 7: Advanced Ranges ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // TODO: Use views::reverse
    
    // TODO: Use views::take_while
    
    // TODO: Use views::drop_while
    
    // TODO: Use views::split to split a string
    std::string text = "one,two,three,four";
    // Your code here
    
    // TODO: Use views::join to flatten nested ranges
    std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}, {5, 6}};
    // Your code here
    
    // TODO: Use views::iota to generate sequence
    
    // TODO: Use views::zip (C++23) or implement manually
    
    // Your code here
}

// ============================================================================
// TASK 8: Range Algorithms
// ============================================================================

void demonstrateRangeAlgorithms() {
    std::cout << "\n=== Task 8: Range Algorithms ===\n";
    
    std::vector<int> numbers = {5, 2, 8, 1, 9};
    
    // TODO: Use std::ranges::sort
    
    // TODO: Use std::ranges::find
    
    // TODO: Use std::ranges::count_if
    
    // TODO: Use std::ranges::for_each
    
    // TODO: Use std::ranges::copy with projection
    
    // TODO: Use std::ranges::transform
    
    // Your code here
    
    // Range algorithms with structs
    struct Person {
        std::string name;
        int age;
    };
    
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35}
    };
    
    // TODO: Sort by age using projection
    
    // TODO: Find person with specific name using projection
    
    // Your code here
}

// ============================================================================
// TASK 9: Custom Ranges
// ============================================================================

// TODO: Implement a custom range for even numbers
class EvenNumbers {
private:
    int current_;
    int max_;
    
public:
    struct Iterator {
        int value;
        
        int operator*() const { return value; }
        Iterator& operator++() {
            value += 2;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return value < other.value;
        }
    };
    
    EvenNumbers(int max) : current_(0), max_(max) {}
    
    Iterator begin() const { return Iterator{current_}; }
    Iterator end() const { return Iterator{max_}; }
};

// TODO: Implement a Fibonacci sequence generator as a range
class FibonacciRange {
    // Your code here
};

void demonstrateCustomRanges() {
    std::cout << "\n=== Task 9: Custom Ranges ===\n";
    
    // TODO: Use your custom EvenNumbers range
    
    // TODO: Use your Fibonacci range
    
    // TODO: Combine custom range with range adaptors
    
    // Your code here
}

// ============================================================================
// BONUS TASK: Practical Applications
// ============================================================================

// TODO: Implement a function to process CSV data using ranges
void processCSVData() {
    std::cout << "\n=== Bonus: Process CSV Data ===\n";
    
    std::string csv = "John,30,Engineer\nJane,25,Designer\nBob,35,Manager";
    
    // TODO: Split by lines, then split each line by comma
    // Parse and process the data using ranges
    
    // Your code here
}

// TODO: Implement a function to find all palindromes in a list of words
void findPalindromes() {
    std::cout << "\n=== Bonus: Find Palindromes ===\n";
    
    std::vector<std::string> words = {
        "radar", "hello", "level", "world", "civic", "test"
    };
    
    // TODO: Use range algorithms to filter palindromes
    
    // Your code here
}

// TODO: Implement a data processing pipeline
void dataProcessingPipeline() {
    std::cout << "\n=== Bonus: Data Processing Pipeline ===\n";
    
    // Simulate sensor data
    std::vector<double> sensor_data = {
        23.5, 24.1, 23.8, 150.0, 24.2, 23.9, -10.0, 24.5
    };
    
    // TODO: Create a pipeline that:
    // 1. Filters out invalid readings (< 0 or > 100)
    // 2. Applies calibration (multiply by 1.05)
    // 3. Rounds to nearest integer
    // 4. Computes average
    
    // Your code here
}

void demonstrateBonusTask() {
    processCSVData();
    findPalindromes();
    dataProcessingPipeline();
}

// ============================================================================
// PRACTICAL TASK: Build a query system
// ============================================================================

struct Product {
    std::string name;
    std::string category;
    double price;
    int stock;
    
    void print() const {
        std::cout << name << " [" << category << "] $" << price 
                  << " (stock: " << stock << ")\n";
    }
};

class ProductDatabase {
private:
    std::vector<Product> products_;
    
public:
    ProductDatabase() {
        products_ = {
            {"Laptop", "Electronics", 999.99, 5},
            {"Mouse", "Electronics", 29.99, 50},
            {"Desk", "Furniture", 299.99, 10},
            {"Chair", "Furniture", 199.99, 15},
            {"Keyboard", "Electronics", 79.99, 30},
            {"Lamp", "Furniture", 49.99, 25}
        };
    }
    
    // TODO: Implement query methods using ranges
    
    // Get all products in a category
    auto getByCategory(const std::string& category) {
        // Your code here
    }
    
    // Get products within price range
    auto getByPriceRange(double min, double max) {
        // Your code here
    }
    
    // Get top N most expensive products
    auto getTopExpensive(size_t n) {
        // Your code here
    }
    
    // Get out of stock items (stock < 10)
    auto getLowStock() {
        // Your code here
    }
    
    // Calculate total inventory value
    double getTotalValue() {
        // Your code here
    }
};

void demonstratePracticalTask() {
    std::cout << "\n=== Practical Task: Product Query System ===\n";
    
    ProductDatabase db;
    
    // TODO: Test your query methods
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ STL Algorithms and Ranges Exercise\n";
    std::cout << "==============================================\n";
    
    demonstrateSearchingAlgorithms();
    demonstrateSortingAlgorithms();
    demonstrateTransformingAlgorithms();
    demonstrateAccumulationAlgorithms();
    // demonstrateParallelAlgorithms(); // Uncomment if TBB is available
    demonstrateRangesBasics();
    demonstrateAdvancedRanges();
    demonstrateRangeAlgorithms();
    demonstrateCustomRanges();
    demonstrateBonusTask();
    demonstratePracticalTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. STL algorithms work on iterator ranges
 * 2. Algorithms don't modify container size (use erase-remove idiom)
 * 3. C++17 parallel algorithms enable easy parallelization
 * 4. C++20 Ranges are lazy and composable
 * 5. Views don't own data, they provide a view into existing data
 * 6. Range adaptors can be chained with the pipe operator |
 * 7. Projections allow operating on specific members
 * 8. Ranges lead to more expressive and maintainable code
 */
