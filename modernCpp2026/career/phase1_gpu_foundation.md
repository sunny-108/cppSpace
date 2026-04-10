# Phase 1: The GPU Foundation (Months 1-3)

**Goal:** Master CUDA C++, understand GPU architecture, and learn how to bind high-performance C++ code to Python.

---

## Month 1: Introduction to GPU Architecture & Basic CUDA

### Learning Objectives
*   Understand the difference between CPU and GPU architecture (Latency vs. Throughput).
*   Learn the CUDA programming model: Grids, Blocks, Threads.
*   Write basic CUDA kernels (Vector Addition, Matrix Multiplication).
*   Understand GPU memory hierarchy: Global, Shared, Constant, and Local memory.

### Learning Resources
*   **Book:** *Programming Massively Parallel Processors: A Hands-on Approach* by David B. Kirk and Wen-mei W. Hwu. (The absolute bible for CUDA).
*   **Course (Free):** [NVIDIA Deep Learning Institute (DLI) - Fundamentals of Accelerated Computing with C/C++](https://courses.nvidia.com/courses/course-v1:DLI+C-AC-01+V1/)
*   **YouTube:** [CUDA Crash Course by CoffeeBeforeArch](https://www.youtube.com/playlist?list=PLxNPSjHT5qvtYRVdNN1yDcdSl39uHV_sU) (Excellent, practical C++ CUDA tutorials).

### Project 1: The "Hello World" of AI Infra (Vector & Matrix Ops)
**Task:** Implement standard linear algebra operations from scratch in CUDA C++.
1.  **Vector Addition:** Write a kernel to add two large arrays (100M+ elements). Compare the execution time against a standard `std::transform` CPU implementation.
2.  **Naive Matrix Multiplication:** Implement $C = A \times B$.
3.  **Tiled Matrix Multiplication:** Optimize the naive implementation using **Shared Memory** to reduce global memory bandwidth bottlenecks. This is a classic interview question.
4.  **Benchmarking:** Write a C++ benchmarking suite (using `std::chrono` or Google Benchmark) to measure the GFLOPS of your implementations.

---

## Month 2: Advanced CUDA & Memory Optimization

### Learning Objectives
*   Master memory coalescing and bank conflicts.
*   Understand warp divergence and how to avoid it.
*   Learn CUDA Streams and concurrency (overlapping compute and data transfer).
*   Use NVIDIA Nsight Systems/Compute for profiling.

### Learning Resources
*   **Course (Coursera):** [GPU Programming Specialization by Johns Hopkins University](https://www.coursera.org/specializations/gpu-programming) (Focus on the CUDA courses).
*   **Documentation:** [NVIDIA CUDA C++ Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html) (Read this cover to cover).
*   **YouTube:** [NVIDIA Developer - CUDA Optimization Techniques](https://www.youtube.com/user/NVIDIADeveloper)

### Project 2: High-Performance Reduction & Prefix Sum
**Task:** Implement parallel reduction (e.g., finding the sum or max of an array) and prefix sum (scan) algorithms.
1.  **Parallel Reduction:** Implement a tree-based reduction in CUDA. Optimize it by unrolling the last warp and minimizing thread divergence.
2.  **Prefix Sum (Scan):** Implement the Blelloch Scan algorithm. This is fundamental for many parallel algorithms (like sorting and stream compaction).
3.  **Profiling:** Use Nsight Compute to analyze your kernels. Identify memory bottlenecks and optimize your memory access patterns to achieve >80% memory bandwidth utilization.

---

## Month 3: Bridging C++ and Python (pybind11)

### Learning Objectives
*   Understand how Python interacts with C/C++ extensions.
*   Learn `pybind11` to expose C++ classes and functions to Python.
*   Pass NumPy arrays (which are just contiguous memory blocks) to C++ and CUDA without copying data.

### Learning Resources
*   **Documentation:** [pybind11 Official Documentation](https://pybind11.readthedocs.io/en/stable/)
*   **Tutorial:** [Python C++ bindings with pybind11 (Real Python)](https://realpython.com/python-bindings-overview/#pybind11)
*   **GitHub Repo:** [pybind/pybind11](https://github.com/pybind/pybind11) (Look at the examples folder).

### Project 3: Custom Python ML Library
**Task:** Package your CUDA kernels from Months 1 & 2 into a Python library.
1.  **C++ Wrapper:** Write a C++ class that manages GPU memory allocation (using RAII, which you already know well) and calls your CUDA kernels.
2.  **pybind11 Integration:** Use `pybind11` to expose this class to Python. Ensure you can pass a `numpy.ndarray` directly to your C++ code, get the raw pointer, and pass it to CUDA.
3.  **Python Testing:** Write a Python script that imports your custom library, generates random matrices using NumPy, calls your custom CUDA matrix multiplication, and verifies the result against `numpy.matmul`.
4.  **Packaging:** Create a `setup.py` file to compile and install your C++/CUDA extension as a standard Python package.