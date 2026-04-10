# Software Architecture & Design for AI Infrastructure

For a Senior/Staff AI Infrastructure Engineer, the "System Design" interview is significantly different from a standard web backend interview (e.g., "Design Twitter" or "Design Uber"). 

Instead of focusing heavily on REST APIs, load balancers, and database sharding, **AI Infrastructure design focuses on hardware utilization, memory bandwidth, distributed computing, and low-latency serving.**

Here is a comprehensive guide on how to prepare for Software Architecture and Design in the AI space.

---

## 1. Core Concepts You Must Master

To pass an AI System Design interview, you need to deeply understand the bottlenecks of Machine Learning workloads.

### A. The "Memory Wall" (The Biggest Bottleneck)
*   **Concept:** Compute (TFLOPS) has scaled much faster than memory bandwidth (GB/s). Most LLM inference is **memory-bandwidth bound**, not compute-bound.
*   **What to know:** How to calculate the memory required to store model weights (e.g., a 70B parameter model in FP16 takes ~140GB of VRAM). How to calculate the memory required for the KV Cache during generation.

### B. Distributed Inference & Training Strategies
When a model (or its activations) doesn't fit on a single GPU, you must distribute it.
*   **Data Parallelism (DP / DDP):** Replicate the model across GPUs, split the data batch. (Used mostly in training).
*   **Tensor Parallelism (TP):** Split individual weight matrices (tensors) across multiple GPUs. Requires high-bandwidth interconnects (NVLink) because GPUs must communicate (All-Reduce) during every matrix multiplication.
*   **Pipeline Parallelism (PP):** Split the layers of the model across GPUs (e.g., GPU 1 gets layers 1-10, GPU 2 gets 11-20). Introduces "bubbles" (idle time) that must be managed.

### C. Advanced LLM Serving Techniques
This is the most common interview topic for AI Infra roles right now.
*   **KV Caching:** Storing the Key and Value vectors of past tokens so you don't recompute them.
*   **Continuous Batching (Iteration-Level Scheduling):** Instead of waiting for all requests in a batch to finish, dynamically insert new requests into the batch as soon as one request finishes generating its sequence.
*   **PagedAttention (vLLM):** Managing the KV cache like an operating system manages virtual memory (using blocks and page tables) to eliminate memory fragmentation and allow sharing of prefixes (e.g., system prompts).

---

## 2. Common AI System Design Interview Questions

Expect questions that ask you to design the infrastructure to train or serve massive models.

### Question 1: "Design a high-throughput inference server for a 70B parameter LLM."
*   **The Challenge:** A 70B model in FP16 requires ~140GB of VRAM. An NVIDIA A100 has 80GB.
*   **Your Architecture:**
    1.  **Hardware:** You need at least 2x A100 GPUs.
    2.  **Parallelism:** Propose **Tensor Parallelism (TP=2)** to split the model across the two GPUs.
    3.  **Serving Engine:** Propose a C++ backend (like Triton or vLLM) using **Continuous Batching** to maximize throughput.
    4.  **Memory Management:** Explain how you will use **PagedAttention** to manage the KV cache efficiently across the GPUs.
    5.  **API Layer:** A lightweight gRPC or REST API (e.g., FastAPI or a C++ gRPC server) to handle incoming requests and queue them for the batcher.

### Question 2: "Design a system to train a 1 Trillion parameter model."
*   **The Challenge:** The model weights alone take ~2TB. Optimizer states (Adam) take another ~4-6TB. This requires thousands of GPUs.
*   **Your Architecture:**
    1.  **3D Parallelism:** You must combine Data Parallelism, Tensor Parallelism (within a single node/server using NVLink), and Pipeline Parallelism (across different nodes using InfiniBand).
    2.  **ZeRO (Zero Redundancy Optimizer):** Explain how you would partition the optimizer states and gradients across the data-parallel workers to save memory (DeepSpeed ZeRO Stage 1/2/3).
    3.  **Fault Tolerance:** GPUs fail frequently at this scale. Design a checkpointing system that asynchronously saves model weights to distributed storage (like AWS S3) without blocking training.

### Question 3: "Design a low-latency recommendation engine (e.g., for TikTok or YouTube)."
*   **The Challenge:** You need to score thousands of items for a user in <50ms.
*   **Your Architecture:**
    1.  **Two-Tower Model:** A retrieval phase (fast, lightweight model like ANN/FAISS to get the top 1000 candidates) followed by a ranking phase (heavy deep learning model to score the top 1000).
    2.  **Feature Store:** A low-latency key-value store (like Redis or a custom C++ in-memory database) to fetch real-time user features (e.g., "last 5 videos watched") during inference.
    3.  **Hardware Acceleration:** Using TensorRT to optimize the ranking model for sub-millisecond latency.

---

## 3. How to Prepare (Resources & Strategy)

### A. Read the Foundational Papers
You don't need to understand the deep math, but you must understand the *systems* architecture proposed in these papers:
1.  **vLLM (PagedAttention):** *Efficient Memory Management for Large Language Model Serving with PagedAttention*
2.  **Orca (Continuous Batching):** *Orca: A Distributed Serving System for Transformer-Based Generative Models*
3.  **Megatron-LM:** *Efficient Large-Scale Language Model Training on GPU Clusters* (For Tensor Parallelism).
4.  **ZeRO (DeepSpeed):** *ZeRO: Memory Optimizations Toward Training Trillion Parameter Models*

### B. Study Open Source Architectures
*   **Triton Inference Server (NVIDIA):** Read their documentation on how they handle dynamic batching, concurrent model execution, and shared memory.
*   **Ray (Anyscale):** Understand how Ray actors work for distributed ML orchestration.

### C. Practice the "Back-of-the-Envelope" Math
You must be able to do these calculations quickly during an interview:
*   **Model Size:** Parameters $\times$ Precision (e.g., 7B params $\times$ 2 bytes for FP16 = 14GB).
*   **KV Cache Size:** $2 \times \text{Layers} \times \text{Hidden Size} \times \text{Sequence Length} \times \text{Batch Size} \times \text{Precision}$.
*   **Communication Overhead:** Time to transfer data over PCIe Gen4 vs. NVLink vs. InfiniBand.

### D. Leverage Your Existing Strengths
During the interview, lean heavily into your 14+ years of C++ experience:
*   When discussing the serving engine, talk about **thread pools, lock-free queues for request batching, and custom memory allocators** (areas where you are already an expert).
*   When discussing fault tolerance, draw parallels to your experience building enterprise backup/restore systems at HPE.