# C++ × AI/ML/GenAI Projects — Job Market Guide

> Bridging your **Effective C++ / smart pointer** study to **industry-grade AI systems**.

---

## Why C++ Still Dominates AI Infrastructure

Python trains models. **C++ runs them** — at scale, in real time, on every device from servers to
microcontrollers. In 2024-2026, essentially every major AI company maintains a critical C++ layer:

| Layer | Who Owns It | C++ Role |
|---|---|---|
| GPU kernels | NVIDIA, Google, Meta | CUDA C++, cuBLAS, cuDNN |
| Inference runtimes | NVIDIA (TensorRT), Microsoft (ONNX RT), Meta (llama.cpp) | Core runtime |
| Serving infra | Groq, Fireworks, Together AI | Token streaming, batching |
| Edge / mobile | Apple (CoreML), Qualcomm (SNPE), ARM | C++ compiled backends |
| Robotics / AV | Waymo, Tesla, Figure AI | ROS2, real-time C++ |
| HFT ML | Jane Street, Citadel, Two Sigma | Low-latency inference |

---

## 2024-2026 Job Market Snapshot

### Hottest Roles (C++ + AI)

| Role | Typical Stack | Salary Range (US) | Companies |
|---|---|---|---|
| **ML Inference Engineer** | CUDA, TensorRT, C++17 | $200K–$380K | NVIDIA, Meta, Apple, Microsoft |
| **MLSys / LLM Infra** | C++20, CUDA, triton | $220K–$400K | Anthropic, OpenAI, Groq, Cerebras |
| **Autonomous Systems Eng.** | ROS2, CUDA, C++17 | $180K–$320K | Waymo, Tesla, Zoox, Mobileye |
| **Robotics Software Eng.** | C++17, real-time, ML | $160K–$280K | Figure AI, Boston Dynamics, 1X |
| **GPU Kernel Engineer** | CUDA C++, PTX, SASS | $240K–$420K | NVIDIA, Google DeepMind |
| **Quant Developer ML** | C++17, Python, low-latency | $300K–$600K TC | Jane Street, Citadel, D.E. Shaw |

### Most Requested C++ + AI Skills (job postings, 2024)

```
CUDA programming            ████████████████████  42%
TensorRT / cuDNN            ███████████████       31%
ONNX Runtime                ████████████          28%
C++17 / C++20               ████████████████████  45%
Multi-threading / locks     ████████████████      36%
Memory optimization         █████████████         29%
Template metaprogramming    ████████              20%
SIMD (AVX2 / NEON)          ███████               18%
llama.cpp / vLLM internals  ██████                16%
ROS2                        ████████              20%
```

---

## Projects in This Folder

| # | Project | Domain | Key C++ Concepts | Est. Effort |
|---|---------|--------|-----------------|-------------|
| [P1](p1-mini-inference-engine.md) | Mini Neural Inference Engine | ML Inference | `unique_ptr`, templates, SIMD | 2–3 weeks |
| [P2](p2-cuda-attention-kernels.md) | CUDA Attention Kernel Library | Deep Learning / GPU | CUDA C++, warp ops, shared mem | 3–4 weeks |
| [P3](p3-llm-streaming-server.md) | LLM Token-Streaming Server | Generative AI | C++20 coroutines, `shared_ptr`, async | 3–4 weeks |
| [P4](p4-vector-database.md) | HNSW Vector Database | GenAI / RAG | Graph algorithms, `shared_ptr`, SIMD | 2–3 weeks |
| [P5](p5-realtime-cv-pipeline.md) | Real-Time CV + TensorRT Pipeline | Computer Vision / Robotics | Zero-copy buffers, `unique_ptr`, threads | 3–4 weeks |

---

## Connection to Effective C++ Chapter 4 (Smart Pointers)

Every project in this folder intentionally exercises the smart-pointer items you are studying:

| Item | Smart Pointer Topic | Where It Appears |
|------|---------------------|-----------------|
| Item 18 | `unique_ptr` for exclusive ownership | Model graph nodes, tensor buffers |
| Item 19 | `shared_ptr` for shared ownership | Engine context shared across threads |
| Item 20 | `weak_ptr` to break cycles | Graph edge references in HNSW |
| Item 21 | Prefer `make_unique` / `make_shared` | Factory methods for all allocations |
| Item 22 | `unique_ptr` in Pimpl idiom | Hiding TensorRT / ONNX implementation details |

---

## Recommended Learning Path

```
[Where you are now]
Effective C++ Items 18-22 (Smart Pointers)
        │
        ▼
P1: Mini Inference Engine   ← reinforce ownership + templates
        │
        ▼
P4: Vector Database          ← reinforce shared_ptr + graph structures
        │
        ├──────────────────────────────────────────────────┐
        ▼                                                  ▼
P3: LLM Streaming Server    ← C++20 coroutines      P5: CV Pipeline  ← ROS2 + real-time
        │                                                  │
        └──────────────────┬───────────────────────────────┘
                           ▼
               P2: CUDA Attention Kernels   ← requires GPU access (stretch)
```

---

## Tooling Prerequisites

```bash
# macOS (Apple Silicon — CPU projects work natively)
brew install cmake ninja llvm
brew install opencv          # P5
vcpkg install onnxruntime    # P1, P5
vcpkg install nlohmann-json  # P3

# GPU projects require NVIDIA GPU (cloud: Lambda Labs, RunPod, Vast.ai)
# CUDA 12.x + cuDNN 9.x + TensorRT 10.x
```
