# MCQ Test - Item 38: Be Aware of Varying Thread Handle Destructor Behavior

## Instructions
- Each question has one correct answer unless otherwise specified
- Focus on future destructor blocking behavior
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
Under what conditions does a `std::future` destructor block?

A) Always  
B) Never  
C) When created by std::async with async policy and it's the last reference  
D) Only if you call get()  

**Answer:** C

**Explanation:** A future blocks on destruction if: (1) created by `std::async`, (2) with `async` launch policy, and (3) it's the last reference to the shared state.

---

## Question 2
```cpp
{
    auto future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(2s);
    });
}  // What happens here?
```

A) Returns immediately  
B) Blocks for ~2 seconds  
C) Undefined behavior  
D) Compile error  

**Answer:** B

**Explanation:** This future meets all three conditions for blocking: from `std::async`, `async` policy, and last reference. Destructor waits for task completion.

---

## Question 3
```cpp
std::packaged_task<void()> pt(task);
auto future = pt.get_future();
std::thread t(std::move(pt));
t.detach();
// future destroyed here
```

Does the future destructor block?

A) Yes  
B) No  
C) Only if task is still running  
D) Depends on platform  

**Answer:** B

**Explanation:** Futures from `std::packaged_task` don't block on destruction - they're not from `std::async`.

---

## Question 4
```cpp
std::promise<void> p;
auto future = p.get_future();
std::thread t([&p]() { /* work */ p.set_value(); });
t.detach();
// future destroyed here
```

Does this block?

A) Yes  
B) No  
C) Only sometimes  
D) Compile error  

**Answer:** B

**Explanation:** Futures from `std::promise` don't block on destruction - only futures from `std::async` with `async` policy block.

---

## Question 5
What is "shared state" in the context of futures?

A) Memory shared between threads  
B) The state shared between a future and its associated promise/task  
C) Global variables  
D) Thread-local storage  

**Answer:** B

**Explanation:** Shared state is the result storage shared between a future and its producer (promise, packaged_task, or async task).

---

## Question 6
```cpp
auto f1 = std::async(std::launch::async, task);
auto f2 = std::move(f1);
// f1 destroyed here
```

Does f1's destructor block?

A) Yes  
B) No  
C) Only if task is done  
D) Undefined behavior  

**Answer:** B

**Explanation:** After moving, `f1` no longer holds the shared state (it's been moved to `f2`), so it's not the "last reference" and doesn't block.

---

## Question 7
```cpp
auto sf = std::async(std::launch::async, task).share();
auto sf2 = sf;
// sf destroyed, sf2 still exists
```

Does sf's destructor block?

A) Yes, it's from async  
B) No, sf2 still holds a reference  
C) Sometimes  
D) Depends on task  

**Answer:** B

**Explanation:** Only the **last** reference to the shared state blocks. Since `sf2` still exists, `sf` is not the last reference.

---

## Question 8
Why was this blocking behavior designed for `std::async` futures?

A) Better performance  
B) Prevent dangling pointers and unsafe detached tasks  
C) Easier to implement  
D) Required by the standard  

**Answer:** B

**Explanation:** Blocking ensures that async tasks complete before futures are destroyed, preventing dangerous "fire and forget" scenarios where tasks might access destroyed data.

---

## Question 9
```cpp
void func() {
    auto future = std::async(std::launch::async, longTask);
    if (error) return;  // Implicit wait here!
}
```

What's the potential problem?

A) Resource leak  
B) Unexpected blocking on early return  
C) Compile error  
D) Undefined behavior  

**Answer:** B

**Explanation:** The future's destructor blocks, waiting for `longTask` to complete. This might not be obvious, causing unexpected delays.

---

## Question 10
How can you make the blocking explicit?

A) auto future = std::async(task).get();  
B) Call future.wait() before return  
C) Use std::launch::deferred  
D) Can't be made explicit  

**Answer:** B

**Explanation:** Explicitly calling `wait()` before the early return makes it clear that you're waiting for the task to complete.

---

## Question 11
```cpp
auto future = std::async(std::launch::deferred, task);
// future destroyed without calling get()
```

Does this block?

A) Yes, still blocks  
B) No, doesn't block  
C) Only if task started  
D) Undefined behavior  

**Answer:** B

**Explanation:** Deferred tasks don't meet the "async policy" condition for blocking. The destructor doesn't block (and the task never runs).

---

## Question 12
Which future source does NOT block on destruction?

