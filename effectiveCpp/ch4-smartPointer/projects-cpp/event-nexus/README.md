# event-nexus

**Thread-safe async event bus with smart pointer ownership semantics.**

---

## What Is This Project?

**event-nexus** is a type-safe, thread-safe publish/subscribe event bus written in modern C++20.
It lets independent components in a program communicate through events without knowing about each
other — publishers fire events, subscribers react to them, and the bus handles routing, lifetime
management, and optional async delivery via a built-in thread pool.

## What Problems Does It Solve?

Building a multi-component C++ system (a game engine, a trading platform, a robotics stack) forces
you to answer several hard ownership and lifetime questions:

| Problem | How It Manifests | How event-nexus Solves It |
|---|---|---|
| **Tight coupling** | Component A calls Component B directly — every change in B breaks A. | The bus decouples them: A publishes an event, B subscribes. Neither knows the other exists. |
| **Dangling subscriber references** | A module registers a callback, then gets destroyed. The bus still holds a pointer to it → **use-after-free / crash**. | Subscribers are tracked via `weak_ptr`. When the owner drops its `shared_ptr`, the `weak_ptr` expires automatically — no dangling, no crash, no manual unsubscribe. |
| **Unclear ownership of shared data** | An event payload is delivered to 5 subscribers. Who owns it? Who deletes it? Copying it 5 times is wasteful. | Payloads are wrapped in `shared_ptr<const T>` — zero-copy multicast. The payload lives until the last subscriber finishes. `const` prevents mutation races. |
| **Exception-unsafe resource management** | Raw `new`/`delete` scattered across the codebase. An exception between `new` and `delete` leaks memory. | Every allocation uses `make_unique` or `make_shared`. There is **zero raw `new`** in the entire codebase — RAII guarantees cleanup even on exceptions. |
| **ABI fragility in library headers** | Adding a private member to a class changes `sizeof` — all downstream code must recompile. | All public classes (`EventBus`, `EventDispatcher`, `ThreadPool`) use the Pimpl idiom (`unique_ptr<Impl>`). Internal changes only require relinking, never recompiling client code. |
| **Blocking the main thread** | Delivering events to slow subscribers stalls the publisher. | Async mode dispatches events on a thread pool — the publisher returns immediately. |

### In short

> Raw pointers can't detect dead subscribers. `shared_ptr` keeps them alive as zombies.
> Only `weak_ptr` gives you non-owning observation with safe dangling detection — and that
> single insight drives the entire architecture of this project.

## Why This Project Exists

This is not just an event bus — it is a **deliberate exercise** in applying every smart pointer
pattern from *Effective Modern C++* (Scott Meyers, Chapter 4, Items 18–22) inside one cohesive,
real-world system:

- **Item 18** — `unique_ptr` for exclusive ownership → event ownership in transit, Pimpl `Impl`, task queue
- **Item 19** — `shared_ptr` for shared ownership → event payloads shared across N subscribers
- **Item 20** — `weak_ptr` for non-owning observation → subscriber registry with auto-expiration
- **Item 21** — Prefer `make_unique`/`make_shared` → zero raw `new` across the entire codebase
- **Item 22** — Pimpl idiom → ABI-stable public headers, compile-time firewall

The goal is to demonstrate that smart pointers are not just "safer `new`/`delete`" — they are a
**design vocabulary** that encodes ownership contracts directly in the type system.

---

## Features

| Feature | Description |
|---|---|
| Type-safe events | Events carry typed, immutable payloads via `shared_ptr<const T>` |
| Zero-copy multicast | Multiple subscribers receive the same payload without copying |
| Auto-cleanup subscribers | `weak_ptr`-based tracking — drop your `shared_ptr`, stop receiving |
| Sync + async delivery | Publish on the calling thread or dispatch to a thread pool |
| Pimpl all public classes | ABI-stable headers; implementation details hidden in `.cpp` files |
| Sanitizer-ready | First-class ASan, TSan, UBSan support via CMake flags |

---

## Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run demo
./build/event_nexus_demo
```

### Build options

| Option | Default | Description |
|---|---|---|
| `BUILD_TESTS` | `ON` | Build unit and stress tests (fetches Google Test) |
| `BUILD_BENCHMARKS` | `OFF` | Build Google Benchmark targets |
| `ENABLE_ASAN` | `OFF` | AddressSanitizer |
| `ENABLE_TSAN` | `OFF` | ThreadSanitizer |
| `ENABLE_UBSAN` | `OFF` | UndefinedBehaviorSanitizer |

### Run tests

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

### Run with ThreadSanitizer

```bash
cmake -B build-tsan -DENABLE_TSAN=ON
cmake --build build-tsan -j$(nproc)
cd build-tsan && ctest --output-on-failure
```

### Run benchmarks

```bash
cmake -B build-bench -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-bench -j$(nproc)
./build-bench/bench_event_bus --benchmark_format=json > benchmarks/results/baseline.json
```

---

## Quick Start

```cpp
#include <event_nexus/event_nexus.h>
#include <iostream>

