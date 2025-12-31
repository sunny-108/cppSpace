# MCQ Test - Item 36: Specify std::launch::async if Asynchronicity is Essential

## Instructions
- Each question has one correct answer unless otherwise specified
- Focus on launch policies and their implications
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
What are the three launch policies for `std::async`?

A) async, sync, default  
B) async, deferred, default  
C) parallel, serial, auto  
D) immediate, lazy, automatic  

**Answer:** B

**Explanation:** The three policies are `std::launch::async` (run immediately in new thread), `std::launch::deferred` (run lazily), and default (async | deferred).

---

## Question 2
```cpp
auto future = std::async(task);
```

When will `task` execute with this default policy?

A) Immediately in a new thread  
B) When you call future.get()  
C) Implementation decides - either immediately or on get()  
D) Never, it's an error  

**Answer:** C

**Explanation:** Default policy is `std::launch::async | std::launch::deferred`, giving the implementation freedom to choose.

---

## Question 3
```cpp
auto future = std::async(std::launch::async, task);
```

When does this start executing?

A) When you call get()  
B) Immediately in the current thread  
C) Immediately in a new thread  
D) After a short delay  

**Answer:** C

**Explanation:** `std::launch::async` forces immediate execution in a new thread.

---

## Question 4
```cpp
auto future = std::async(std::launch::deferred, task);
future.wait_for(std::chrono::seconds(1));
```

What happens here?

A) Waits up to 1 second for task to complete  
B) Returns immediately with deferred status  
C) Executes task then waits  
D) Compile error  

**Answer:** B

**Explanation:** Deferred tasks don't start until `get()` or `wait()` is called (not `wait_for`). `wait_for` returns immediately with `std::future_status::deferred`.

---

## Question 5
Why is using `wait_for()` with default policy problematic?

A) It's slower  
B) It might return deferred status immediately, never timing out  
C) It crashes the program  
D) It's not problematic  

**Answer:** B

**Explanation:** If the task is deferred, `wait_for` returns immediately with `deferred` status rather than actually waiting, breaking timeout logic.

---

## Question 6
```cpp
thread_local int tls = 0;

void task() {
    tls = 42;
    std::cout << tls << std::endl;
}

auto future = std::async(task);
```

With default policy, what's the issue?

A) No issue  
B) tls value is unpredictable - depends on which thread runs task  
C) Compile error  
D) tls is always 0  

**Answer:** B

**Explanation:** With default policy, task might run in the calling thread (deferred) or a new thread (async), making thread-local storage behavior unpredictable.

---

## Question 7
How do you force asynchronous execution?

A) std::async(std::launch::sync, task)  
B) std::async(std::launch::async, task)  
C) std::async(std::launch::parallel, task)  
D) std::async(task, true)  

**Answer:** B

**Explanation:** Explicitly specify `std::launch::async` to force execution in a new thread.

---

## Question 8
```cpp
auto future = std::async(std::launch::deferred, task);
// future destroyed without calling get()
```

What happens to task?

A) Executes in destructor  
B) Never executes  
C) Executes in background  
D) Undefined behavior  

**Answer:** B

**Explanation:** Deferred tasks only execute when `get()` or `wait()` is called. If neither is called, the task never runs.

---

## Question 9
When should you use `std::launch::deferred`?

A) When you need immediate execution  
B) When you want lazy evaluation and might not need the result  
C) For better performance always  
D) Never, it's deprecated  

**Answer:** B

**Explanation:** Deferred is useful for lazy evaluation - the task only runs if you actually request the result.

---

## Question 10
```cpp
auto f = std::async(task);
if (f.wait_for(0ms) == std::future_status::deferred) {
    // Handle deferred case
}
```

What is this code doing?

A) Starting the task  
B) Checking if task was deferred  
C) Timing the task  
D) Creating a race condition  

**Answer:** B

**Explanation:** Calling `wait_for(0ms)` returns immediately with the status - `deferred` if the task was deferred, allowing you to detect the policy used.

---

## Question 11
Which statement is TRUE about the default launch policy?

A) Always creates a new thread  
B) Always defers execution  
C) Gives implementation freedom to choose  
D) Is the same as std::launch::async  

**Answer:** C

**Explanation:** The default is `std::launch::async | std::launch::deferred`, allowing the implementation to choose based on system load and other factors.

---

