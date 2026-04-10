# Phase 3: Advanced Projects & Open Source (Months 7-12)

**Goal:** Transition from learning to building production-grade AI infrastructure. Focus on Large Language Models (LLMs), distributed systems, and contributing to major open-source AI projects to build a world-class portfolio.

---

## Month 7: LLM Inference Architecture (The "llama.cpp" Deep Dive)

### Learning Objectives
*   Understand the architecture of Transformer models (Attention, Feed Forward, LayerNorm).
*   Learn how LLM inference works under the hood (Prefill phase vs. Decode phase).
*   Understand KV Caching and why memory bandwidth is the primary bottleneck for LLMs.
*   Study the source code of `llama.cpp` (a pure C/C++ implementation of LLaMA inference).

### Learning Resources
*   **Blog Post:** [LLM Inference Performance Engineering (Databricks)](https://www.databricks.com/blog/llm-inference-performance-engineering-best-practices)
*   **Video:** [Let's build GPT: from scratch, in code, spelled out (Andrej Karpathy)](https://www.youtube.com/watch?v=kCc8FmEb1nY)
*   **GitHub Repo:** [ggerganov/llama.cpp](https://github.com/ggerganov/llama.cpp) (Spend significant time reading the core tensor operations in `ggml.c`).

### Project 7: Mini C++ Transformer Inference Engine
**Task:** Build a simplified, CPU-only inference engine for a small Transformer model (like GPT-2) from scratch in C++.
1.  **Weight Loading:** Write a C++ parser to load pre-trained GPT-2 weights from a `.bin` or `.safetensors` file.
2.  **Tensor Math:** Implement basic matrix multiplication and softmax in C++ (no external libraries like Eigen or BLAS initially, to understand the math).
3.  **KV Cache:** Implement a basic Key-Value cache to store past attention states during token generation.
4.  **Text Generation:** Implement a simple greedy decoding loop to generate text token by token.
5.  **Optimization (Your Expertise):** Profile your engine using Valgrind/Callgrind. Optimize the matrix multiplications using multi-threading (`std::thread` or OpenMP) and SIMD instructions (AVX2/AVX-512).

---

## Month 8: Advanced LLM Serving & Continuous Batching

### Learning Objectives
*   Understand the challenges of serving LLMs to multiple concurrent users.
*   Learn about Continuous Batching (Orca) and PagedAttention (vLLM).
*   Understand how to manage GPU memory fragmentation during text generation.

### Learning Resources
*   **Paper:** [vLLM: Easy, Fast, and Cheap LLM Serving with PagedAttention](https://arxiv.org/abs/2309.06180)
*   **Blog Post:** [How continuous batching enables 23x throughput in LLM inference (Anyscale)](https://www.anyscale.com/blog/continuous-batching-llm-inference)
*   **GitHub Repo:** [vllm-project/vllm](https://github.com/vllm-project/vllm) (Look at the C++/CUDA custom ops for PagedAttention).

### Project 8: Implementing PagedAttention (Conceptual or Simplified)
**Task:** Implement a simplified version of PagedAttention in C++/CUDA to manage the KV cache efficiently.
1.  **Memory Pool:** Design a C++ memory manager (similar to your Object Pool pattern experience) that allocates fixed-size "blocks" of GPU memory for the KV cache, rather than contiguous chunks per request.
2.  **Block Table:** Implement a mapping table that translates logical token positions to physical memory blocks.
3.  **Custom CUDA Kernel:** Write a custom CUDA kernel that performs the attention calculation by reading keys and values from these non-contiguous memory blocks.
4.  **Integration:** Integrate this custom kernel into your mini inference engine from Month 7 (or a small PyTorch script via pybind11).

---

## Month 9: Distributed Training & Inference (Multi-GPU)

### Learning Objectives
*   Understand how to scale models across multiple GPUs.
*   Learn the differences between Data Parallelism (DDP), Tensor Parallelism (TP), and Pipeline Parallelism (PP).
*   Learn NVIDIA NCCL (NVIDIA Collective Communications Library) for inter-GPU communication.

### Learning Resources
*   **Tutorial:** [PyTorch Distributed Overview](https://pytorch.org/tutorials/beginner/dist_overview.html)
*   **Documentation:** [NVIDIA NCCL Documentation](https://docs.nvidia.com/deeplearning/nccl/user-guide/docs/index.html)
*   **Paper:** [Megatron-LM: Training Multi-Billion Parameter Language Models Using Model Parallelism](https://arxiv.org/abs/1909.08053)

### Project 9: Multi-GPU Tensor Parallelism
**Task:** Implement a simple Tensor Parallel matrix multiplication across 2 GPUs using NCCL.
1.  **Setup:** Ensure you have access to a multi-GPU instance (e.g., AWS EC2 p3.8xlarge or a local machine with 2 GPUs).
2.  **NCCL Initialization:** Write a C++ program that initializes an NCCL communicator across the GPUs.
3.  **Split the Weights:** Take a large weight matrix and split it column-wise across the two GPUs.
4.  **Distributed Compute:** Perform the matrix multiplication on each GPU independently.
5.  **All-Reduce:** Use `ncclAllReduce` to sum the partial results from both GPUs and synchronize the final output.

---

## Months 10-12: Open Source Contributions & Interview Prep

### Learning Objectives
*   Build a public track record of high-quality C++/CUDA code in the AI space.
*   Prepare for rigorous systems engineering interviews at top AI companies.

### Action Plan: Open Source Contributions
Choose **one** major open-source project and become a regular contributor. Good targets for your C++ background:
*   **`pytorch/pytorch`:** Look for issues tagged "module: cpp", "module: memory_format", or "bootcamp".
*   **`microsoft/onnxruntime`:** Look for issues related to C++ API, memory leaks, or performance optimizations.
*   **`ggerganov/llama.cpp`:** Contribute to backend optimizations (e.g., improving the Vulkan or SYCL backends) or fixing memory management bugs.
*   **`huggingface/text-generation-inference` (TGI):** (If you learned Rust) Contribute to the router or custom CUDA kernels.

### Action Plan: Interview Preparation
1.  **Systems Design (AI Focus):** Practice designing large-scale ML systems.
    *   *Example Prompt:* "Design a system to serve a 70B parameter model to 100,000 DAU with <50ms time-to-first-token."
    *   *Focus Areas:* Load balancing, KV cache management, model sharding (Tensor Parallelism), and fault tolerance.
2.  **C++ & Concurrency Deep Dive:** Review advanced C++ concepts.
    *   *Topics:* `std::memory_order` (Acquire/Release semantics), lock-free queues, custom allocators, and RAII best practices.
3.  **CUDA Whiteboarding:** Be prepared to write CUDA kernels on a whiteboard or shared document.
    *   *Practice:* Matrix multiplication (tiled), reduction, prefix sum, and basic 1D convolution.
4.  **Resume Update:** Rewrite your resume to highlight the projects from Phases 1-3, emphasizing the performance gains (e.g., "Optimized custom PyTorch operator in CUDA, achieving 3x speedup over native implementation").