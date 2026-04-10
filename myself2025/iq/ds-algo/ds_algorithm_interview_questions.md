# Data Structures & Algorithms Interview Questions

Based on your experience with enterprise systems, concurrent data structures, and performance optimization.

---

## **Arrays & Strings**

### Basic Level
1. How do you find duplicates in an array? What is the time complexity?
2. Implement a function to reverse a string in-place.
3. How do you check if two strings are anagrams?
4. Find the first non-repeating character in a string.
5. Implement string tokenization (similar to parsing command-line arguments).

### Intermediate Level
6. Find the longest substring without repeating characters.
7. Implement a sliding window algorithm for finding maximum sum subarray.
8. How do you rotate an array by k positions efficiently?
9. Merge two sorted arrays in-place with O(1) extra space.
10. Find all pairs in an array that sum to a target value.

---

## **Linked Lists**

### Basic Level
11. How do you detect a cycle in a linked list?
12. Reverse a singly linked list.
13. Find the middle element of a linked list in one pass.
14. Merge two sorted linked lists.

### Intermediate Level
15. Remove the N-th node from the end of a linked list in one pass.
16. Detect the starting point of a cycle in a linked list.
17. How would you implement a thread-safe linked list?
18. Explain the difference between `std::list` and `std::forward_list`.

---

## **Stacks & Queues**

### Basic Level
19. Implement a stack using two queues.
20. Implement a queue using two stacks.
21. Check if parentheses are balanced using a stack.
22. Design a min-stack that supports push, pop, top, and getMin in O(1).

### Intermediate Level
23. Implement a circular queue (ring buffer). How did you use this in your buffer management?
24. Design a thread-safe queue for Producer-Consumer pattern.
25. What is `std::deque`? When would you use it over `std::vector`?
26. Implement a monotonic stack. What problems does it solve?

### Applied to Your Experience
27. How did you implement the job queue in your thread pool using `std::list`?
28. What data structure would you use for managing backup job priorities?

---

## **Trees & Graphs**

### Basic Level
29. Implement in-order, pre-order, and post-order tree traversal (both recursive and iterative).
30. Find the height/depth of a binary tree.
31. Check if a binary tree is balanced.
32. Find the lowest common ancestor (LCA) of two nodes.

### Intermediate Level
33. Implement level-order traversal (BFS) of a binary tree.
34. Serialize and deserialize a binary tree.
35. Convert a sorted array to a balanced BST.
36. Validate if a binary tree is a valid BST.

### Graphs
37. Implement BFS and DFS for graph traversal.
38. Detect a cycle in a directed graph (useful for dependency detection).
39. Topological sort - how would you use it for dependency resolution in installations?
40. Find the shortest path using Dijkstra's algorithm.
41. How would you represent a dependency graph for plugin installation order?

---

## **Hash Tables**

### Basic Level
42. Explain how a hash table works. What is a collision?
43. Implement a simple hash map from scratch.
44. What is the difference between `std::unordered_map` and `std::map`?

### Intermediate Level
45. How do you handle hash collisions? (Chaining vs. Open Addressing)
46. What makes a good hash function?
47. Implement LRU Cache using hash map and doubly linked list.
48. How would you implement a thread-safe hash map?

### Applied to Your Experience
49. How would you use hash tables for credential caching with concurrent access?
50. Design a cache for frequently accessed backup metadata.

---

## **Heaps & Priority Queues**

### Basic Level
51. Explain the difference between min-heap and max-heap.
52. Implement heap operations: insert, extract-min/max, heapify.
53. What is `std::priority_queue`? How does it work?

### Intermediate Level
54. Find the K largest elements in an array.
55. Merge K sorted lists using a heap.
56. How would you use a priority queue for job scheduling with different priorities?
57. Implement a thread-safe priority queue for job dispatching.

---

## **Sorting & Searching**

### Basic Level
58. Explain QuickSort. What is its average and worst-case complexity?
59. Explain MergeSort. Why is it preferred for linked lists?
60. Implement binary search (iterative and recursive).
61. When would you use counting sort or radix sort?

### Intermediate Level
62. How do you find the K-th smallest element in an unsorted array?
63. Search in a rotated sorted array.
64. Find the peak element in an array.
65. Implement a custom comparator for sorting complex objects.

---

## **Concurrent Data Structures**

### Based on Your Threading Experience
66. How do you implement a lock-free queue?
67. What is a concurrent hash map? How does it achieve thread safety?
68. Explain the difference between fine-grained and coarse-grained locking.
69. How would you implement a thread-safe object pool?
70. What is a read-write lock? How does it improve concurrent read performance?

### Applied Questions
71. Design a thread-safe job queue with priority support.
72. How did you ensure thread safety for the credential file access?
73. What data structures would you use for managing multiple backup streams?

---

## **Dynamic Programming**

### Basic Level
74. Explain dynamic programming. What is memoization vs. tabulation?
75. Solve the Fibonacci sequence using DP.
76. Find the longest common subsequence (LCS) of two strings.
77. Coin change problem - find minimum coins needed.

### Intermediate Level
78. Longest increasing subsequence (LIS).
79. Edit distance (Levenshtein distance) - useful for diff algorithms.
80. 0/1 Knapsack problem.
81. Maximum subarray sum (Kadane's algorithm).

---

## **Algorithm Design Techniques**

82. Explain divide and conquer with examples.
83. Explain greedy algorithms. When do they work optimally?
84. What is backtracking? Give an example problem.
85. Explain the two-pointer technique.

---

## **Complexity Analysis**

86. What is Big O notation? Explain O(1), O(log n), O(n), O(n log n), O(n²).
87. What is amortized time complexity? Give an example.
88. How do you analyze space complexity?
89. What is the time complexity of operations on `std::map` vs `std::unordered_map`?
90. How does complexity analysis guide your data structure choice?

---

## **System Design Related Algorithms**

### Based on Your Backup System Experience
91. How would you design an efficient data deduplication algorithm?
92. Explain consistent hashing - useful for distributed backup storage.
93. How would you implement a file diff algorithm?
94. Design an algorithm for scheduling backup jobs across multiple workers.
95. How would you implement a retry mechanism with exponential backoff?

---

## **Practical Problem Solving**

96. Design a rate limiter using appropriate data structures.
97. Implement a simple memory allocator.
98. Design a log aggregation system - what data structures would you use?
99. How would you implement a file system path parser?
100. Design an algorithm for detecting circular dependencies in plugin installations.
