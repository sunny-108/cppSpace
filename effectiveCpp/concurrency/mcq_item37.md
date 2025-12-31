# MCQ Test - Item 37: Make std::threads Unjoinable on All Paths

## Instructions
- Each question has one correct answer unless otherwise specified
- Focus on thread lifecycle and RAII principles
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
What happens if a `std::thread` destructor is called on a joinable thread?

A) The thread is automatically joined  
B) The thread is automatically detached  
C) `std::terminate` is called  
D) Nothing, it's safe  

**Answer:** C

**Explanation:** If a `std::thread` object is destroyed while still joinable (neither joined nor detached), `std::terminate` is called, terminating the program.

---

## Question 2
```cpp
void func() {
    std::thread t(doWork);
    if (error) return;  // Bug!
    t.join();
}
```

What's wrong with this code?

A) Cannot return from function with active thread  
B) Thread not joined if early return happens  
C) doWork might throw  
D) Nothing, it's correct  

**Answer:** B

**Explanation:** If the early return is taken, `t.join()` is never called, leaving the thread joinable when its destructor runs, causing `std::terminate`.

---

## Question 3
What does it mean for a thread to be "joinable"?

A) It's running  
B) It represents an active thread of execution  
C) It can be joined  
D) It has completed  

**Answer:** B

**Explanation:** A thread is joinable if it represents an actual thread of execution. Default-constructed and moved-from threads are not joinable.

---

## Question 4
```cpp
std::thread t(doWork);
t.join();
t.join();  // What happens?
```

A) Works fine, waits again  
B) Undefined behavior / crash  
C) Does nothing  
D) Compile error  

**Answer:** B

**Explanation:** After `join()`, the thread is no longer joinable. Calling `join()` again on a non-joinable thread results in undefined behavior.

---

## Question 5
What is the RAII solution for thread management?

A) Use try-catch blocks  
B) Create a wrapper class that joins/detaches in destructor  
C) Always use detach()  
D) Call join() in a finally block  

**Answer:** B

**Explanation:** RAII (Resource Acquisition Is Initialization) means creating a wrapper class that ensures the thread is joined or detached in its destructor, guaranteeing cleanup.

---

## Question 6
```cpp
class ThreadGuard {
    std::thread t;
public:
    explicit ThreadGuard(std::thread t_) : t(std::move(t_)) {}
    ~ThreadGuard() { if (t.joinable()) t.join(); }
    // ...
};
```

Why check `joinable()` before calling `join()`?

A) For performance  
B) Because thread might already be joined or moved-from  
C) To avoid blocking  
D) It's not necessary  

**Answer:** B

**Explanation:** The thread might have been joined, detached, or moved-from, making it non-joinable. Calling `join()` on a non-joinable thread is undefined behavior.

---

## Question 7
What happens when you move a `std::thread`?

A) Creates a copy  
B) Source becomes non-joinable, target becomes joinable  
C) Both become joinable  
D) Compile error - threads can't be moved  

**Answer:** B

**Explanation:** Moving a thread transfers ownership. The source thread becomes non-joinable (no longer represents a thread), and the target takes over.

---

## Question 8
```cpp
{
    std::thread t(doWork);
    t.detach();
}  // Destructor called here
```

Is this safe?

A) Yes, detached threads don't cause terminate  
B) No, still causes terminate  
C) Only safe if doWork completes quickly  
D) Depends on the platform  

**Answer:** A

**Explanation:** After `detach()`, the thread is no longer joinable, so the destructor doesn't call `std::terminate`. However, you must ensure the thread doesn't access local variables.

---

## Question 9
What's the danger of using `detach()`?

A) Causes memory leaks  
B) Thread might outlive the data it accesses  
C) Slower performance  
D) Not portable  

**Answer:** B

**Explanation:** Detached threads run independently. If they access local variables or data from their parent scope, that data might be destroyed while the thread is still using it.

---

## Question 10
```cpp
void func() {
    int localData = 42;
    std::thread t([&localData]() {
        std::cout << localData;  // Danger!
    });
    t.detach();
}  // localData destroyed, but thread might still run
```

How to fix this?

A) Use join() instead of detach()  
B) Make localData static  
C) Pass by value, not reference  
D) All of the above could work  

**Answer:** D

**Explanation:** Using `join()` ensures the thread completes before `localData` is destroyed. Making it `static` makes it outlive the function. Passing by value copies the data to the thread.

---

## Question 11
Which is better for exception safety?

A) Manual join() calls  
B) RAII wrapper that joins in destructor  
C) Always use detach()  
D) They're equivalent  

**Answer:** B

