# Assignment 01: Thread Basics

## Overview
Introduction to threading fundamentals in C++. Understand what threads are, why they're used, and how they differ from processes. Learn to create basic threads and handle their lifecycle.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 4-5 hours  
**Prerequisites:** Solid understanding of C++ fundamentals, basic familiarity with compilation

---

## Learning Objectives
- Understand the concept of threads and concurrency
- Differentiate between processes and threads
- Create and manage basic threads in C++
- Understand thread execution models
- Recognize common threading pitfalls

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. What is a thread?
A) A separate process with its own memory space  
B) A lightweight unit of execution within a process sharing the process's memory  
C) A function that runs sequentially  
D) A data structure for storing values  

**Answer:** B

---

### Q2. What is the main advantage of using threads over processes?
A) Threads are always faster  
B) Threads have lower overhead and can share memory easily  
C) Threads never have race conditions  
D) Threads don't need synchronization  

**Answer:** B

---

### Q3. In C++, which header provides thread support?
A) `<pthread.h>`  
B) `<thread>`  
C) `<concurrency>`  
D) `<parallel>`  

**Answer:** B

---

### Q4. What happens if you don't join or detach a thread before the thread object is destroyed?
A) The thread continues running  
B) The thread is automatically joined  
C) `std::terminate()` is called, crashing the program  
D) Nothing, it's fine  

**Answer:** C

---

### Q5. Which of the following is true about threads?
A) Threads in the same process share the heap but have separate stacks  
B) Threads have completely separate memory spaces  
C) Threads share everything including the stack  
D) Threads cannot access global variables  

**Answer:** A

---

### Q6. What is the purpose of `std::thread::hardware_concurrency()`?
A) To start multiple threads  
B) To return a hint about the number of concurrent threads supported by hardware  
C) To join all threads  
D) To measure thread performance  

**Answer:** B

---

### Q7. Can you create a thread without passing a function to execute?
A) Yes, threads can be empty  
B) No, a thread must have a callable to execute  
C) Only in C++20  
D) Yes, but only if you use detach()  

**Answer:** B

---

### Q8. What is "context switching"?
A) Changing between different programs  
B) The overhead of saving and restoring thread/process state  
C) Switching between user and kernel mode  
D) Changing thread priorities  

**Answer:** B

---

### Q9. Which statement about threads is FALSE?
A) Threads can share global variables  
B) Each thread has its own stack  
C) Creating threads is free and has no overhead  
D) Threads within a process share file descriptors  

**Answer:** C

---

### Q10. What is a "thread-safe" function?
A) A function that never crashes  
B) A function that can be called by multiple threads simultaneously without causing issues  
C) A function that only runs on one thread  
D) A function that automatically uses locks  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Find the Bug

Review this code and identify the issues:

```cpp
#include <iostream>
#include <thread>

void printMessage() {
    std::cout << "Hello from thread!\n";
}

int main() {
    std::thread t(printMessage);
    std::cout << "Hello from main!\n";
    return 0;  // Bug: thread not joined or detached!
}
```

**Questions:**
1. What will happen when this program runs?
2. Why is this dangerous?
3. How would you fix it?
4. What are the two ways to fix this problem?

**Solution:**
```cpp
int main() {
    std::thread t(printMessage);
    std::cout << "Hello from main!\n";
    t.join();  // Wait for thread to complete
    return 0;
}
```

---

### Exercise 2.2: Race Condition in Output

```cpp
#include <iostream>
#include <thread>

void printNumbers(int id) {
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread " << id << ": " << i << std::endl;
    }
}

int main() {
    std::thread t1(printNumbers, 1);
    std::thread t2(printNumbers, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Questions:**
1. Run this program multiple times. Is the output always the same?
2. Why is the output interleaved?
3. Is this a data race or just non-deterministic output?
4. How would you make the output more predictable?

---

### Exercise 2.3: Thread with References

```cpp
#include <iostream>
#include <thread>

