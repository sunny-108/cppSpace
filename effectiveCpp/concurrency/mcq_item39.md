# MCQ Test - Item 39: Consider void Futures for One-Shot Event Communication

## Instructions
- Each question has one correct answer unless otherwise specified
- Focus on void futures vs condition variables
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
What is a "void future" used for?

A) Futures that return nothing  
B) Invalid futures  
C) One-shot event signaling between threads  
D) Futures that have been moved  

**Answer:** C

**Explanation:** `std::future<void>` and `std::promise<void>` are used for one-shot signaling - notifying that an event has occurred without passing data.

---

## Question 2
```cpp
std::promise<void> p;
auto f = p.get_future();

// Thread 1
f.wait();

// Thread 2
p.set_value();
```

What does this accomplish?

A) Passes void data  
B) Thread 1 waits until Thread 2 signals  
C) Nothing useful  
D) Compile error  

**Answer:** B

**Explanation:** Thread 1 blocks on `wait()` until Thread 2 calls `set_value()`, providing simple one-shot synchronization.

---

## Question 3
How many times can you call `set_value()` on a `std::promise<void>`?

A) Once  
B) Unlimited  
C) Twice  
D) None - it's automatic  

**Answer:** A

**Explanation:** A promise can only be satisfied once. Calling `set_value()` a second time throws `std::future_error`.

---

## Question 4
What advantage does void future have over condition variable for simple signaling?

A) Better performance  
B) Simpler - no mutex or boolean flag needed  
C) Can signal multiple times  
D) Supports more waiters  

**Answer:** B

**Explanation:** Void futures require only a promise and future - no mutex, no condition variable, no boolean flag, making them simpler for one-shot events.

---

## Question 5
```cpp
std::condition_variable cv;
std::mutex mtx;
bool ready = false;

// vs

std::promise<void> p;
auto f = p.get_future();
```

How many synchronization primitives in each?

A) Both use 3  
B) CV uses 3, future uses 2  
C) CV uses 2, future uses 1  
D) Both use 2  

**Answer:** B

**Explanation:** Condition variable approach needs mutex, condition variable, AND boolean flag (3). Future approach needs promise and future (2, and the future is created from promise).

---

## Question 6
What happens if you wait on a void future that never gets signaled?

A) Returns immediately  
B) Waits forever (deadlock)  
C) Throws exception after timeout  
D) Undefined behavior  

**Answer:** B

**Explanation:** `wait()` blocks indefinitely until the promise is satisfied. If `set_value()` is never called, it waits forever.

---

## Question 7
How do you enable multiple threads to wait for the same void signal?

A) Multiple promises  
B) shared_future<void>  
C) Multiple condition variables  
D) Not possible  

**Answer:** B

**Explanation:** `std::shared_future<void>` can be copied and waited on by multiple threads, all released when the promise is satisfied.

---

## Question 8
```cpp
std::promise<void> p;
auto sf = p.get_future().share();

// 10 threads all call:
sf.wait();

// Main thread:
p.set_value();
```

How many threads are woken up?

A) 1  
B) Random number  
C) All 10  
D) None  

**Answer:** C

**Explanation:** `set_value()` releases all threads waiting on the shared_future.

---

## Question 9
Can you reuse a void future for multiple signals?

A) Yes  
B) No, it's one-shot only  
C) Only with reset()  
D) Only shared_futures can  

**Answer:** B

**Explanation:** Futures are one-shot. Once the promise is satisfied, you cannot "reset" it. You'd need to create a new promise/future pair.

---

## Question 10
When should you use condition variable instead of void future?

A) Never  
B) When you need repeated signaling  
C) Always  
D) When you have multiple threads  

**Answer:** B

**Explanation:** Condition variables can be signaled repeatedly, making them better for ongoing synchronization. Void futures are one-shot only.

---

## Question 11
```cpp
std::promise<void> p;

void detectActivity() {
    while (scanning) {
        if (activityDetected) {
            p.set_value();  // Problem!
        }
    }
}
```

What's wrong?

A) Nothing  
B) Can only call set_value() once  
C) Should use set_value_at()  
D) Missing mutex  

**Answer:** B

**Explanation:** This tries to signal multiple times, but `set_value()` can only be called once. Subsequent calls throw an exception.

---

## Question 12
What is a "spurious wakeup"?

A) Waking up without being signaled  
B) Waking up multiple threads  
C) Waking up the wrong thread  
D) Waking up too quickly  

**Answer:** A

**Explanation:** Condition variables can wake up without being notified (spurious wakeup), requiring you to recheck the condition. Futures don't have this problem.

---

## Question 13
Do void futures have spurious wakeups?