## Question 12
```cpp
auto future = std::async(std::launch::async, []() {
    std::this_thread::sleep_for(5s);
});
```

When does the 5-second sleep start?

A) When you call get()  
B) Immediately  
C) After checking CPU availability  
D) When the future is destroyed  

**Answer:** B

**Explanation:** With `std::launch::async`, the task starts immediately in a new thread.

---

## Question 13
Why might you avoid the default launch policy in production code?

A) It's slower  
B) Unpredictable behavior with timeouts and thread-local storage  
C) It's deprecated  
D) It uses more memory  

**Answer:** B

**Explanation:** The unpredictability of the default policy can cause bugs with timeout-based logic and thread-local variables.

---

## Question 14
```cpp
template<typename F>
auto reallyAsync(F&& f) {
    return std::async(std::launch::async, std::forward<F>(f));
}
```

What is the purpose of this wrapper?

A) Better performance  
B) Force asynchronous execution  
C) Add error handling  
D) Enable caching  

**Answer:** B

**Explanation:** This wrapper ensures the task always runs asynchronously by explicitly specifying `std::launch::async`.

---

## Question 15
Which scenario REQUIRES `std::launch::async`?

A) When you want fast execution  
B) When using wait_for/wait_until for timeouts  
C) When the task is simple  
D) When you want to save memory  

**Answer:** B

**Explanation:** Timeout-based waits (`wait_for`/`wait_until`) require `std::launch::async` to work correctly, otherwise deferred tasks return immediately with deferred status.

---

## Question 16
```cpp
auto f1 = std::async(std::launch::deferred, task1);
auto f2 = std::async(std::launch::deferred, task2);
f1.get();
f2.get();
```

What thread(s) execute task1 and task2?

A) Two new threads  
B) One new thread  
C) The calling thread  
D) A thread pool  

**Answer:** C

**Explanation:** With `std::launch::deferred`, tasks execute in the thread that calls `get()` - in this case, the calling thread executes both sequentially.

---

## Question 17
What happens if you specify both policies?

```cpp
auto f = std::async(std::launch::async | std::launch::deferred, task);
```

A) Compile error  
B) Same as default policy  
C) Always async  
D) Always deferred  

**Answer:** B

**Explanation:** `std::launch::async | std::launch::deferred` is exactly the default policy.

---

## Question 18
When is it acceptable to use the default launch policy?

A) Never  
B) When you don't care about when/where the task runs  
C) Always  
D) Only in debug builds  

**Answer:** B

**Explanation:** If the timing and thread of execution don't matter for your use case, default policy is acceptable. But if you use timeouts or thread-locals, be explicit.

---

## Question 19
```cpp
auto f = std::async(std::launch::async, longTask);
// Do other work
f.get();  // Wait for result
```

This pattern is best for:

A) Tasks that must run in calling thread  
B) Tasks that should run concurrently while you do other work  
C) Tasks that might not be needed  
D) Tasks that are very quick  

**Answer:** B

**Explanation:** `std::launch::async` is perfect for running work concurrently while the calling thread continues with other tasks.

---

## Question 20
What's the risk of using default policy with thread-local storage?

A) Crashes  
B) Memory leaks  
C) Unpredictable values - depends on execution thread  
D) No risk  

**Answer:** C

**Explanation:** Thread-local variables depend on which thread they're accessed from. With default policy, you don't know which thread will run the task.

---

## Answer Key

1. B - Three policies
2. C - Implementation decides
3. C - Immediate in new thread
4. B - Returns deferred status
5. B - Timeout doesn't work
6. B - Thread unpredictable
7. B - launch::async
8. B - Never executes
9. B - Lazy evaluation
10. B - Checking deferred
11. C - Implementation freedom
12. B - Immediately
13. B - Unpredictable behavior
14. B - Force async
15. B - Timeouts require it
16. C - Calling thread
17. B - Same as default
18. B - Don't care when/where
19. B - Concurrent execution
20. C - Unpredictable values

---

## Scoring Guide

- 18-20: Expert level ⭐⭐⭐
- 15-17: Good understanding ⭐⭐
- 12-14: Basic understanding ⭐
- <12: Review Item 36 material

---

## Common Mistakes

1. **Relying on default policy** - Can cause subtle bugs
2. **Using wait_for with default policy** - Timeouts don't work with deferred
3. **Forgetting about thread-local storage** - Behavior changes with policy
4. **Not checking for deferred status** - Assuming task has started
