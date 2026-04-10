# P3 — LLM Token-Streaming Server in C++20

> **Domain**: Generative AI / LLM Infra | **Effort**: 3–4 weeks | **GPU required**: No (uses llama.cpp CPU mode)

---

## Job Market Relevance

Every LLM API (OpenAI, Anthropic, Groq, Together AI) is fundamentally an **HTTP server that
streams tokens**. The C++ layer underneath handles:
- Batch request scheduling (continuous batching)
- KV-cache management
- Token encoding/decoding
- Backpressure and flow control

**Target Roles**: LLM Infrastructure Engineer, Backend Systems Engineer, ML Serving Engineer  
**Companies**: Groq, Fireworks, Together AI, Mistral, any company self-hosting LLMs  
**Salary Signal**: $180K–$300K at AI infra companies; senior roles up to $380K TC

---

## What You'll Build

An HTTP server that:

1. Accepts `POST /v1/completions` requests with a JSON body
2. Loads a GGUF model via **llama.cpp** C API
3. Runs inference and streams tokens back via **Server-Sent Events (SSE)**
4. Handles concurrent requests using a C++20 **thread pool with coroutines**
5. Tracks active contexts with `shared_ptr` to safely share across threads
6. Provides `/health` and `/metrics` (tokens/sec, queue depth) endpoints

The API is compatible with OpenAI's completions format so you can point any OpenAI client at it.

---

## Architecture

```
Client (curl / Python openai SDK)
        │  HTTP POST /v1/completions
        ▼
┌─────────────────────────────────────────────────┐
│                  HTTP Server                     │
│          (Asio + Beast / cpp-httplib)            │
│                                                  │
│   RequestParser  ──→  RequestQueue (bounded)     │
│                              │                   │
│                  ┌───────────▼──────────┐        │
│                  │   InferenceScheduler  │        │
│                  │  (continuous batching)│        │
│                  └───────────┬──────────┘        │
│                              │ shared_ptr<Ctx>   │
│                  ┌───────────▼──────────┐        │
│                  │   LlamaEngine         │        │
│                  │  (wraps llama.cpp)    │        │
│                  └───────────┬──────────┘        │
│                              │ token callback    │
│                  ┌───────────▼──────────┐        │
│                  │   SSE Streamer        │        │
│                  │  streams to client   │        │
│                  └──────────────────────┘        │
└─────────────────────────────────────────────────┘
```

---

## Key C++ Concepts Practiced

| Concept | Effective C++ Item | Where Used |
|---------|-------------------|------------|
| `shared_ptr` for inference context | Item 19 | `InferenceContext` shared between scheduler + streamer |
| `weak_ptr` to observe without owning | Item 20 | Metrics collector observes active contexts |
| `make_shared` exclusively | Item 21 | All context allocation via factory |
| `unique_ptr` for RAII llama model | Item 18 | `LlamaModel` owns `llama_model*` raw handle |
| C++20 coroutines (`co_await`) | — | Async token generation loop |
| `std::atomic<bool>` | — | Cancellation flag per request |

---

## File Structure

```
p3-llm-server/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── llama_engine.hpp       # RAII wrapper around llama.cpp
│   ├── inference_context.hpp  # Per-request state (shared_ptr target)
│   ├── scheduler.hpp          # Continuous batching scheduler
│   ├── sse_streamer.hpp       # Server-Sent Events writer
│   └── server.hpp             # HTTP server glue
├── src/
│   ├── llama_engine.cpp
│   ├── inference_context.cpp
│   ├── scheduler.cpp
│   ├── sse_streamer.cpp
│   ├── server.cpp
│   └── main.cpp
├── models/                    # Put .gguf model files here (gitignored)
└── tests/
    └── test_tokenizer.cpp
```

---

## Starter Code

### `include/llama_engine.hpp` — RAII Wrapper (Item 18 + 22)

```cpp
#pragma once
#include <llama.h>       // llama.cpp C API
#include <memory>
#include <string>
#include <functional>
#include <span>

// Custom deleters for llama.cpp C handles (Item 18: unique_ptr + custom deleter)
struct LlamaModelDeleter {
    void operator()(llama_model* m) const noexcept { llama_free_model(m); }
};
struct LlamaContextDeleter {
    void operator()(llama_context* c) const noexcept { llama_free(c); }
};

using UniqueModel   = std::unique_ptr<llama_model,   LlamaModelDeleter>;
using UniqueContext = std::unique_ptr<llama_context, LlamaContextDeleter>;

class LlamaEngine {
public:
    struct Config {
        std::string modelPath;
        int nCtx      = 2048;
        int nThreads  = 4;
        int nGpuLayers = 0;     // >0 requires CUDA build of llama.cpp
    };

    explicit LlamaEngine(Config cfg);
    ~LlamaEngine();

    // Tokenize prompt; returns token ids
    std::vector<llama_token> tokenize(const std::string& text, bool addBos = true) const;

    // Generate tokens, calling `onToken` for each new token.
    // Returns false if cancelled via `cancel` flag.
    bool generate(
        std::span<const llama_token> promptTokens,
        int maxNewTokens,
        std::function<bool(llama_token, std::string_view)> onToken,
        const std::atomic<bool>& cancel);

    std::string detokenize(llama_token tok) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;   // hides llama.cpp internals (Item 22)
};
```

