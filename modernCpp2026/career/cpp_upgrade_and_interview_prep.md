# C++ Upgrade & Interview Preparation Guide for AI Infrastructure

## 1. Do I need to upgrade my C++ knowledge? What should be the focus area?

Yes, absolutely. While your C++14/17 experience is excellent, the AI infrastructure world (especially frameworks like PyTorch, ONNX Runtime, and vLLM) heavily utilizes modern C++ features for performance, safety, and compile-time optimizations.

### Focus Areas for C++ Upgrade (C++20/23)

1.  **Concepts (C++20):**
    *   **Why:** AI frameworks use heavy template metaprogramming (e.g., PyTorch's ATen library). Concepts make templates readable and provide much better compiler error messages when writing custom tensor operations.
    *   **Focus:** Learn how to constrain template parameters (e.g., ensuring a type is a floating-point number or a specific tensor shape).

2.  **Coroutines (C++20):**
    *   **Why:** AI inference servers (like Triton Inference Server) handle thousands of concurrent requests. Coroutines allow for highly efficient, asynchronous I/O (e.g., fetching weights from disk or network) without the overhead of OS threads.
    *   **Focus:** Understand `co_await`, `co_yield`, and `co_return`. Learn how to write a simple asynchronous task scheduler.

3.  **Ranges (C++20):**
    *   **Why:** Data preprocessing pipelines (e.g., tokenization, image resizing) require chaining multiple operations. Ranges allow you to compose transformations lazily and efficiently without intermediate memory allocations.
    *   **Focus:** Learn `std::views::transform`, `std::views::filter`, and how they differ from traditional `<algorithm>` functions.

4.  **Memory Management & Concurrency (Deep Dive):**
    *   **Why:** You already know this, but you need to master the absolute lowest levels.
    *   **Focus:** `std::memory_order` (Acquire/Release semantics for lock-free data structures), custom allocators (e.g., `std::pmr::polymorphic_allocator` for arena allocation, crucial for KV caching), and SIMD intrinsics (AVX-512).

---

## 2. Which companies can I target?

With your background in systems programming and a transition into AI Infra, you are targeting companies that build the "picks and shovels" of the AI gold rush.

### Tier 1: The AI Labs & Cloud Providers (Highest Comp, Hardest Interviews)
*   **OpenAI:** (Inference optimization, Triton, custom kernels).
*   **Anthropic:** (Scaling infrastructure, Claude serving).
*   **Google DeepMind / Google Brain:** (JAX internals, TPU optimization, XLA compiler).
*   **Meta (FAIR):** (PyTorch core team, Llama infrastructure).
*   **Microsoft:** (ONNX Runtime team, Azure AI infrastructure, DeepSpeed).
*   **NVIDIA:** (TensorRT, cuDNN, Megatron-LM, Triton Inference Server).

### Tier 2: AI Infrastructure Startups (High Growth, Equity Upside)
*   **Databricks (MosaicML):** (Training infrastructure, LLM serving).
*   **Anyscale:** (Creators of Ray, distributed ML orchestration).
*   **Together AI / Fireworks AI:** (High-performance LLM inference APIs).
*   **Hugging Face:** (Text-Generation-Inference (TGI), Candle (Rust), optimization).
*   **Cohere:** (Enterprise LLM serving and infrastructure).
*   **Groq / Cerebras:** (Custom AI hardware compilers and runtime).

### Tier 3: Tech Giants with Massive ML Workloads
*   **Tesla:** (Autopilot infrastructure, Dojo supercomputer).
*   **Uber / Airbnb / Netflix:** (Recommendation engine infrastructure, feature stores).
*   **Snowflake:** (Integrating ML directly into the data warehouse).

---

## 3. Is there any specific preparation for this?

Yes, AI Infrastructure interviews are a unique hybrid of Systems Engineering, High-Performance Computing (HPC), and Machine Learning.

### Specific Preparation Strategy:
1.  **The "CUDA Whiteboard":** You will be asked to write or optimize a CUDA kernel on a whiteboard or shared doc.
    *   *Practice:* Matrix Multiplication (tiled, shared memory), Reduction, Prefix Sum, 1D/2D Convolution.
2.  **Systems Design (AI Flavor):** Standard system design (like designing Twitter) is less common. Instead, expect:
    *   "Design a system to serve a 100B parameter model to 10,000 concurrent users." (Focus: KV Cache, PagedAttention, Continuous Batching, Tensor Parallelism).
    *   "Design a distributed training pipeline for a 1T parameter model." (Focus: Data vs. Tensor vs. Pipeline parallelism, NCCL, checkpointing).
3.  **C++ Deep Dive:** Expect brutal C++ trivia and architecture questions.
    *   "Implement a lock-free queue."
    *   "Explain false sharing and how to prevent it."
    *   "How does `std::shared_ptr` work under the hood? Is the control block thread-safe?"
4.  **ML Fundamentals:** You don't need to invent new architectures, but you must understand the math.
    *   Understand the exact matrix dimensions at every step of a Transformer block.
    *   Understand backpropagation (calculating gradients).

---

## 4. Are LeetCode problems still asked during interviews?

**Yes, unfortunately, LeetCode is still very much alive, even for Senior/Staff roles.** However, the *type* of LeetCode questions differs slightly for AI Infra roles.

### What to expect regarding LeetCode:
1.  **The "Screening" Round:** Almost all Tier 1 and Tier 2 companies will still use 1-2 standard LeetCode Medium/Hard questions to filter candidates in the first round.
2.  **Focus Areas for AI Infra:**
    *   **Arrays & Strings:** Heavy focus here (since tensors are just multi-dimensional arrays).
    *   **Graphs & Trees:** Important for understanding computation graphs (like PyTorch's Autograd or ONNX graphs). Topological sort is a very common requirement.
    *   **Dynamic Programming:** Less common, but still appears.
    *   **Concurrency Problems:** (e.g., LeetCode's "Print FooBar Alternately" or "Design a Blocking Queue"). Given your background, you must nail these.
3.  **The "Practical" Coding Round:** Instead of pure LeetCode, some companies (like Stripe or some AI startups) are shifting to practical coding:
    *   "Here is a buggy C++ thread pool. Fix the deadlocks and race conditions."
    *   "Implement a basic memory allocator (malloc/free) from scratch."
    *   "Write a C++ program to parse a binary file format and extract specific data."

### Recommendation:
Do not skip LeetCode. Spend 20-30% of your prep time grinding **LeetCode Mediums** (focusing on Arrays, Graphs, and Concurrency) in modern C++. You don't need to solve 500 problems, but you need to be fast and bug-free on the top 100 standard patterns.