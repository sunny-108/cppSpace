# MCQ Test - Item 35: Prefer Task-Based Programming to Thread-Based

## Instructions
- Each question has one correct answer unless otherwise specified
- Consider code behavior, performance, and best practices
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
What is the PRIMARY advantage of using `std::async` over `std::thread`?

A) It's always faster  
B) It automatically manages return values  
C) It uses less memory  
D) It creates more threads  

**Answer:** B

**Explanation:** `std::async` returns a `std::future` that automatically handles return values, while `std::thread` requires manual mechanisms like shared variables to get results.

---

## Question 2
Which statement is TRUE about exception handling?

A) `std::thread` automatically propagates exceptions to the caller  
B) `std::async` stores exceptions in the future, `std::thread` does not  
C) Both handle exceptions the same way  
D) Neither can handle exceptions from the called function  

**Answer:** B

**Explanation:** With `std::async`, exceptions are captured and transported via the future. With `std::thread`, if an exception escapes the thread function, `std::terminate` is called.

---

## Question 3
What happens when you create too many threads manually?

A) They automatically queue  
B) The OS efficiently manages them  
C) You get oversubscription and poor performance  
D) The program waits for available threads  

**Answer:** C

**Explanation:** Creating too many threads leads to oversubscription - more threads than CPU cores, causing excessive context switching and degraded performance.

---

## Question 4
```cpp
auto result = std::async([]() { return 42; });
int value = result.get();
```

How many lines would the equivalent `std::thread` code require?

A) Same number (2 lines)  
B) About 5-7 lines  
C) About 10-15 lines  
D) Cannot be done with `std::thread`  

**Answer:** B

**Explanation:** You'd need to create a variable to store the result, create the thread passing the variable by reference, join the thread, then retrieve the value - significantly more complex.

---

## Question 5
When does the C++ runtime create a thread pool for `std::async`?

A) Always, automatically  
B) Never, it creates a new thread every time  
C) Implementation-defined, may use a thread pool  
D) Only if you specify a special flag  

**Answer:** C

**Explanation:** The C++ standard allows implementations to use thread pools for `std::async`, but it's not required. The runtime can make smart decisions about resource management.

---

## Question 6
```cpp
std::thread t(doWork);
// Some code that might throw
t.join();
```

What's the problem with this code?

A) Cannot join threads  
B) If exception thrown, thread might not be joined  
C) doWork might throw  
D) Nothing, it's correct  

**Answer:** B

**Explanation:** If an exception is thrown before `join()`, the thread won't be joined and the program will terminate in the thread's destructor.

---

## Question 7
How does `std::async` handle this situation automatically?

A) It doesn't, same problem exists  
B) The future's destructor handles cleanup  
C) It catches all exceptions  
D) It prevents exceptions  

**Answer:** B

**Explanation:** The future returned by `std::async` has a destructor that properly handles the underlying thread, ensuring cleanup even with exceptions.

---

## Question 8
```cpp
auto future = std::async([]() { throw std::runtime_error("error"); });
```

What happens when you call `future.get()`?

A) Program terminates  
B) Returns an error code  
C) The exception is re-thrown  
D) Returns a default value  

**Answer:** C

**Explanation:** Exceptions thrown in async tasks are stored in the future and re-thrown when you call `get()` or `wait()`.

---

## Question 9
Which scenario is a valid reason to use `std::thread` instead of `std::async`?

A) When you need to get a return value  
B) When you need to set thread priority or affinity  
C) When you want exception handling  
D) When you want automatic resource management  

**Answer:** B

**Explanation:** Low-level thread control (priority, affinity, native handles) requires `std::thread`. For most cases, `std::async` is preferred.

---

## Question 10
```cpp
std::vector<std::future<int>> futures;
for (int i = 0; i < 1000; ++i) {
    futures.push_back(std::async(compute, i));
}
```

What's the advantage of this approach over creating 1000 threads?

A) Uses less memory  
B) Runtime can limit thread creation  
C) Guaranteed to be faster  
D) All of the above  

**Answer:** B

**Explanation:** The runtime can use a thread pool and limit concurrent threads based on hardware capabilities, preventing oversubscription. Direct thread creation would create all 1000 threads.

---

## Question 11
What does this code return?

```cpp
int compute() { return 42; }
auto future = std::async(compute);
```

A) An int  
B) A std::future<int>  
C) A std::thread  
D) Compile error  

**Answer:** B

**Explanation:** `std::async` returns a `std::future<T>` where T is the return type of the callable.

---

## Question 12
How do you retrieve the value from a future?

A) future.value()  
B) future.get()  
C) future.retrieve()  
D) *future  

**Answer:** B

**Explanation:** The `get()` method retrieves the value (or rethrows an exception) and can only be called once per future.

---

## Question 13
```cpp
auto f1 = std::async(task1);
auto f2 = std::async(task2);
```

Are task1 and task2 guaranteed to run in parallel?

A) Yes, always  
B) No, depends on launch policy  
C) Yes, but only on multi-core systems  
D) No, they always run sequentially  

**Answer:** B

**Explanation:** The default launch policy allows the implementation to run tasks either asynchronously or deferred (lazily). See Item 36.

---

## Question 14
What's the PRIMARY reason task-based programming is recommended?

A) It's newer syntax  
B) Higher-level abstraction with automatic resource management  
C) Always uses fewer threads  
D) Required by C++17 standard  

**Answer:** B

**Explanation:** Task-based programming provides a higher-level abstraction that handles complexity like return values, exceptions, and resource management automatically.

---

## Question 15
```cpp
void threadBased() {
    std::vector<int> results(4);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&results, i]() {
            results[i] = compute(i);
        });
    }
    
    for (auto& t : threads) t.join();
}
```

How many lines would the equivalent `std::async` version be?

A) Same number  
B) About half  
C) About double  
D) Cannot be converted  

**Answer:** B

**Explanation:** With `std::async`, you don't need the separate results vector or manual joining - futures handle both automatically.

---

## Answer Key

1. B - Return value handling
2. B - Exception storage
3. C - Oversubscription
4. B - More code needed
5. C - Implementation-defined
6. B - Exception safety
7. B - Future destructor
8. C - Exception re-thrown
9. B - Low-level control
10. B - Thread pool benefits
11. B - Returns future
12. B - get() method
13. B - Launch policy dependent
14. B - High-level abstraction
15. B - More concise

---

## Scoring Guide

- 15/15: Expert level ⭐⭐⭐
- 12-14: Good understanding ⭐⭐
- 9-11: Basic understanding ⭐
- <9: Review Item 35 material

---

## Common Mistakes

1. **Thinking `std::async` is always async** - Default policy may defer execution
2. **Forgetting exception handling** - `std::thread` terminates, `std::async` stores
3. **Not understanding oversubscription** - Creating too many threads hurts performance
4. **Assuming thread-based is faster** - Task-based often performs better due to pooling
