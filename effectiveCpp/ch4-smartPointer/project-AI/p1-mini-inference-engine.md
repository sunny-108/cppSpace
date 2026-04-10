# P1 — Mini Neural Inference Engine in C++17

> **Domain**: ML Inference | **Effort**: 2–3 weeks | **GPU required**: No (CPU-only)

---

## Job Market Relevance

This project mirrors what every **ML Inference Engineer** role asks about in interviews:

- NVIDIA (TensorRT internals), Apple (CoreML C++ backend), Meta (ONNX Runtime contributions)
- Interview questions: "Design a graph execution engine", "How do you manage tensor lifetimes?"
- GitHub signal: A working inference engine in C++ is immediately impressive to recruiters

**Target Roles**: ML Inference Engineer, ML Systems Engineer, On-device ML Engineer

---

## What You'll Build

A minimal, dependency-light inference engine that:

1. Loads a computation graph (nodes + edges) from JSON
2. Allocates and manages tensors using `unique_ptr<float[]>`
3. Executes operators (ReLU, MatMul, Softmax, Conv2D) in topological order
4. Supports SIMD-accelerated matrix multiply (AVX2 on x86, NEON on Apple Silicon)
5. Runs a real MLP (multi-layer perceptron) for MNIST digit classification

---

## Architecture

```
                    ┌─────────────────────────────┐
                    │        InferenceEngine        │
                    │  (owns graph, owns tensors)   │
                    └──────────────┬──────────────┘
                                   │ unique_ptr
                    ┌──────────────▼──────────────┐
                    │         ComputeGraph          │
                    │   vector<unique_ptr<Node>>    │
                    └──────────────┬──────────────┘
                                   │
             ┌─────────────────────┼──────────────────┐
             ▼                     ▼                   ▼
      ┌─────────────┐     ┌─────────────┐    ┌─────────────┐
      │  MatMulNode  │     │  ReLUNode   │    │ SoftmaxNode │
      └─────────────┘     └─────────────┘    └─────────────┘
             │                     │                   │
             └─────────────────────┴──────────────────┘
                                   │ produce / consume
                    ┌──────────────▼──────────────┐
                    │         TensorPool            │
                    │  unordered_map<string,        │
                    │    unique_ptr<Tensor>>         │
                    └─────────────────────────────┘
```

---

## Key C++ Concepts Practiced

| Concept | Effective C++ Item | Where Used |
|---------|-------------------|------------|
| `unique_ptr` for exclusive ownership | Item 18 | `TensorPool`, `ComputeGraph` holds nodes |
| Factory via `make_unique` | Item 21 | `NodeFactory::create(opType)` |
| Pimpl idiom with `unique_ptr` | Item 22 | `InferenceEngine::Impl` hides SIMD details |
| `shared_ptr` for shared weights | Item 19 | Shared weight tensors across batched calls |
| Virtual dispatch + smart ptr | Item 20 | `Node` base class polymorphism |

---

## File Structure

```
p1-inference-engine/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── tensor.hpp          # Tensor with unique_ptr buffer
│   ├── node.hpp            # Abstract Node base class
│   ├── nodes/
│   │   ├── matmul.hpp
│   │   ├── relu.hpp
│   │   └── softmax.hpp
│   ├── graph.hpp           # ComputeGraph
│   └── engine.hpp          # InferenceEngine (Pimpl)
├── src/
│   ├── tensor.cpp
│   ├── nodes/matmul.cpp    # SIMD-accelerated
│   ├── nodes/relu.cpp
│   ├── nodes/softmax.cpp
│   ├── graph.cpp
│   └── engine.cpp
├── models/
│   └── mnist_mlp.json      # Pre-trained weights as JSON
├── tests/
│   └── test_engine.cpp     # GoogleTest
└── benchmark/
    └── bench_matmul.cpp    # Google Benchmark
```

---

## Starter Code

### `include/tensor.hpp`

