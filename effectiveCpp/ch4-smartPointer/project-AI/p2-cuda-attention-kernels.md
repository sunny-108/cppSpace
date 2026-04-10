# P2 — CUDA Attention Kernel Library

> **Domain**: Deep Learning / GPU Programming | **Effort**: 3–4 weeks | **GPU required**: Yes (NVIDIA)

---

## Job Market Relevance

The **most lucrative single C++ skill in AI right now is writing fast CUDA kernels**.
The transformer attention mechanism is the beating heart of every LLM and vision transformer.

- **Anthropic, OpenAI, Google DeepMind** employ teams whose entire job is making attention faster
- **NVIDIA** hired the FlashAttention team because fast kernels are worth hundreds of millions in compute cost
- **Groq, Cerebras, SambaNova** build custom chips but still need kernel engineers bridging C++ ↔ hardware

**Target Roles**: GPU Kernel Engineer, MLSys Engineer, LLM Infra Engineer  
**Salary Signal**: $250K–$420K TC at FAANG+, $200K–$350K at AI labs

---

## What You'll Build

A CUDA C++ library implementing the core attention operations for transformers:

1. **Naive attention** — baseline for correctness (`QKᵀV` in three separate kernels)
2. **Fused softmax** — single-pass online softmax (numerically stable)
3. **FlashAttention-style tiled attention** — reduces HBM bandwidth via shared memory tiling
4. **Multi-head attention launcher** — batched across heads using CUDA streams
5. **Benchmarking harness** — compare against cuBLAS baseline

---

## Background: Why Attention is Hard to Optimize

```
Standard attention:  S = Q × Kᵀ  (N×N matrix — O(N²) memory!)
                     P = softmax(S / √d)
                     O = P × V

For N=4096, d=128: S alone is 4096×4096×4 bytes = 64 MB per layer per batch item.
FlashAttention insight: Tile Q,K,V in SRAM and compute output without materializing S.
HBM reads: O(N²) → O(N)  ... 5-10× faster in practice.
```

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                 attention_lib (CUDA C++)              │
├─────────────────────────────────────────────────────┤
│  attention.cuh          — kernel declarations        │
│  naive_attention.cu     — baseline, 3-kernel         │
│  fused_softmax.cu       — online softmax kernel      │
│  flash_attention.cu     — tiled FlashAttention       │
│  mha_launcher.cu        — multi-head, streams        │
│  bench.cu               — CUDA Events timing         │
└─────────────────────────────────────────────────────┘
        │                           │
    Host API                   Test Driver
  (C++ wrapper)             (GoogleTest + cuBLAS ref)
```

---

## Key CUDA Concepts Practiced

| Concept | What It Means | Why It Matters |
|---------|---------------|----------------|
| Shared memory tiling | Load QKV tiles into `__shared__` | Avoids slow HBM re-reads |
| Warp shuffle (`__shfl_xor_sync`) | Reduce max/sum within 32 threads | Online softmax without atomics |
| Memory coalescing | Ensure 128-byte aligned global loads | Full memory bandwidth utilization |
| CUDA streams | Overlap head computation | Multi-head parallelism |
| `cudaMallocAsync` + pool | Memory pool for Q/K/V buffers | Avoid per-call allocation overhead |
| `__launch_bounds__` | Guide compiler register allocation | Increase occupancy |

---

## Key C++ Concepts Practiced

| Concept | Effective C++ Item | Where Used |
|---------|-------------------|------------|
| `unique_ptr` + custom deleter | Item 18 | `CudaBuffer<T>` wrapping `cudaMalloc` |
| `shared_ptr` for stream handles | Item 19 | Shared `cudaStream_t` across heads |
| `make_unique` | Item 21 | Factory for `AttentionContext` |
| Pimpl hiding CUDA headers | Item 22 | Host `.hpp` never includes `cuda_runtime.h` |

---

## File Structure

```
p2-cuda-attention/
├── CMakeLists.txt
├── include/
│   ├── attention.hpp        # Host-side API (no CUDA headers leaked)
│   └── cuda_buffer.hpp      # RAII wrapper for device memory
├── src/
│   ├── naive_attention.cu
│   ├── fused_softmax.cu
│   ├── flash_attention.cu
│   ├── mha_launcher.cu
│   └── attention.cpp        # Pimpl impl
├── tests/
│   └── test_attention.cpp   # correctness vs cuBLAS reference
└── bench/
    └── bench_attention.cu   # CUDA Events timing
```

---

## Starter Code

### `include/cuda_buffer.hpp` — RAII Device Memory (Item 18)

```cpp
#pragma once
#include <cuda_runtime.h>
#include <memory>
#include <cstddef>
#include <stdexcept>

struct CudaDeleter {
    void operator()(void* ptr) const noexcept {
        cudaFree(ptr);          // safe to call on nullptr
    }
};

template<typename T>
class CudaBuffer {
public:
    explicit CudaBuffer(std::size_t count) : count_(count) {
        void* raw = nullptr;
        if (cudaMalloc(&raw, count * sizeof(T)) != cudaSuccess)
            throw std::runtime_error("cudaMalloc failed");
        ptr_ = std::unique_ptr<T, CudaDeleter>(static_cast<T*>(raw));
    }

    T* get() noexcept { return ptr_.get(); }
    const T* get() const noexcept { return ptr_.get(); }
    std::size_t count() const noexcept { return count_; }

    // Non-copyable, movable (unique ownership)
    CudaBuffer(const CudaBuffer&) = delete;
    CudaBuffer& operator=(const CudaBuffer&) = delete;
    CudaBuffer(CudaBuffer&&) noexcept = default;
    CudaBuffer& operator=(CudaBuffer&&) noexcept = default;

private:
    std::unique_ptr<T, CudaDeleter> ptr_;
    std::size_t count_;
};
```

### Fused Online Softmax Kernel

```cuda
// src/fused_softmax.cu
#include <cuda_fp16.h>