**Explanation:** RAII wrapper ensures the thread is properly cleaned up even if an exception is thrown, avoiding manual cleanup code on every exit path.

---

## Question 12
```cpp
class ThreadGuard {
    std::thread t;
public:
    ~ThreadGuard() { t.join(); }  // Is this safe?
};
```

What's the problem?

A) Nothing, it's perfect  
B) Should check joinable() first  
C) Should use detach()  
D) Should use try-catch  

**Answer:** B

**Explanation:** If the thread has been moved from or is already non-joinable, calling `join()` causes undefined behavior. Always check `joinable()` first.

---

## Question 13
What does C++20's `std::jthread` provide?

A) Faster threads  
B) Automatic joining in destructor  
C) Better performance  
D) GPU acceleration  

**Answer:** B

**Explanation:** `std::jthread` (joining thread) automatically joins in its destructor, providing built-in RAII for thread management.

---

## Question 14
```cpp
std::thread t;
t.join();  // What happens?
```

A) Waits forever  
B) Returns immediately  
C) Undefined behavior  
D) Compile error  

**Answer:** C

**Explanation:** Default-constructed threads are not joinable. Calling `join()` on a non-joinable thread is undefined behavior.

---

## Question 15
Why should ThreadGuard disable copy operations?

A) For performance  
B) Because std::thread is not copyable  
C) To save memory  
D) It's just a convention  

**Answer:** B

**Explanation:** `std::thread` is move-only (not copyable), so any wrapper should also be move-only to maintain the same semantics.

---

## Question 16
```cpp
{
    ThreadGuard guard(std::thread(task1));
    ThreadGuard guard2(std::thread(task2));
    throw std::runtime_error("Error!");
}
```

What happens to the threads?

A) They leak  
B) Program terminates  
C) Both are properly joined during stack unwinding  
D) Undefined behavior  

**Answer:** C

**Explanation:** RAII ensures that destructors are called during stack unwinding, so both threads are joined even when an exception is thrown.

---

## Question 17
What is the correct order of cleanup for ThreadGuard?

A) Delete copy constructor, delete copy assignment  
B) Implement move constructor, implement move assignment  
C) Both A and B  
D) Neither is necessary  

**Answer:** C

**Explanation:** ThreadGuard should delete copy operations (because `std::thread` can't be copied) and implement move operations (because `std::thread` can be moved).

---

## Question 18
```cpp
void runTasks() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(task, i);
    }
    // Missing join() calls - what happens?
}
```

A) Threads automatically join  
B) Program terminates  
C) Threads leak but program continues  
D) Undefined behavior  

**Answer:** B

**Explanation:** When the vector is destroyed, each joinable thread's destructor calls `std::terminate`, terminating the program.

---

## Question 19
How should the above code be fixed?

A) Call detach() on each thread  
B) Call join() on each thread before function ends  
C) Use ThreadGuard or similar RAII wrapper  
D) Both B and C work  

**Answer:** D

**Explanation:** Either manually join all threads before they're destroyed, or use an RAII wrapper that automatically handles it.

---

## Question 20
```cpp
class ThreadPool {
    std::vector<std::thread> workers;
public:
    ~ThreadPool() {
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
    }
};
```

Is this destructor exception-safe?

A) Yes, perfect  
B) No, should be noexcept and handle join() exceptions  
C) No, should use detach()  
D) Destructors don't need exception safety  

**Answer:** B

**Explanation:** Destructors should generally be `noexcept`. If `join()` throws, it should be caught to prevent exception escaping the destructor (which would call `std::terminate`).

---

## Answer Key

1. C - std::terminate called
2. B - Thread not joined
3. B - Represents active thread
4. B - Undefined behavior
5. B - RAII wrapper
6. B - Might not be joinable
7. B - Source non-joinable
8. A - Detached safe
9. B - Data lifetime issue
10. D - Multiple solutions
11. B - RAII for exceptions
12. B - Check joinable
13. B - Auto joining
14. C - Undefined behavior
15. B - thread not copyable
16. C - Properly joined
17. C - Both needed
18. B - Terminates
19. D - Both work
20. B - Exception safety

---

## Scoring Guide

- 18-20: Expert level ⭐⭐⭐
- 15-17: Good understanding ⭐⭐
- 12-14: Basic understanding ⭐
- <12: Review Item 37 material

---

## Common Mistakes

1. **Forgetting to join or detach** - Causes termination
2. **Not using RAII** - Manual management error-prone
3. **Calling join() on non-joinable thread** - Undefined behavior
4. **Detaching threads that access local data** - Dangling references
5. **Not handling exceptions in thread management** - Resource leaks