void modifyValue(int& value) {
    value = 100;
}

int main() {
    int x = 10;
    std::thread t(modifyValue, x);  // Bug: passing by value, not reference!
    t.join();
    
    std::cout << "x = " << x << std::endl;  // Still 10, not 100!
    return 0;
}
```

**Questions:**
1. What will be the value of `x` after the thread completes?
2. Why doesn't the modification work?
3. How do you correctly pass by reference to a thread?
4. Fix the code

**Solution:**
```cpp
std::thread t(modifyValue, std::ref(x));  // Use std::ref
```

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Basic Thread Creation

Implement a program that:
1. Creates a thread that counts from 1 to 10
2. The main thread counts from 11 to 20
3. Both threads print their counts
4. Properly join the thread

**Template:**
```cpp
#include <iostream>
#include <thread>

void countFrom1to10() {
    // Your implementation
}

int main() {
    // Your implementation
    return 0;
}
```

---

### Exercise 3.2: Multiple Threads

Create a program that:
1. Launches 5 threads
2. Each thread prints its thread ID and a message
3. Use a vector to store thread objects
4. Join all threads before exiting

**Requirements:**
- Use `std::thread::get_id()` to print thread ID
- Store threads in `std::vector<std::thread>`
- Properly join all threads

**Template:**
```cpp
#include <iostream>
#include <thread>
#include <vector>

void workerFunction(int workerId) {
    // Your implementation
}

int main() {
    const int numThreads = 5;
    std::vector<std::thread> threads;
    
    // Create threads
    
    // Join threads
    
    return 0;
}
```

---

### Exercise 3.3: Thread with Different Callables

Demonstrate creating threads with:
1. A regular function
2. A lambda function
3. A function object (functor)
4. A member function of a class

**Template:**
```cpp
#include <iostream>
#include <thread>

// 1. Regular function
void regularFunction() {
    std::cout << "Regular function\n";
}

// 2. Functor
class Functor {
public:
    void operator()() const {
        std::cout << "Functor\n";
    }
};

// 3. Class with member function
class MyClass {
public:
    void memberFunction() {
        std::cout << "Member function\n";
    }
};

int main() {
    // Create thread with regular function
    std::thread t1(regularFunction);
    
    // Create thread with lambda
    // Your implementation
    
    // Create thread with functor
    // Your implementation
    
    // Create thread with member function
    // Your implementation
    
    // Join all threads
    
    return 0;
}
```

---

### Exercise 3.4: Passing Multiple Arguments

Create a function that takes multiple arguments and run it in a thread:

```cpp
#include <iostream>
#include <thread>
#include <string>

void printPersonInfo(const std::string& name, int age, double height) {
    std::cout << "Name: " << name 
              << ", Age: " << age 
              << ", Height: " << height << "m\n";
}

int main() {
    // Create a thread that calls printPersonInfo with arguments
    // Your implementation
    
    return 0;
}
```

**Extension:** Modify the function to take a struct by reference and update its fields.

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Thread Not Joined

This program has a subtle bug:

```cpp
#include <iostream>
#include <thread>
#include <vector>

void process(int id) {
    std::cout << "Processing " << id << "\n";
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread(process, i));
    }
    
    // Oops, forgot to join!
    
    return 0;
}
```

**Tasks:**
1. Run this program. What happens?
2. Add code to join all threads
3. What happens if an exception is thrown before joining?
4. Implement RAII to ensure threads are always joined

---

### Exercise 4.2: Dangling Reference

```cpp
#include <iostream>
#include <thread>

void printValue(const int& value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Value: " << value << "\n";
}

void dangerousFunction() {
    int localValue = 42;
    std::thread t(printValue, std::ref(localValue));
    t.detach();  // Dangerous!
    // localValue goes out of scope here!
}