A) std::async with async policy  
B) std::packaged_task  
C) std::promise  
D) Both B and C  

**Answer:** D

**Explanation:** Only futures from `std::async` with `async` policy block. Futures from `packaged_task` and `promise` don't block.

---

## Question 13
```cpp
std::vector<std::future<void>> futures;
for (int i = 0; i < 100; ++i) {
    futures.push_back(std::async(std::launch::async, task));
}
futures.clear();  // What happens?
```

A) Returns immediately  
B) Blocks until all 100 tasks complete  
C) Undefined behavior  
D) Clears without waiting  

**Answer:** B

**Explanation:** Each future destructor blocks, so `clear()` waits for all 100 tasks to complete. This might take a long time!

---

## Question 14
What's a better practice than letting future destructors block implicitly?

A) Never use std::async  
B) Call get() or wait() explicitly  
C) Always use deferred policy  
D) Use packaged_task instead  

**Answer:** B

**Explanation:** Explicitly calling `get()` or `wait()` makes the waiting behavior clear and intentional, improving code readability.

---

## Question 15
```cpp
void processData() {
    auto future = std::async(std::launch::async, process);
    doOtherWork();
    // future destroyed here - blocks!
}
```

If `process()` takes 10 seconds but you don't need the result, what happens?

A) Returns immediately after doOtherWork()  
B) Waits 10 seconds at function end  
C) Process is cancelled  
D) Undefined behavior  

**Answer:** B

**Explanation:** The future destructor blocks, waiting for `process()` to complete, even if you don't need the result.

---

## Question 16
```cpp
{
    std::future<int> f;  // Default constructed
}  // Does this block?
```

A) Yes  
B) No  
C) Undefined behavior  
D) Compile error  

**Answer:** B

**Explanation:** Default-constructed futures don't have shared state, so they don't block (they're not from `std::async`).

---

## Question 17
What's the exception to the "non-blocking" rule for packaged_task futures?

A) There is no exception  
B) If you explicitly set a flag  
C) On certain platforms  
D) If running on single-core CPU  

**Answer:** A

**Explanation:** Futures from `packaged_task` and `promise` **never** block on destruction, regardless of circumstances.

---

## Question 18
```cpp
auto future = std::async(std::launch::async | std::launch::deferred, task);
```

Will this block on destruction?

A) Always  
B) Never  
C) Maybe - depends on which policy was chosen  
D) Compile error  

**Answer:** C

**Explanation:** The implementation chooses the policy. If it chose `async`, it blocks. If it chose `deferred`, it doesn't.

---

## Question 19
How many conditions must be met for a future destructor to block?

A) 1  
B) 2  
C) 3  
D) 4  

**Answer:** C

**Explanation:** Three conditions: (1) created by `std::async`, (2) launch policy is `async`, (3) last reference to shared state.

---

## Question 20
```cpp
class FutureHolder {
    std::future<void> fut;
public:
    FutureHolder(std::future<void> f) : fut(std::move(f)) {}
    ~FutureHolder() {
        // Do I need explicit wait here?
    }
};
```

Should the destructor explicitly wait?

A) No, future handles it  
B) Yes, for clarity and documentation  
C) Only if from async  
D) Depends on use case  

**Answer:** B

**Explanation:** While the future destructor might block automatically, explicitly waiting improves code clarity and documents the blocking behavior.

---

## Answer Key

1. C - Three conditions
2. B - Blocks ~2 seconds
3. B - packaged_task doesn't block
4. B - promise doesn't block
5. B - Shared result storage
6. B - Moved-from, no reference
7. B - Not last reference
8. B - Safety from dangling
9. B - Unexpected blocking
10. B - Explicit wait()
11. B - Deferred doesn't block
12. D - Both B and C
13. B - All tasks complete
14. B - Explicit get/wait
15. B - Waits 10 seconds
16. B - No shared state
17. A - No exception
18. C - Depends on policy
19. C - Three conditions
20. B - For clarity

---

## Scoring Guide

- 18-20: Expert level ⭐⭐⭐
- 15-17: Good understanding ⭐⭐
- 12-14: Basic understanding ⭐
- <12: Review Item 38 material

---

## Common Mistakes

1. **Assuming all futures block** - Only async futures do
2. **Not knowing the three conditions** - Source, policy, last reference
3. **Implicit blocking surprises** - Not realizing destructor waits
4. **Forgetting about shared_future** - Only last reference blocks
5. **Not making waits explicit** - Hurts code clarity