A) Yes, same as condition variables  
B) No, they wake up only when signaled  
C) Only on some platforms  
D) Only with shared_future  

**Answer:** B

**Explanation:** Futures don't have spurious wakeups - they only wake up when the promise is actually satisfied.

---

## Question 14
```cpp
class Resource {
    std::promise<void> ready;
    std::shared_future<void> readyFuture;
public:
    Resource() : readyFuture(ready.get_future()) {}
    
    void init() {
        // Initialize...
        ready.set_value();
    }
    
    void use() {
        readyFuture.wait();  // Wait for init
        // Use resource...
    }
};
```

What pattern is this?

A) Lazy initialization  
B) One-time initialization synchronization  
C) Resource pool  
D) Singleton  

**Answer:** B

**Explanation:** Multiple threads can call `use()` which waits for initialization to complete once, then all proceed. Classic one-shot initialization pattern.

---

## Question 15
What's the minimum code needed for one-shot signaling with void future?

A) 10+ lines  
B) 5-7 lines  
C) 2-3 lines  
D) 15+ lines  

**Answer:** C

**Explanation:** Just create promise, get future, wait on future, set promise value - very minimal code.

---

## Question 16
```cpp
void shutdownWorkers(std::vector<std::thread>& workers) {
    std::promise<void> shutdown;
    auto sf = shutdown.get_future().share();
    
    // Workers check: if (sf.wait_for(0) == ready) return;
    
    shutdown.set_value();  // Signal all workers
}
```

What's this doing?

A) Starting workers  
B) Broadcasting shutdown signal to all workers  
C) Restarting workers  
D) Checking worker status  

**Answer:** B

**Explanation:** Uses shared_future to broadcast a shutdown signal to all workers simultaneously - perfect use of void futures.

---

## Question 17
How do condition variables handle multiple waiters?

A) Only one wakes up  
B) Use notify_all() to wake all  
C) Automatically wake all  
D) Can't have multiple waiters  

**Answer:** B

**Explanation:** Must explicitly use `notify_all()` to wake all waiters. `notify_one()` wakes only one, and you must manage this.

---

## Question 18
How do void futures handle multiple waiters?

A) Automatically wake all (with shared_future)  
B) Need to call wake_all()  
C) Only one wakes up  
D) Can't have multiple waiters  

**Answer:** A

**Explanation:** All threads waiting on a `shared_future` are automatically released when the promise is satisfied - no special call needed.

---

## Question 19
```cpp
std::promise<void> p;
auto f = p.get_future();
f.wait();
f.wait();  // Is this safe?
```

A) Yes, waits again  
B) Yes, returns immediately (already signaled)  
C) No, undefined behavior  
D) No, throws exception  

**Answer:** B

**Explanation:** After the promise is satisfied, subsequent `wait()` calls return immediately - the future remains valid and ready.

---

## Question 20
What's the best use case for void futures?

A) Repeated synchronization  
B) Complex conditional logic  
C) Simple one-shot "ready" or "go" signals  
D) Data transfer between threads  

**Answer:** C

**Explanation:** Void futures excel at simple one-shot events like "initialization complete", "start now", or "shutdown" - where you just need a signal, not data.

---

## Bonus Question 21
Can a void future be used for cancellation tokens?

A) No, futures can't be cancelled  
B) Yes, signal represents cancellation request  
C) Only with special library  
D) Only in C++20  

**Answer:** B

**Explanation:** You can check `wait_for(0)` to see if cancellation was requested (promise satisfied), making it usable as a simple cancellation token.

---

## Answer Key

1. C - One-shot signaling
2. B - Wait until signal
3. A - Once only
4. B - Simpler code
5. B - CV:3, future:2
6. B - Waits forever
7. B - shared_future
8. C - All 10 threads
9. B - One-shot only
10. B - Repeated signaling
11. B - Only once
12. A - Wake without signal
13. B - No spurious wakeups
14. B - One-time init sync
15. C - 2-3 lines
16. B - Shutdown broadcast
17. B - notify_all()
18. A - Automatic all
19. B - Returns immediately
20. C - One-shot signals
21. B - Cancellation signal

---

## Scoring Guide

- 18-21: Expert level ⭐⭐⭐
- 15-17: Good understanding ⭐⭐
- 12-14: Basic understanding ⭐
- <12: Review Item 39 material

---

## Common Mistakes

1. **Trying to reuse void futures** - They're one-shot only
2. **Not using shared_future for multiple waiters** - Regular future can't be copied
3. **Thinking futures have spurious wakeups** - They don't, unlike CVs
4. **Using futures for repeated signaling** - Use condition variables instead
5. **Forgetting the simplicity advantage** - No mutex/flag needed