int main() {
    dangerousFunction();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return 0;
}
```

**Questions:**
1. What is the problem with this code?
2. What is "undefined behavior" and why does it occur here?
3. How would you fix this?
4. When is it safe to detach a thread?

---

## Part 5: Performance & Analysis

### Exercise 5.1: Thread Creation Overhead

Measure the overhead of creating threads:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void emptyFunction() {
    // Does nothing
}

void measureThreadCreation(int numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(emptyFunction);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Creating " << numThreads << " threads took " 
              << duration.count() << " microseconds\n";
    std::cout << "Average per thread: " 
              << duration.count() / numThreads << " microseconds\n";
}

int main() {
    std::cout << "Hardware concurrency: " 
              << std::thread::hardware_concurrency() << "\n\n";
    
    measureThreadCreation(1);
    measureThreadCreation(10);
    measureThreadCreation(100);
    measureThreadCreation(1000);
    
    return 0;
}
```

**Analysis Tasks:**
1. Run this program and record the results
2. What do you observe about thread creation time?
3. Is there a linear relationship?
4. What conclusions can you draw about creating many threads?

---

### Exercise 5.2: Sequential vs Concurrent

Compare sequential and concurrent execution:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>

void computeSum(const std::vector<int>& data, size_t start, size_t end, long long& result) {
    result = 0;
    for (size_t i = start; i < end; ++i) {
        result += data[i];
    }
}

int main() {
    const size_t dataSize = 10000000;
    std::vector<int> data(dataSize, 1);
    
    // Sequential
    auto start = std::chrono::high_resolution_clock::now();
    long long seqResult = 0;
    computeSum(data, 0, dataSize, seqResult);
    auto end = std::chrono::high_resolution_clock::now();
    auto seqDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Concurrent (2 threads)
    start = std::chrono::high_resolution_clock::now();
    long long result1 = 0, result2 = 0;
    std::thread t1(computeSum, std::ref(data), 0, dataSize/2, std::ref(result1));
    std::thread t2(computeSum, std::ref(data), dataSize/2, dataSize, std::ref(result2));
    t1.join();
    t2.join();
    long long concResult = result1 + result2;
    end = std::chrono::high_resolution_clock::now();
    auto concDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential: " << seqDuration.count() << "ms, Result: " << seqResult << "\n";
    std::cout << "Concurrent: " << concDuration.count() << "ms, Result: " << concResult << "\n";
    std::cout << "Speedup: " << static_cast<double>(seqDuration.count()) / concDuration.count() << "x\n";
    
    return 0;
}
```

**Tasks:**
1. Run and record results
2. Try with 4 threads, 8 threads
3. What is the optimal number of threads?
4. When does adding more threads stop helping?

---

## Submission Guidelines

### Required Files:
1. **Source Code** - All implementations (`.cpp` files)
2. **MCQ Answers** - Document with explanations
3. **Analysis Report** - Performance measurements and conclusions
4. **Compilation Instructions** - How to build your code

### Compilation:
```bash
g++ -std=c++17 -pthread -o program program.cpp
```

### Testing:
- Verify all programs compile without warnings
- Test on multiple runs to observe non-deterministic behavior
- Document any interesting observations

---

## Evaluation Criteria

- **Understanding (30%):** MCQ answers show comprehension
- **Correctness (30%):** Code compiles and runs correctly
- **Completeness (20%):** All exercises attempted
- **Analysis (20%):** Thoughtful performance analysis

---

## Key Takeaways

After completing this assignment, you should understand:
- ✅ What threads are and how they differ from processes
- ✅ How to create and manage basic threads in C++
- ✅ The importance of joining or detaching threads
- ✅ How to pass arguments to threads
- ✅ Basic thread overhead and when to use threads

---

## Next Steps

After mastering thread basics, proceed to:
- **Assignment 02:** Advanced std::thread usage
- **Assignment 03:** Thread lifecycle management

---

## Additional Resources

- [cppreference - std::thread](https://en.cppreference.com/w/cpp/thread/thread)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 2
- [ISO C++ Threading Tutorial](https://isocpp.org/wiki/faq/cpp11-library-concurrency)

Good luck!