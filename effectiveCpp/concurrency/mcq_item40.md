# MCQ Test - Item 40: Use std::atomic for Concurrency, volatile for Special Memory

## Instructions
- Each question has one correct answer unless otherwise specified
- Focus on atomic vs volatile distinction
- Time limit: 20 minutes
- Passing score: 70%

---

## Question 1
What is the PRIMARY purpose of `std::atomic`?

A) Prevent compiler optimization  
B) Thread-safe concurrent access  
C) Access hardware registers  
D) Faster performance  

**Answer:** B

**Explanation:** `std::atomic` provides thread-safe, race-free concurrent access to variables with atomic operations and memory ordering guarantees.

---

## Question 2
What is the PRIMARY purpose of `volatile`?

A) Thread synchronization  
B) Prevent race conditions  
C) Prevent compiler optimization of memory access  
D) Faster multi-threaded code  

**Answer:** C

**Explanation:** `volatile` tells the compiler not to optimize away reads/writes - used for memory-mapped I/O and special memory, NOT for threading.

---

## Question 3
```cpp
volatile int counter = 0;

void increment() {
    ++counter;  // Is this thread-safe?
}
```

A) Yes, volatile makes it thread-safe  
B) No, this has a race condition  
C) Only on some platforms  
D) Yes, if counter is global  

**Answer:** B

**Explanation:** `volatile` does NOT provide atomicity. `++counter` is a read-modify-write that can be interrupted by other threads, causing races.

---

## Question 4
```cpp
std::atomic<int> counter{0};

void increment() {
    ++counter;  // Is this thread-safe?
}
```

A) Yes, atomic guarantees thread safety  
B) No, still has race  
C) Only with mutex  
D) Only on x86 platforms  

**Answer:** A

**Explanation:** `std::atomic` makes the increment operation atomic and thread-safe, preventing race conditions.

---

## Question 5
What does this code do?

```cpp
volatile int* hardwareReg = (volatile int*)0x40000000;
int value = *hardwareReg;
```

A) Thread-safe read  
B) Read from specific memory address without optimization  
C) Atomic read  
D) Compile error  

**Answer:** B

**Explanation:** `volatile` ensures the compiler actually reads from the specified address (hardware register) and doesn't cache or optimize away the read.

---

## Question 6
Can `volatile` prevent race conditions?

A) Yes, that's its purpose  
B) No, it only prevents optimization  
C) Only with atomic operations  
D) Only on x86 architecture  

**Answer:** B

**Explanation:** `volatile` does NOT prevent race conditions. It only prevents the compiler from optimizing away memory accesses.

---

## Question 7
```cpp
std::atomic<int> x{0};
x++;
x++;
```

What is guaranteed?

A) Both increments visible to all threads  
B) Final value is 2  
C) Both A and B  
D) Neither guaranteed  

**Answer:** C

**Explanation:** Each increment is atomic, and memory ordering ensures all threads will eventually see the correct value of 2.

---

## Question 8
```cpp
volatile int x = 0;
x++;
x++;
```

In multithreaded code, what's guaranteed?

A) Final value is 2  
B) No optimization of the increments  
C) Thread-safe access  
D) Both A and C  

**Answer:** B

**Explanation:** `volatile` only prevents optimization - the compiler must actually perform both increments. But with multiple threads, the final value is NOT guaranteed to be 2.

---

## Question 9
When should you use `volatile`?

A) Multi-threaded counters  
B) Memory-mapped hardware registers  
C) Shared flags between threads  
D) Lock-free data structures  

**Answer:** B

**Explanation:** Use `volatile` for hardware registers and special memory where reads/writes must actually happen (not be optimized away).

---

## Question 10
When should you use `std::atomic`?

A) Hardware I/O  
B) Signal handlers  
C) Shared variables between threads  
D) To prevent optimization  

**Answer:** C

**Explanation:** Use `std::atomic` for any variable accessed by multiple threads to ensure thread-safe, race-free access.

---

## Question 11
```cpp
std::atomic<int> x{5};
x.store(10);
int y = x.load();
```

What does this code do?

A) Compile error  
B) Atomic write and read operations  
C) Same as regular assignment  
D) Requires mutex  

**Answer:** B

**Explanation:** `store()` and `load()` are explicit atomic write and read operations, making the intent clear.

---

## Question 12
```cpp
volatile int* reg = ...;
int x = *reg;
int y = *reg;
```

Will the compiler optimize this to a single read?

A) Yes  
B) No, volatile prevents optimization  
C) Only in release builds  
D) Depends on compiler  

**Answer:** B

**Explanation:** `volatile` ensures both reads actually occur - the compiler cannot assume the value hasn't changed between reads.

---

## Question 13
Can you combine `std::atomic` and `volatile`?

A) No, compile error  
B) Yes: std::atomic<int> volatile x  
C) Yes but rarely needed  
D) Never useful  

**Answer:** C

**Explanation:** You can declare `std::atomic<T> volatile`, combining both semantics, but it's rarely needed (maybe for memory accessed by both threads and hardware).

---

## Question 14
```cpp
std::atomic<bool> done{false};

while (!done) {
    // Work
}
```

Is this safe for thread synchronization?

A) No, need volatile  
B) Yes, atomic provides synchronization  
C) Only with memory_order_seq_cst  
D) No, need mutex  

**Answer:** B

**Explanation:** `std::atomic` provides the memory ordering needed for thread synchronization. The atomic read ensures visibility.

---