### `include/inference_context.hpp` — Shared State (Item 19)

```cpp
#pragma once
#include <string>
#include <atomic>
#include <chrono>
#include <memory>
#include <functional>

// Shared between HTTP handler, scheduler, and SSE streamer.
// Lifetime managed by shared_ptr (Item 19).
struct InferenceContext {
    std::string requestId;
    std::string prompt;
    int maxTokens   = 256;
    float temperature = 0.7f;

    // Output channel: scheduler calls this with each new piece of text
    std::function<void(std::string_view token, bool done)> onToken;

    // Cancellation (client disconnect or max-tokens reached)
    std::atomic<bool> cancelled{false};

    // Metrics
    std::atomic<int>  tokensGenerated{0};
    std::chrono::steady_clock::time_point startTime;

    InferenceContext() : startTime(std::chrono::steady_clock::now()) {}
};

using ContextPtr  = std::shared_ptr<InferenceContext>;
using WeakCtxPtr  = std::weak_ptr<InferenceContext>;
```

### Server-Sent Events Format

```cpp
// src/sse_streamer.cpp
// Each token arrives as:
//   data: {"id":"cmpl-xxx","choices":[{"delta":{"content":"Hello"},"finish_reason":null}]}
//
// Final message:
//   data: {"id":"cmpl-xxx","choices":[{"delta":{},"finish_reason":"stop"}]}
//   data: [DONE]

#include "sse_streamer.hpp"
#include <nlohmann/json.hpp>
#include <format>   // C++20

std::string makeSseChunk(const std::string& requestId,
                          std::string_view token,
                          bool done) {
    using json = nlohmann::json;
    json chunk = {
        {"id", requestId},
        {"object", "text_completion"},
        {"choices", {{
            {"delta", done ? json::object() : json{{"content", token}}},
            {"finish_reason", done ? "stop" : nullptr}
        }}}
    };
    if (done)
        return std::format("data: {}\n\ndata: [DONE]\n\n", chunk.dump());
    return std::format("data: {}\n\n", chunk.dump());
}
```

### C++20 Thread Pool (simplified)

```cpp
// include/thread_pool.hpp
#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lk(mu_);
                        cv_.wait(lk, [this]{ return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    void enqueue(F&& f) {
        { std::scoped_lock lk(mu_); tasks_.emplace(std::forward<F>(f)); }
        cv_.notify_one();
    }

    ~ThreadPool() {
        { std::scoped_lock lk(mu_); stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mu_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
};
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(LlmServer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# llama.cpp (build as submodule)
add_subdirectory(third_party/llama.cpp EXCLUDE_FROM_ALL)

find_package(nlohmann_json REQUIRED)

# cpp-httplib (header-only)
include_directories(third_party/cpp-httplib)

add_executable(llm_server
    src/llama_engine.cpp
    src/inference_context.cpp
    src/scheduler.cpp
    src/sse_streamer.cpp
    src/server.cpp
    src/main.cpp
)
target_include_directories(llm_server PRIVATE include)
target_link_libraries(llm_server PRIVATE llama nlohmann_json::nlohmann_json)
```

---

## Example Usage

```bash
# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# Download a model (Llama 3.2 3B Q4_K_M, ~2 GB)
huggingface-cli download bartowski/Llama-3.2-3B-Instruct-GGUF \
    --include "Llama-3.2-3B-Instruct-Q4_K_M.gguf" --local-dir models/

# Run server
./build/llm_server --model models/Llama-3.2-3B-Instruct-Q4_K_M.gguf --port 8080

# Stream completions (OpenAI-compatible)
curl http://localhost:8080/v1/completions \
     -H "Content-Type: application/json" \
     -d '{"prompt": "The C++ smart pointer", "max_tokens": 100, "stream": true}'
```

---

## Stretch Goals

| Goal | Skill Gained |
|------|-------------|
| Continuous batching (multiple concurrent users) | Production LLM serving architecture |
| KV-cache sharing across requests | Memory management at scale |
| Speculative decoding | Draft model + verifier pattern |
| OpenAI Chat Completions API (`/v1/chat/completions`) | Full API compatibility |
| gRPC endpoint with Protobuf | Alternative transport for microservices |
| Prometheus `/metrics` with token/sec | Observability in production systems |

---

## Interview Topics This Covers

- "How does token streaming work on the server side?"
- "How would you implement request batching for an LLM server?"
- "What happens when a client disconnects mid-stream?"
- "How do you prevent memory leaks when a context is shared across threads?"
