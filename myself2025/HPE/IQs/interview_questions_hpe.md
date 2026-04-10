# Interview Questions - Hewlett Packard Enterprise
## System Software Engineer (December 2018 – Present)

---

## **1. Modern C++ Thread Pool Refactoring**

### **1.1 Refactoring Strategy & Design**
1. You modernized a legacy Windows API thread pool to C++14/17 standards. What was the original architecture, and what were the main pain points that drove the refactoring?
2. Walk me through the migration from Windows Events (`CreateEvent`, `WaitForMultipleObjects`) to `std::condition_variable`. What challenges did you face?
3. How did you replace Windows mutexes with `std::mutex`? Were there any behavioral differences you had to account for?
4. Explain the migration from Windows API threads to `std::thread`. How did this improve portability?
5. What smart pointers did you adopt during the refactoring, and how did they improve resource management?

### **1.2 Modern C++ Synchronization**
6. How did you refactor the RAII-based lock management to use `std::lock_guard` and `std::unique_lock`? What are the advantages over the custom lock wrappers?
7. Explain the migration from polling (Sleep) to condition variable-based notification. What performance improvements did you observe?
8. How did you use `std::condition_variable::wait()` with predicates to prevent spurious wakeups?
9. Did you encounter any deadlock scenarios during the refactoring? How did modern C++ features help prevent them?
10. How did you ensure backward compatibility during the gradual refactoring process?

### **1.3 Code Quality & Maintainability Improvements**
11. How did the modern C++ refactoring improve code maintainability compared to the Windows API version?
12. What were the testing strategies you used to ensure the refactored code maintained functional equivalence?
13. How did you use smart pointers to eliminate manual resource management in the thread pool?
14. Did the refactoring maintain the same performance characteristics? How did you validate this?
15. What C++14/17 features (beyond threading) did you adopt during the refactoring (move semantics, lambdas, etc.)?

### **1.4 Refactoring Patterns & Best Practices**
16. How did you preserve the Command pattern during refactoring? Were there opportunities to improve it with modern C++?
17. Did the Object Pool pattern implementation change with `std::thread`? How do you manage thread reuse?
18. How did you modernize the Singleton pattern? Did you use C++11's thread-safe static initialization?
19. What refactoring patterns (Strangler Fig, Branch by Abstraction) did you use for incremental migration?

---

## **2. Memory Leak Resolution**

### **2.1 RMAN Plugin**
20. What tools did you use to identify memory leaks in the RMAN plugin?
21. Can you describe a specific memory leak you found? What was the root cause?
22. How did memory leaks manifest in long-running operations? What symptoms did you observe?
23. Walk me through your debugging process from detection to fix.

### **2.2 SAP-HANA Plugin**
24. What memory leaks did you identify in the SAP-HANA plugin?
25. You mentioned fixing "concurrent backup expiration failures." What was causing these failures?
26. Explain your process-level mutex implementation. Why process-level instead of thread-level?
27. How did you identify the race condition in credential file access?
28. What was your testing strategy to ensure the fix didn't introduce new issues?

### **2.3 Smart Pointers & RAII**
29. You implemented "comprehensive smart pointer adoption across legacy codebase." What types of smart pointers did you use (`unique_ptr`, `shared_ptr`, `weak_ptr`)?
30. Can you give an example of converting raw pointer code to smart pointer code?
31. How did you handle smart pointer usage across DLL boundaries or when interfacing with C APIs?
32. What challenges did you face when introducing smart pointers to a legacy codebase?

---

## **3. SQL Catalyst Plugin (C++ / Windows Server)**

### **3.1 Thread-Safe Communication Layer**
33. Explain your thread-safe communication layer architecture.
34. You mentioned "reader-writer locks enabling parallel backup streams." How did you implement reader-writer locks?
35. When do readers acquire locks vs. writers? What's your priority scheme (reader-preferred, writer-preferred, or fair)?
36. How many parallel backup streams can your system handle? What limits this number?
37. How did you optimize memory allocation for streaming data?

### **3.2 Performance & Data Flow**
38. Walk me through the data flow from SQL Server to StoreOnce through your plugin.
39. How do you handle backpressure when StoreOnce is slower than SQL Server?
40. What buffer management strategies did you implement?
41. How did you test the thread safety of your communication layer?

---

## **4. SAP-HANA Catalyst Plugin (C++ / Linux)**

### **4.1 Multi-threaded Backup Orchestration**
42. Describe the multi-threaded backup orchestration architecture.
43. How did you use condition variables for synchronization? Give a specific scenario.
44. How do you coordinate multiple threads backing up different SAP-HANA data volumes?
45. What happens if one backup thread fails? How do you handle cleanup?

### **4.2 Connection Pooling & Memory**
46. Explain your connection pooling implementation for SAP-HANA.
47. What memory leaks did you find in connection pooling, and how did you fix them?
48. How do you manage connection lifecycle (creation, reuse, timeout, destruction)?
49. How many connections do you maintain in the pool? How did you determine this number?

### **4.3 Credential File Race Condition**
50. You mentioned "concurrent backup expiration failures" due to credential file access. Explain the race condition in detail.
51. How does your process-level mutex solution work? What API did you use (flock, fcntl, named semaphore)?
52. Why was a process-level mutex necessary instead of a thread-level mutex?
53. How did you test that the race condition was resolved?

---

## **5. RMAN Catalyst Plugin (C++ / Linux & Windows)**

### **5.1 Concurrency Refactoring**
54. You refactored single-threaded code to support concurrent backup channels. What was the original architecture?
55. What challenges did you face when introducing concurrency to existing single-threaded code?
56. How do you handle multiple concurrent RMAN channels writing to different backup pieces?
57. What synchronization primitives did you use for this refactoring?