```cpp
#pragma once
#include <memory>
#include <vector>
#include <cassert>
#include <span>          // C++20 (or use gsl::span in C++17)

class Tensor {
public:
    Tensor(std::vector<int> shape, float fill = 0.f)
        : shape_(std::move(shape))
        , size_(computeSize(shape_))
        , data_(std::make_unique<float[]>(size_))
    {
        std::fill(data_.get(), data_.get() + size_, fill);
    }

    // Non-copyable (Item 18: unique ownership of buffer)
    Tensor(const Tensor&) = delete;
    Tensor& operator=(const Tensor&) = delete;

    // Movable
    Tensor(Tensor&&) noexcept = default;
    Tensor& operator=(Tensor&&) noexcept = default;

    float* data() noexcept { return data_.get(); }
    const float* data() const noexcept { return data_.get(); }
    std::size_t size() const noexcept { return size_; }
    const std::vector<int>& shape() const noexcept { return shape_; }

private:
    static std::size_t computeSize(const std::vector<int>& shape) {
        std::size_t s = 1;
        for (int d : shape) s *= static_cast<std::size_t>(d);
        return s;
    }

    std::vector<int> shape_;
    std::size_t size_;
    std::unique_ptr<float[]> data_;   // ← Item 18 applied
};
```

### `include/node.hpp`

```cpp
#pragma once
#include "tensor.hpp"
#include <string>
#include <vector>

class Node {
public:
    virtual ~Node() = default;

    virtual void forward(
        const std::vector<const Tensor*>& inputs,
        std::vector<Tensor*>& outputs) = 0;

    virtual std::string opType() const = 0;

    std::string name;
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
};
```

### `include/engine.hpp` (Pimpl — Item 22)

```cpp
#pragma once
#include "tensor.hpp"
#include <memory>
#include <string>
#include <unordered_map>

class InferenceEngine {
public:
    explicit InferenceEngine(const std::string& modelPath);
    ~InferenceEngine();                // defined in engine.cpp (after Impl is complete)

    // Run inference; inputs/outputs are name-keyed tensors
    void run(
        const std::unordered_map<std::string, Tensor>& inputs,
        std::unordered_map<std::string, Tensor>& outputs);

private:
    struct Impl;                       // ← Pimpl: hides SIMD, graph internals
    std::unique_ptr<Impl> impl_;       // ← Item 22
};
```

---

## SIMD MatMul (Apple Silicon — NEON)

```cpp
// src/nodes/matmul.cpp  (simplified 2D × 2D)
#include "nodes/matmul.hpp"
#include <arm_neon.h>

void MatMulNode::forward(const std::vector<const Tensor*>& in,
                         std::vector<Tensor*>& out) {
    // Shape: A[M,K] × B[K,N] = C[M,N]
    const float* A = in[0]->data();
    const float* B = in[1]->data();
    float*       C = out[0]->data();

    int M = in[0]->shape()[0], K = in[0]->shape()[1], N = in[1]->shape()[1];

    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; n += 4) {
            float32x4_t acc = vdupq_n_f32(0.0f);
            for (int k = 0; k < K; ++k) {
                float32x4_t b = vld1q_f32(B + k * N + n);
                float32x4_t a = vdupq_n_f32(A[m * K + k]);
                acc = vmlaq_f32(acc, a, b);        // fused multiply-accumulate
            }
            vst1q_f32(C + m * N + n, acc);
        }
    }
}
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(InferenceEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apple Silicon NEON is always available; x86: add -mavx2
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    add_compile_options(-O3)
else()
    add_compile_options(-O3 -mavx2 -mfma)
endif()

find_package(nlohmann_json REQUIRED)     # model loading

add_library(engine STATIC
    src/tensor.cpp
    src/nodes/matmul.cpp
    src/nodes/relu.cpp
    src/nodes/softmax.cpp
    src/graph.cpp
    src/engine.cpp
)
target_include_directories(engine PUBLIC include)
target_link_libraries(engine PRIVATE nlohmann_json::nlohmann_json)

add_executable(run_mnist examples/run_mnist.cpp)
target_link_libraries(run_mnist PRIVATE engine)
```

---

## Stretch Goals

| Goal | Skill Gained |
|------|-------------|
| Load real ONNX protobuf models | protobuf parsing, real model compatibility |
| Add INT8 quantization support | quantization theory, bit manipulation |
| Thread pool for layer parallelism | `std::thread`, `std::atomic`, lock-free queues |
| Python bindings via pybind11 | cross-language FFI, ABI stability |
| CUDA backend for MatMul | CUBLAS, device memory management |

---

## Interview Topics This Covers

- "Walk me through how you'd design a tensor memory allocator"
- "How do you handle operator fusion in an inference graph?"
- "What are the cache implications of different MatMul loop orderings?"
- "How would you add a new operator to your engine?"