// Block = 1 row of the attention score matrix. Threads = min(N, 1024).
// Uses warp-level reduction to find max and sum without shared memory atomics.
__global__ void fused_softmax_kernel(
    float* __restrict__ scores,   // [B, H, N, N]
    int N,
    float scale)
{
    int row = blockIdx.x;   // one block per row
    float* row_ptr = scores + row * N;

    const int tid = threadIdx.x;
    const int stride = blockDim.x;

    // Pass 1: find max (online, warp shuffle reduction)
    float local_max = -1e20f;
    for (int i = tid; i < N; i += stride)
        local_max = fmaxf(local_max, row_ptr[i] * scale);

    // Warp reduction
    for (int offset = 16; offset > 0; offset >>= 1)
        local_max = fmaxf(local_max, __shfl_xor_sync(0xffffffff, local_max, offset));

    __shared__ float smax;
    if (tid == 0) smax = local_max;
    __syncthreads();

    // Pass 2: compute exp and sum
    float local_sum = 0.f;
    for (int i = tid; i < N; i += stride) {
        float val = __expf(row_ptr[i] * scale - smax);
        row_ptr[i] = val;
        local_sum += val;
    }

    for (int offset = 16; offset > 0; offset >>= 1)
        local_sum += __shfl_xor_sync(0xffffffff, local_sum, offset);

    __shared__ float ssum;
    if (tid == 0) ssum = local_sum;
    __syncthreads();

    // Pass 3: normalize
    for (int i = tid; i < N; i += stride)
        row_ptr[i] /= ssum;
}
```

### `include/attention.hpp` — Host API (Pimpl — Item 22)

```cpp
#pragma once
#include <memory>
#include <cstdint>

struct AttentionConfig {
    int batchSize;
    int numHeads;
    int seqLen;
    int headDim;
    float scale;        // typically 1/sqrt(headDim)
    bool useFlash;      // false = naive 3-kernel, true = tiled
};

class MultiHeadAttention {
public:
    explicit MultiHeadAttention(const AttentionConfig& cfg);
    ~MultiHeadAttention();

    // Q, K, V are device pointers [B, H, N, d] in row-major
    void forward(const float* Q, const float* K, const float* V,
                 float* output);

    float lastKernelMs() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;    // hides all CUDA headers from callers
};
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.24)
project(CudaAttention CUDA CXX)

set(CMAKE_CUDA_ARCHITECTURES 80 86 89 90)   # A100, RTX30/40, H100
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

find_package(CUDAToolkit REQUIRED)

add_library(attention STATIC
    src/naive_attention.cu
    src/fused_softmax.cu
    src/flash_attention.cu
    src/mha_launcher.cu
    src/attention.cpp
)
target_include_directories(attention PUBLIC include)
target_link_libraries(attention PUBLIC CUDA::cudart CUDA::cublas)
target_compile_options(attention PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math -O3>)

# Tests
include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/main.zip)
FetchContent_MakeAvailable(googletest)
add_executable(test_attention tests/test_attention.cpp)
target_link_libraries(test_attention attention GTest::gtest_main)
```

---

## Benchmarking Template

```cpp
// bench/bench_attention.cu
#include "attention.hpp"
#include <cuda_runtime.h>
#include <cstdio>

int main() {
    AttentionConfig cfg{1, 12, 1024, 64, 1.f/8.f, false};
    MultiHeadAttention mha(cfg);

    // ... allocate Q, K, V on device ...

    // Warm-up
    for (int i = 0; i < 5; ++i) mha.forward(dQ, dK, dV, dOut);
    cudaDeviceSynchronize();

    // Timed runs
    constexpr int RUNS = 100;
    for (int i = 0; i < RUNS; ++i) mha.forward(dQ, dK, dV, dOut);
    cudaDeviceSynchronize();

    printf("Naive  avg: %.3f ms\n", mha.lastKernelMs());

    cfg.useFlash = true;
    MultiHeadAttention mha_flash(cfg);
    for (int i = 0; i < 5; ++i) mha_flash.forward(dQ, dK, dV, dOut);
    for (int i = 0; i < RUNS; ++i) mha_flash.forward(dQ, dK, dV, dOut);
    cudaDeviceSynchronize();
    printf("Flash  avg: %.3f ms\n", mha_flash.lastKernelMs());
}
```

---

## Stretch Goals

| Goal | Skill Gained |
|------|-------------|
| BF16 / FP16 inputs using `__half` | Mixed-precision training workflows |
| Causal mask (decoder attention) | LLM autoregressive generation |
| Grouped Query Attention (GQA) | LLaMA 3, Mistral architecture support |
| Persistent kernel across layers | Reducing kernel launch overhead |
| Triton port for comparison | Python-side kernel engineering in AI labs |

---

## Key Papers to Read

| Paper | Why |
|-------|-----|
| FlashAttention (Dao et al., 2022) | Foundation of this project |
| FlashAttention-2 (Dao, 2023) | Work partitioning across warps |
| FlashAttention-3 (Shah et al., 2024) | H100 Tensor Cores + async copies |
| Online normalizer calculation for softmax (Milakov & Gimelshein) | The math behind online softmax |

---

## Cloud GPU Options (No local GPU needed)

```
Lambda Labs   — A10 (24 GB)  ~$0.60/hr    ← Best value for this project
RunPod        — RTX 4090     ~$0.44/hr
Vast.ai       — RTX 3090     ~$0.20/hr    ← Cheapest
Google Colab  — T4 (free)    limited       ← Good to start
```
