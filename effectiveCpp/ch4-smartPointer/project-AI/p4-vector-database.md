# P4 — HNSW Vector Database in C++17

> **Domain**: Generative AI / RAG Systems | **Effort**: 2–3 weeks | **GPU required**: No

---

## Job Market Relevance

**Retrieval-Augmented Generation (RAG)** is the dominant pattern for grounding LLMs in private
data. The retrieval step is powered by **vector similarity search**, and the companies building
that infrastructure write it in C++:

- **Qdrant** (C++ + Rust core), **Milvus** (C++ engine), **Weaviate**, **Chroma**
- Every enterprise AI team building internal RAG systems needs this expertise
- **FAISS** (Meta's vector library) is C++ — contributing to it is a strong signal

**Target Roles**: Search Infrastructure Engineer, ML Platform Engineer, Applied AI Engineer  
**Companies**: Pinecone, Qdrant, Zilliz (Milvus), MongoDB Atlas Search, PostgreSQL pgvector contributors  
**Salary Signal**: $160K–$280K, more at Pinecone/Meta at senior levels

---

## What You'll Build

A header-heavy, dependency-free C++17 vector database library:

1. **HNSW index** — Hierarchical Navigable Small World graph for approximate nearest neighbor
2. **SIMD distance functions** — L2, cosine, dot-product with NEON/AVX2 acceleration
3. **Thread-safe insert + query** — `shared_mutex` for concurrent reads
4. **Persistence** — serialize/deserialize the graph to binary file
5. **Simple CLI** — `vdb add`, `vdb query`, `vdb build` commands

---

## Background: How HNSW Works

```
Layer 2 (sparse):  1 ──── 5
                    \    /
Layer 1 (medium):   2──3──4──6──7
                   / \     \ /
Layer 0 (dense):  8──9──10──11──12──13──14──15

Insert(new_vector):
  1. Enter at top layer, greedily navigate to ef nearest neighbors
  2. Descend to next layer, repeat
  3. At layer 0: connect to M nearest neighbors with heuristic edge selection

Query(q, k=5):
  1. Enter at top layer, greedy search to get entry point for layer below
  2. At layer 0: beam search with beam width ef_search, return top-k
  
Complexity: O(log N) average insert & query  (vs O(N) brute-force)
```

---

## Architecture

```
┌──────────────────────────────────────────────────────┐
│                   VectorIndex<dim>                    │
│            (template on embedding dimension)          │
├──────────────────────────────────────────────────────┤
│  insert(id, vector)  — thread-safe                   │
│  query(vector, k)    → vector<pair<float,uint64_t>>  │
│  save(path) / load(path)                             │
└─────────────────────────┬────────────────────────────┘
                           │
           ┌───────────────┼────────────────────┐
           ▼               ▼                    ▼
  ┌──────────────┐ ┌──────────────┐  ┌────────────────┐
  │  HnswGraph   │ │ DistanceFn   │  │  NodeStorage   │
  │  layers of   │ │ L2 / cosine  │  │ unordered_map  │
  │  adjacency   │ │ SIMD accel.  │  │ <id, Embedding>│
  │  lists       │ └──────────────┘  └────────────────┘
  └──────────────┘
```

---

## Key C++ Concepts Practiced

| Concept | Effective C++ Item | Where Used |
|---------|-------------------|------------|
| `shared_ptr` for node data | Item 19 | `HnswNode` shared between graph layers |
| `weak_ptr` to break graph cycles | Item 20 | Back-edges in HNSW adjacency lists |
| `make_shared` | Item 21 | All node construction |
| Template on dimension | — | `VectorIndex<768>` for BERT, `<1536>` for OpenAI |
| `shared_mutex` (C++17) | — | Concurrent read / exclusive write |
| Custom allocators | — | Pool allocator for graph nodes |

---

## File Structure

```
p4-vector-db/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── vector_index.hpp     # Main public API (template)
│   ├── hnsw_graph.hpp       # Core HNSW algorithm
│   ├── hnsw_node.hpp        # Node type (shared_ptr target)
│   ├── distance.hpp         # SIMD-accelerated distance functions
│   └── serializer.hpp       # Binary I/O
├── src/
│   ├── hnsw_graph.cpp
│   └── serializer.cpp
├── cli/
│   └── vdb_cli.cpp          # Command-line tool
└── tests/
    ├── test_distance.cpp
    ├── test_hnsw.cpp
    └── bench_query.cpp
```

---

## Starter Code

### `include/hnsw_node.hpp` — Graph Node (Items 19+20)

```cpp
#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <shared_mutex>

struct HnswNode {
    uint64_t id;
    std::vector<float> embedding;   // copy of the vector for distance calcs

    // Adjacency lists per layer.
    // Using shared_ptr here because nodes at layer L+1 reference the same
    // logical node objects as layer L. Using weak_ptr for back-edges
    // to break reference cycles (Item 20).
    struct LayerEdges {
        std::vector<std::weak_ptr<HnswNode>> neighbors;  // ← Item 20
        mutable std::shared_mutex mu;                     // protect per-node neighbor list
    };
    std::vector<LayerEdges> layers;

    HnswNode(uint64_t id_, std::vector<float> emb, int maxLayers)
        : id(id_), embedding(std::move(emb)), layers(maxLayers) {}
};
```

### `include/distance.hpp` — SIMD L2 Distance

```cpp
#pragma once
#include <cstddef>
#include <span>

#if defined(__ARM_NEON)
  #include <arm_neon.h>
  inline float l2_distance(const float* a, const float* b, std::size_t dim) noexcept {
      float32x4_t sum = vdupq_n_f32(0.f);
      std::size_t i = 0;
      for (; i + 4 <= dim; i += 4) {
          float32x4_t va = vld1q_f32(a + i);
          float32x4_t vb = vld1q_f32(b + i);
          float32x4_t diff = vsubq_f32(va, vb);
          sum = vmlaq_f32(sum, diff, diff);
      }
      // Horizontal sum
      float32x2_t s = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
      float result = vget_lane_f32(vpadd_f32(s, s), 0);
      for (; i < dim; ++i) { float d = a[i]-b[i]; result += d*d; }
      return result;
  }
#elif defined(__AVX2__)
  #include <immintrin.h>
  inline float l2_distance(const float* a, const float* b, std::size_t dim) noexcept {
      __m256 sum = _mm256_setzero_ps();
      std::size_t i = 0;
      for (; i + 8 <= dim; i += 8) {
          __m256 diff = _mm256_sub_ps(_mm256_loadu_ps(a+i), _mm256_loadu_ps(b+i));
          sum = _mm256_fmadd_ps(diff, diff, sum);
      }
      // Horizontal sum
      __m128 lo = _mm256_castps256_ps128(sum);
      __m128 hi = _mm256_extractf128_ps(sum, 1);
      lo = _mm_add_ps(lo, hi);
      lo = _mm_hadd_ps(lo, lo);
      float result = _mm_cvtss_f32(_mm_hadd_ps(lo, lo));
      for (; i < dim; ++i) { float d = a[i]-b[i]; result += d*d; }
      return result;
  }
#else
  inline float l2_distance(const float* a, const float* b, std::size_t dim) noexcept {
      float result = 0.f;
      for (std::size_t i = 0; i < dim; ++i) { float d = a[i]-b[i]; result += d*d; }
      return result;
  }
#endif
```

### `include/vector_index.hpp` — Public API

```cpp
#pragma once
#include "hnsw_graph.hpp"
#include <vector>
#include <utility>
#include <string>
#include <cstdint>

template<int Dim>
class VectorIndex {
public:
    struct Config {
        int M          = 16;     // max neighbors per layer
        int efConstruct = 200;   // beam width during insert
        int efSearch    = 50;    // beam width during query
        int seed        = 42;
    };

    explicit VectorIndex(Config cfg = {})
        : graph_(cfg.M, cfg.efConstruct, cfg.seed) {}

    void add(uint64_t id, const float (&vec)[Dim]) {
        graph_.insert(id, std::vector<float>(vec, vec + Dim));
    }

    // Returns (distance, id) pairs, sorted ascending by distance
    std::vector<std::pair<float, uint64_t>>
    query(const float (&vec)[Dim], int k, int efSearch = -1) const {
        return graph_.search(std::vector<float>(vec, vec+Dim), k,
                             efSearch < 0 ? cfg_.efSearch : efSearch);
    }

    void save(const std::string& path) const;
    static VectorIndex load(const std::string& path);

private:
    Config cfg_;
    HnswGraph graph_;   // defined in hnsw_graph.hpp
};
```

---

## Persistence Format

```
Magic: "VDBHNSW1"  (8 bytes)
Header:
  uint32  dim
  uint32  numNodes
  uint32  M
  uint32  maxLayers
Per-node:
  uint64  id
  float[] embedding  (dim floats)
  uint32  numLayers
  Per-layer:
    uint32  numNeighbors
    uint64[] neighborIds
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(VectorDB CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    add_compile_options(-O3)
else()
    add_compile_options(-O3 -mavx2 -mfma)
endif()

add_library(vectordb STATIC
    src/hnsw_graph.cpp
    src/serializer.cpp)
target_include_directories(vectordb PUBLIC include)

add_executable(vdb cli/vdb_cli.cpp)
target_link_libraries(vdb PRIVATE vectordb)

# Tests
include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/main.zip)
FetchContent_MakeAvailable(googletest)
add_executable(test_vdb tests/test_hnsw.cpp tests/test_distance.cpp)
target_link_libraries(test_vdb vectordb GTest::gtest_main)
```

---

## Recall Benchmarking

A good HNSW index on 1M vectors of dim=128 should achieve:

| Config | Recall@10 | QPS (single thread) |
|--------|-----------|---------------------|
| M=8,  ef=50  | ~92% | ~5 000 |
| M=16, ef=100 | ~97% | ~2 500 |
| M=32, ef=200 | ~99% | ~1 000 |

Use the ANN Benchmarks dataset (SIFT1M) to measure against these baselines.

---

## Stretch Goals

| Goal | Skill Gained |
|------|-------------|
| Product Quantization (PQ) compression | Reduce memory 8-32× with minimal recall loss |
| Concurrent insert + query correctness test | Advanced lock-free / MVCC techniques |
| IVF (Inverted File) index as alternative | Understand when HNSW vs IVF is better |
| Python bindings (pybind11) | Expose to LangChain, LlamaIndex |
| REST API for embeddings + search | Full RAG backend service |

---

## Interview Topics This Covers

- "What is approximate nearest neighbor search and when is exact search impractical?"
- "How does HNSW achieve O(log N) complexity?"
- "How do you prevent data races when multiple threads insert concurrently?"
- "How would you reduce memory usage for 100M vectors?"
