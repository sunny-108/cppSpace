# C++ Interview Questions

Based on your experience with multi-threaded C++ systems programming, memory management, and modern C++ features.

---

## **Threading & Concurrency**

### Basic Level
1. What is the difference between `std::mutex` and `std::recursive_mutex`? When would you use each?
2. Explain the difference between `std::lock_guard` and `std::unique_lock`.
3. What is a race condition? How do you prevent it?
4. What is the purpose of `std::condition_variable`? How does it work with mutexes?
5. Explain the difference between `std::thread::join()` and `std::thread::detach()`.

### Intermediate Level
6. How does `std::condition_variable::wait()` work internally? Why must it be used with a mutex?
7. What is spurious wakeup? How do you handle it in your code?
8. Explain the Producer-Consumer pattern. How did you implement it in your thread pool?
9. What is the difference between `notify_one()` and `notify_all()`? When would you use each?
10. How do you implement a thread-safe singleton in C++11 and later?
11. What are atomic operations? When would you use `std::atomic` vs a mutex?
12. Explain lock-free programming. What are its advantages and challenges?

### Advanced Level
13. What is the ABA problem in lock-free programming? How do you solve it?
14. How did you migrate from Windows Events to `std::condition_variable`? What challenges did you face?
15. Explain memory ordering in C++ (`memory_order_relaxed`, `memory_order_acquire`, `memory_order_release`, etc.)
16. How do you detect and prevent deadlocks? What is lock ordering?
17. What is ThreadSanitizer? How did you use it to verify your memory corruption fix?
18. Explain the "double-checked locking" pattern. Why was it broken before C++11?
19. How would you implement a read-write lock? When is it beneficial?
20. What is a memory barrier/fence? Why is it needed in concurrent programming?

---

## **Memory Management**

### Basic Level
21. What is RAII? Why is it important in C++?
22. Explain the difference between `std::unique_ptr`, `std::shared_ptr`, and `std::weak_ptr`.
23. What is a memory leak? How do you detect one?
24. What is the Rule of Three/Five/Zero in C++?
25. What is the difference between stack and heap memory allocation?

### Intermediate Level
26. How does `std::shared_ptr` maintain reference counting? Is it thread-safe?
27. What is a circular reference? How does `std::weak_ptr` solve it?
28. Explain `std::make_unique` and `std::make_shared`. Why are they preferred over raw `new`?
29. How did you achieve 25% memory reduction in the SQL Catalyst Plugin?
30. What tools do you use for memory leak detection? Explain your experience with Valgrind.
31. What is AddressSanitizer? How does it differ from Valgrind?
32. Explain custom deleters with smart pointers. When would you use them?

### Advanced Level
33. How did you resolve the 7.8 GB memory leak in the RMAN plugin? Walk through your debugging process.
34. Explain memory fragmentation. How do you mitigate it in long-running applications?
35. What is placement new? When would you use it?
36. How do you manage COM interface lifetimes with smart pointers?
37. Explain object pooling for memory optimization. How did you implement it?
38. What are allocators in C++? When would you write a custom allocator?

---

## **Modern C++ (C++11/14/17)**

### Basic Level
39. What are lambda expressions? Explain capture modes (`[=]`, `[&]`, `[this]`).
40. What is move semantics? What problem does it solve?
41. Explain `std::move` and `std::forward`.
42. What are `auto` and `decltype`? When should you use them?
43. What is `nullptr` and why was it introduced?

### Intermediate Level
44. Explain perfect forwarding. Why is it useful?
45. What is SFINAE? How is it used in template metaprogramming?
46. Explain `constexpr`. How does it differ between C++11 and C++14/17?
47. What are variadic templates? Give an example use case.
48. What is structured bindings (C++17)? How does it improve code readability?
49. Explain `std::optional`, `std::variant`, and `std::any` (C++17).
50. What are fold expressions (C++17)?

### Advanced Level
51. How did you modernize legacy Windows API code to C++14/17 standards?
52. Explain the difference between `std::function` and function pointers. Performance implications?
53. What is type erasure? How is it implemented?
54. Explain CRTP (Curiously Recurring Template Pattern). When would you use it?
55. What are user-defined literals? Give practical examples.

---

## **COM & Windows API (Based on Your Experience)**

56. What is COM (Component Object Model)? Explain `IUnknown` interface.
57. How do you manage COM object lifetimes? What is `AddRef()` and `Release()`?
58. How did you implement RAII wrappers for COM interfaces?
59. Explain the difference between apartment threading models (STA vs MTA).
60. What is marshalling in COM? When is it needed?

---

## **Debugging & Profiling**

61. How do you debug a multi-threaded application? What tools do you use?
62. Explain your approach to debugging a deadlock in production.
63. How do you use GDB for debugging multi-threaded applications?
64. What is a core dump? How do you analyze it?
65. How do you profile CPU and memory usage in C++ applications?

---

## **General C++ Concepts**

66. What is the difference between `const` and `constexpr`?
67. Explain virtual functions and vtable mechanism.
68. What is object slicing? How do you prevent it?
69. Explain the difference between `static_cast`, `dynamic_cast`, `const_cast`, and `reinterpret_cast`.
70. What is undefined behavior? Give examples and explain how to avoid it.
71. What is the One Definition Rule (ODR)?
72. Explain inline functions and their implications.
73. What is the difference between `struct` and `class` in C++?
74. Explain exception safety guarantees (basic, strong, no-throw).
75. What is PIMPL idiom? When would you use it?
