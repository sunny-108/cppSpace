# Design Patterns Interview Questions

Based on your experience implementing design patterns in enterprise backup/restore systems and plugin frameworks.

---

## **Creational Patterns**

### Singleton Pattern
1. What is the Singleton pattern? When should you use it?
2. How do you implement a thread-safe Singleton in C++? Show different approaches.
3. What are the drawbacks of using Singleton? How do you mitigate them?
4. How did you use Singleton for "global resource coordination" in your thread pool?
5. What is the difference between eager and lazy initialization in Singleton?
6. How do you unit test code that uses Singleton?
7. Explain the Meyers' Singleton. Why is it preferred in modern C++?

### Factory Pattern
8. Explain the Factory Method pattern. How does it differ from Abstract Factory?
9. How did you implement Factory pattern for "platform-specific checkers" in the Catalyst Plugin Installer?
10. When would you choose Factory over direct object construction?
11. How do you handle object creation with varying parameters using Factory?
12. What is the relationship between Factory and Dependency Injection?

### Object Pool Pattern
13. What is the Object Pool pattern? What problem does it solve?
14. How did you use Object Pool for "thread lifecycle management"?
15. When is Object Pool beneficial? When should you avoid it?
16. How do you handle object cleanup/reset when returning to the pool?
17. How do you make an Object Pool thread-safe?

### Builder Pattern
18. What is the Builder pattern? When is it preferred over constructors?
19. How does Builder pattern help with immutable objects?
20. Explain the difference between Builder and Factory patterns.

---

## **Structural Patterns**

### Adapter Pattern
21. What is the Adapter pattern? Give a real-world example.
22. What is the difference between class adapter and object adapter?
23. How would you use Adapter to integrate legacy code with new interfaces?

### Facade Pattern
24. What is the Facade pattern? How does it simplify complex systems?
25. How did your "hybrid C++/CLI architecture" act as a facade for ADO.NET access?
26. What is the difference between Facade and Adapter patterns?

### Bridge Pattern
27. What is the Bridge pattern? When would you use it?
28. How does Bridge pattern support platform-independent abstractions?

### Decorator Pattern
29. What is the Decorator pattern? How does it differ from inheritance?
30. Give an example where Decorator is more flexible than subclassing.

---

## **Behavioral Patterns**

### Command Pattern
31. What is the Command pattern? What are its components?
32. How did you implement Command pattern for "job execution" in your thread pool?
33. How did you use Command pattern for "install/uninstall/upgrade operations"?
34. How does Command pattern support undo/redo functionality?
35. What is the relationship between Command and Queue patterns in job systems?

### Strategy Pattern
36. What is the Strategy pattern? How does it differ from Template Method?
37. How did you implement Strategy for "plugin-specific validation" in the installer framework?
38. When would you choose Strategy over polymorphism?
39. How do you select strategies at runtime?

### Observer Pattern
40. What is the Observer pattern? Give a real-world example.
41. How do you implement Observer pattern in a thread-safe manner?
42. What is the difference between push and pull models in Observer?
43. How would you use Observer for monitoring backup job status?

### State Pattern
44. What is the State pattern? How does it differ from Strategy?
45. How did you use "state machine" to resolve memory corruption in RMAN plugin?
46. When should you use State pattern vs switch statements?
47. How do you handle state transitions in concurrent environments?

### Template Method Pattern
48. What is the Template Method pattern?
49. How does it enforce a consistent algorithm structure?
50. What is the difference between Template Method and Strategy?

---

## **Concurrency Patterns**

### Producer-Consumer Pattern
51. Explain the Producer-Consumer pattern in detail.
52. How did you implement Producer-Consumer in your custom thread pool?
53. What synchronization mechanisms did you use (events, mutexes, condition variables)?
54. How do you handle the case when the queue is full or empty?
55. How do you gracefully shutdown a Producer-Consumer system?

### Thread Pool Pattern
56. What is the Thread Pool pattern? What problems does it solve?
57. How did you design your thread pool with configurable worker threads (1-4)?
58. How do you implement job dispatching with "free-thread detection algorithm"?
59. What is work stealing in thread pools? When is it beneficial?
60. How do you handle task priorities in a thread pool?
61. How did you use Windows API (CreateEvent, WaitForMultipleObjects) for synchronization?
62. How would you implement the same thread pool using only C++ standard library?

### Active Object Pattern
63. What is the Active Object pattern?
64. How does it decouple method execution from method invocation?

### Monitor Pattern
65. What is the Monitor pattern in concurrent programming?
66. How do condition variables relate to the Monitor pattern?

---

## **Architectural Patterns**

### Repository Pattern
67. What is the Repository pattern? When would you use it?
68. How does it abstract data access logic?

### Dependency Injection
69. What is Dependency Injection? What are its types (constructor, setter, interface)?
70. How does DI improve testability?
71. How would you implement DI in C++ without a framework?

---

## **Pattern Application & Design Decisions**

72. How do you decide which design pattern to use for a given problem?
73. What are anti-patterns? Give examples you've encountered.
74. How do you balance pattern usage vs. over-engineering?
75. How did combining multiple patterns (Command + Object Pool + Singleton) help in your thread pool design?
76. What patterns would you use for a plugin architecture?
77. How do patterns help with code maintainability and testing?
78. Explain how you would refactor legacy code to introduce design patterns incrementally.
79. What is the relationship between SOLID principles and design patterns?
80. How do you document design pattern usage in your codebase?