### **5.2 Memory Corruption**
58. You resolved "memory corruption in shared buffer access." Describe the corruption issue.
59. What debugging techniques did you use to identify the corruption?
60. How did you fix the shared buffer access pattern?
61. What tools did you use (Valgrind, AddressSanitizer, etc.) to verify the fix?

---

## **6. Install-Update Component (C++ & Java / Linux)**

### **6.1 Concurrent Upgrade Framework**
62. Explain your concurrent upgrade framework architecture.
63. What is "barrier synchronization," and how did you implement it?
64. How do you ensure all components reach a synchronization point before proceeding?
65. What happens if one component fails during upgrade?

### **6.2 Rollback Mechanisms**
66. Describe the memory leaks you found in rollback mechanisms.
67. How do rollback mechanisms work in your upgrade framework?
68. How did you fix these memory leaks?
69. What testing did you perform to ensure rollback works correctly?

---

## **7. Modular Update System (Team Lead)**

### **7.1 Leadership & Architecture**
70. You led a team of 4 engineers. How did you divide responsibilities?
71. Describe the component-level upgrade architecture you designed.
72. What does "fine-grained concurrency control" mean in this context?
73. How did you achieve zero-downtime updates?

### **7.2 Technical Implementation**
74. How do you upgrade individual components without affecting others?
75. What locking strategy did you implement to prevent conflicts during upgrades?
76. How do you handle dependencies between components during upgrade?
77. What was the most challenging technical problem you faced, and how did you solve it?

---

## **8. General Concurrency & Threading**

### **8.1 Synchronization Primitives**
78. Compare mutexes, semaphores, and condition variables. When would you use each?
79. Explain the difference between recursive and non-recursive mutexes. When have you used each?
80. What is spurious wakeup in condition variables? How do you handle it?
81. Have you used atomic operations (`std::atomic`)? In what scenarios?

### **8.2 Deadlock & Race Conditions**
82. Describe a deadlock scenario you encountered. How did you debug it?
83. What strategies do you use to prevent deadlocks (lock ordering, timeout, etc.)?
84. Explain the difference between data race and race condition.
85. What tools have you used to detect race conditions (ThreadSanitizer, Helgrind)?

### **8.3 Lock-Free Programming**
86. You mentioned "lock-free programming" in your competencies. What lock-free data structures have you implemented?
87. When would you choose lock-free over lock-based synchronization?
88. What are the challenges of lock-free programming?

---

## **9. Memory Management Deep Dive**

### **9.1 Leak Detection & Analysis**
89. Walk me through your typical workflow for identifying memory leaks in production.
90. What Valgrind options do you use for memory leak detection?
91. How do you distinguish between actual leaks and "still reachable" memory?
92. Have you used AddressSanitizer? How does it compare to Valgrind?

### **9.2 Heap Profiling**
93. What tools have you used for heap profiling?
94. How do you identify which allocations are consuming the most memory?
95. Describe a scenario where heap profiling helped you optimize memory usage.

---

## **10. Platform-Specific Questions**

### **10.1 Linux vs. Windows**
96. You've worked on both Linux and Windows. What are the key differences in threading APIs?
97. How do Windows Events compare to Linux futex or POSIX condition variables?
98. What differences have you noticed in memory management between the two platforms?

### **10.2 Multi-Platform Considerations**
99. How do you write portable code that works on both Linux and Windows?
100. Have you used platform abstraction layers? How did you design them?

---

## **11. Performance & Optimization**

### **11.1 Profiling & Benchmarking**
101. What profiling tools have you used (perf, gprof, Visual Studio Profiler)?
102. How do you identify performance bottlenecks?
103. Describe a specific performance optimization you implemented and its impact.

### **11.2 Resource Management**
104. How do you optimize resource usage in long-running backup operations?
105. What strategies do you use for buffer management in streaming scenarios?
106. How do you balance memory usage vs. performance?

---

## **12. Code Quality & Best Practices**

### **12.1 Code Reviews & Testing**
107. What code review practices do you follow?
108. How do you test multi-threaded code?
109. What unit testing frameworks have you used (Google Test, Catch2)?
110. How do you ensure thread safety in your unit tests?

### **12.2 Documentation & Maintenance**
111. How do you document complex concurrent systems?
112. What strategies do you use to make concurrent code maintainable?
113. How do you onboard new team members to a complex concurrent codebase?

---

## **13. Scenario-Based Questions**

114. **Scenario:** A customer reports that backup operations slow down over time. How would you investigate?
115. **Scenario:** You discover a race condition in production. Walk me through your debugging and fix process.
116. **Scenario:** You need to add a new feature that requires modifying shared state accessed by multiple threads. How do you approach this?
117. **Scenario:** Your thread pool occasionally hangs. How would you debug this?
118. **Scenario:** Memory usage grows unbounded during long backup operations. How do you investigate?

---

## **14. Problem-Solving & Critical Thinking**

119. What was the most challenging concurrency bug you've debugged? How did you solve it?
120. Describe a situation where you had to make a trade-off between performance and code simplicity.
121. How do you stay updated with modern C++ features and best practices?
122. What would you do differently if you were to redesign your thread pool today?

---

## **15. Modern C++ Features**

### **15.1 C++14/17**
123. What C++14/17 features have you used in your projects?
124. How have you used move semantics to optimize performance?
125. Give examples of how you've used lambda functions in concurrent code.
126. Have you used `std::optional`, `std::variant`, or `std::any`? In what scenarios?

### **15.2 Concurrency Features**
127. Have you used `std::thread`, `std::mutex`, `std::condition_variable` from C++11?
128. Why did you choose Windows API over C++ standard library for threading?
129. What are the advantages and disadvantages of each approach?
