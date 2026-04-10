# Design — event-nexus

## Motivation

This project exists to demonstrate **production-grade** usage of all five smart pointer
items from *Effective Modern C++* (Chapter 4) in a single, cohesive system. A pub/sub
event bus is a natural fit because it inherently involves:

- **Exclusive ownership** (events in transit)
- **Shared ownership** (payloads delivered to multiple subscribers)
- **Non-owning observation** (subscriber tracking that must tolerate subscriber death)
- **Hidden implementation** (library-quality API with ABI stability)

---

## Core Design Decisions

### 1. weak_ptr for Subscriber Registry (Items 19 + 20)

**Problem:** The event bus needs to hold references to subscribers, but must NOT prevent
subscribers from being destroyed when their owners release them.

**Decision:** Store `weak_ptr<ISubscriber>` in the dispatcher. On each dispatch,
`lock()` atomically checks if the subscriber is still alive and acquires a temporary
`shared_ptr` for the duration of the callback.

**Consequences:**
- Subscribers are automatically cleaned up — no explicit unsubscribe API needed.
- The bus never holds dead subscribers alive (no zombie objects).
- Minor overhead: `lock()` involves atomic operations on each dispatch.

### 2. Pimpl Idiom on All Public Classes (Item 22)

**Problem:** Headers expose implementation details. Adding a member to `EventBus` would
break ABI — all users must recompile.

**Decision:** `EventBus`, `EventDispatcher`, and `ThreadPool` all use `unique_ptr<Impl>`.
Special members (destructor, move ctor, move assign) are declared in the header and
`= default`'d in the `.cpp` where `Impl` is complete.

**Consequences:**
- ABI stable: adding members to `Impl` only requires relinking, not recompiling clients.
- Compile times are reduced — heavy headers (`<mutex>`, `<thread>`, etc.) are in `.cpp` only.
- Small cost: one heap allocation per object, one pointer dereference per method call.

### 3. shared_ptr<const T> for Event Payloads (Item 19)

**Problem:** An event may be delivered to N subscribers. Copying the payload N times is
wasteful for large payloads (strings, vectors, ML tensors).

**Decision:** `TypedEvent<T>` stores `shared_ptr<const T>`. All subscribers receive a
reference to the same immutable object.

**Consequences:**
- Zero-copy multicast — O(1) memory per event regardless of subscriber count.
- `const T` prevents mutation races between concurrent subscribers.
- Payload lifetime is automatically extended until the last subscriber finishes.

### 4. unique_ptr for Event Ownership Transfer (Item 18)

**Problem:** Who owns an event between creation and delivery?

**Decision:** `publish()` takes `unique_ptr<EventBase>`. Ownership transfers from
publisher → bus → dispatcher. For async delivery, the `unique_ptr` is converted to
`shared_ptr` (Item 18→19 conversion) so the lambda can be copied into `std::function`.

**Consequences:**
- Clear ownership semantics — only one owner at a time for sync delivery.
- Conversion to `shared_ptr` for async is explicit and documented.
- Factory function `makeEvent<T>()` returns `unique_ptr` — most restrictive, easily convertible.

### 5. Thread Pool with Pimpl (Item 22) and unique_ptr Tasks (Item 18)

**Problem:** Tasks submitted to the thread pool need type-erased storage.

**Decision:** Tasks are stored as `unique_ptr<std::function<void()>>` in a
`std::queue`. The `unique_ptr` ensures single ownership — each task is executed once.

**Consequences:**
- No accidental double-execution.
- Move semantics throughout the queue — no copying of task objects.

---

## Thread Safety Model

| Component | Synchronization | Notes |
|---|---|---|
| `EventDispatcher` | `std::mutex` | Lock held only to copy weak_ptrs; callbacks called lock-free |
| `ThreadPool` | `std::mutex` + `condition_variable` | Standard producer-consumer pattern |
| `EventBus` | Delegates to dispatcher + pool | No additional synchronization needed |
| Event payloads | `shared_ptr<const T>` | Immutable — no data races by design |
| Subscriber lifecycle | `weak_ptr::lock()` | Atomic check-and-acquire |

**Key safety property:** Subscriber callbacks are invoked **outside** the dispatcher's
mutex. This prevents deadlocks if a callback publishes another event.

---

## Memory Layout

```
sizeof(EventBus)       = 8 bytes  (one unique_ptr)
sizeof(EventDispatcher) = 8 bytes  (one unique_ptr)
sizeof(ThreadPool)     = 8 bytes  (one unique_ptr)
sizeof(TypedEvent<T>)  = 32 bytes (EventBase fields + shared_ptr<const T>)
```

All public classes are pointer-sized thanks to Pimpl — ABI-optimal.