## Question 15
```cpp
volatile bool done = false;

while (!done) {
    // Work  
}
```

Is this safe for thread synchronization?

A) Yes, volatile ensures visibility  
B) No, volatile doesn't provide synchronization  
C) Only on x86  
D) Yes, with -O2 optimization  

**Answer:** B

**Explanation:** `volatile` does NOT provide memory ordering or synchronization guarantees needed for thread communication.

---

## Question 16
What operations does `std::atomic` provide that `volatile` doesn't?

A) Memory ordering  
B) Atomicity  
C) Thread synchronization  
D) All of the above  

**Answer:** D

**Explanation:** `std::atomic` provides memory ordering, atomic operations, and thread synchronization - none of which `volatile` provides.

---

## Question 17
```cpp
std::atomic<int> x{0};
x = 5;
```

Is this atomic?

A) No, need x.store(5)  
B) Yes, operator= is atomic for std::atomic  
C) Only if single-threaded  
D) Undefined behavior  

**Answer:** B

**Explanation:** `std::atomic` overloads `operator=` to perform atomic store operations.

---

## Question 18
What does `compare_exchange_strong` do?

A) Strong comparison  
B) Atomic compare-and-swap  
C) Throws if fails  
D) Requires volatile  

**Answer:** B

**Explanation:** Compare-and-swap atomically compares the value with expected, and if equal, swaps in the desired value - fundamental for lock-free programming.

---

## Question 19
Can `volatile` be used for signal handlers?

A) No, never  
B) Yes, with sig_atomic_t  
C) Only with std::atomic  
D) Not recommended  

**Answer:** B

**Explanation:** For signal handlers, use `volatile sig_atomic_t` which is specifically designed for this use case.

---

## Question 20
```cpp
std::atomic<int> counter{0};
```

How many threads can safely increment this?

A) 1  
B) 2  
C) Number of CPU cores  
D) Unlimited  

**Answer:** D

**Explanation:** `std::atomic` is thread-safe for any number of threads concurrently accessing it.

---

## Question 21
What's the performance cost of `std::atomic` vs regular int?

A) No cost  
B) Some overhead for synchronization  
C) 10x slower  
D) Depends on volatile  

**Answer:** B

**Explanation:** Atomic operations have some overhead (memory barriers, atomic instructions) but it's necessary for correctness in multithreaded code.

---

## Question 22
```cpp
volatile int x = 10;
int y = x + x;
```

What's guaranteed?

A) y = 20  
B) x is read twice from memory  
C) Thread-safe  
D) Atomic addition  

**Answer:** B

**Explanation:** `volatile` ensures `x` is read twice from memory (not cached in register), but doesn't guarantee the final value in multithreaded code.

---

## Question 23
What memory ordering does `std::atomic` use by default?

A) memory_order_relaxed  
B) memory_order_acquire  
C) memory_order_seq_cst  
D) No ordering  

**Answer:** C

**Explanation:** Default is `memory_order_seq_cst` (sequentially consistent), which provides the strongest ordering guarantees.

---

## Question 24
Does `volatile` provide any memory ordering?

A) Yes, same as atomic  
B) No, none at all  
C) Only on x86  
D) Only with barriers  

**Answer:** B

**Explanation:** `volatile` provides NO memory ordering guarantees - it only prevents the compiler from optimizing away accesses.

---

## Question 25
```cpp
std::atomic<int*> ptr{nullptr};
```

Is this valid?

A) No, can't make pointers atomic  
B) Yes, std::atomic supports pointers  
C) Only with volatile  
D) Only for shared_ptr  

**Answer:** B

**Explanation:** `std::atomic` can be instantiated with pointer types, providing thread-safe pointer operations.

---

## Answer Key

1. B - Thread-safe access
2. C - Prevent optimization
3. B - Race condition
4. A - Thread-safe
5. B - Hardware register read
6. B - Only prevents optimization
7. C - Both guaranteed
8. B - No optimization only
9. B - Hardware registers
10. C - Shared between threads
11. B - Atomic operations
12. B - No optimization
13. C - Rarely needed combo
14. B - Provides synchronization
15. B - No synchronization
16. D - All of above
17. B - operator= is atomic
18. B - Compare-and-swap
19. B - With sig_atomic_t
20. D - Unlimited threads
21. B - Some overhead
22. B - Read twice
23. C - seq_cst default
24. B - No ordering
25. B - Pointers supported

---

## Scoring Guide

- 22-25: Expert level ⭐⭐⭐
- 18-21: Good understanding ⭐⭐
- 14-17: Basic understanding ⭐
- <14: Review Item 40 material

---

## Common Mistakes

1. **Using volatile for threading** - Wrong! Use std::atomic
2. **Thinking volatile prevents races** - It doesn't
3. **Using atomic for hardware I/O** - Use volatile instead
4. **Not understanding memory ordering** - Critical for atomics
5. **Assuming volatile is thread-safe** - It absolutely is NOT

---

## Key Distinctions

| Feature | `std::atomic` | `volatile` |
|---------|---------------|------------|
| **Purpose** | Threading | Special memory |
| **Atomicity** | ✅ Yes | ❌ No |
| **Memory Ordering** | ✅ Yes | ❌ No |
| **Thread-Safe** | ✅ Yes | ❌ No |
| **Prevents Optimization** | ⚠️ For atomic ops | ✅ Yes |

---

## Remember

- **`std::atomic`** = Use for threads
- **`volatile`** = Use for hardware
- They are NOT interchangeable!