struct PriceUpdate {
    std::string symbol;
    double price;
};

int main() {
    nexus::EventBus bus(4);  // 4 worker threads

    // Subscribe — returns shared_ptr (you own the subscription lifetime)
    auto sub = bus.subscribe<PriceUpdate>([](const PriceUpdate& p) {
        std::cout << p.symbol << " = $" << p.price << "\n";
    });

    // Publish synchronously
    bus.publishSync<PriceUpdate>("AAPL", 178.50);

    // Publish asynchronously (delivered on thread pool)
    bus.publishAsync<PriceUpdate>("GOOGL", 141.20);

    // Drop subscription — weak_ptr in bus auto-expires
    sub.reset();

    bus.shutdown();
}
```

---

## Architecture

```
┌───────────────────┐     unique_ptr<EventBase>     ┌───────────────────┐
│    Publisher       │ ────────────────────────────▶ │     EventBus      │
│  (user code)      │                                │  (Pimpl: Impl)   │
└───────────────────┘                                └────────┬──────────┘
                                                              │
                                        ┌─────────────────────┼─────────────────┐
                                        ▼                                       ▼
                              ┌─────────────────┐                    ┌─────────────────┐
                              │ EventDispatcher  │                    │   ThreadPool     │
                              │ (Pimpl: Impl)    │                    │  (Pimpl: Impl)   │
                              └────────┬─────────┘                    └──────────────────┘
                                       │
                      ┌────────────────┼──────────────────┐
                      ▼                ▼                   ▼
              weak_ptr<ISub>    weak_ptr<ISub>      weak_ptr<ISub>
                  │                   │                    │
                  ▼                   ▼                    ▼
           ┌────────────┐     ┌────────────┐       ┌────────────┐
           │ Subscriber │     │ Subscriber │       │  (expired)  │
           │ (alive)    │     │ (alive)    │       │  auto-purge │
           └────────────┘     └────────────┘       └────────────┘
```

### Ownership flow

1. **Event creation** → `makeEvent<T>(args...)` returns `unique_ptr<EventBase>` (Item 18)
2. **Event payload** → `shared_ptr<const T>` shared across all subscribers (Item 19)
3. **Subscriber tracking** → Bus stores `weak_ptr<ISubscriber>` (Item 20)
4. **Subscriber ownership** → User holds `shared_ptr<ISubscriber>` — dropping it unsubscribes
5. **Async delivery** → `unique_ptr` event converted to `shared_ptr` for capture in lambda (Item 18→19)

---

## Project Structure

```
event-nexus/
├── CMakeLists.txt
├── README.md
├── DESIGN.md
├── .clang-tidy
├── .clang-format
├── include/event_nexus/
│   ├── event.h               # EventBase, TypedEvent<T>, makeEvent()
│   ├── subscriber.h           # ISubscriber, TypedSubscriber<T>
│   ├── event_dispatcher.h     # Routes events to subscribers (Pimpl)
│   ├── event_bus.h            # Central hub: dispatcher + thread pool (Pimpl)
│   ├── thread_pool.h          # Fixed-size thread pool (Pimpl)
│   └── event_nexus.h          # Umbrella header
├── src/
│   ├── core/
│   │   ├── event_bus.cpp
│   │   ├── event_dispatcher.cpp
│   │   └── thread_pool.cpp
│   └── main.cpp               # Demo: stock trading event system
├── tests/
│   ├── unit/
│   │   ├── event_bus_test.cpp
│   │   ├── event_dispatcher_test.cpp
│   │   └── thread_pool_test.cpp
│   └── stress/
│       └── concurrent_pub_sub_test.cpp
├── benchmarks/
│   ├── bench_main.cpp
│   └── results/
├── docs/
│   ├── build.md
│   ├── usage.md
│   └── lessons-learned.md
├── design/
│   ├── architecture.md
│   └── decisions/
│       ├── ADR-001-weak-ptr-subscribers.md
│       └── ADR-002-pimpl-all-public.md
└── .github/
    └── workflows/
        └── ci.yml
```

---

## License

MIT
