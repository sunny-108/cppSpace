# ADR-002: Pimpl Idiom on All Public Classes

## Status
Accepted

## Context
event-nexus is designed as a library. Public headers are included by user code.
Implementation changes (adding members, changing data structures) should not require
users to recompile.

Additionally, implementation headers like `<mutex>`, `<thread>`, `<unordered_map>`,
and `<condition_variable>` are heavy — including them in public headers slows
compilation.

## Decision
Apply the Pimpl idiom (Item 22) to all three public classes:
- `EventBus` → `unique_ptr<EventBus::Impl>`
- `EventDispatcher` → `unique_ptr<EventDispatcher::Impl>`
- `ThreadPool` → `unique_ptr<ThreadPool::Impl>`

Each class:
1. Forward-declares `struct Impl` in the header.
2. Declares destructor, move constructor, and move assignment in the header.
3. Defines them as `= default` in the `.cpp` where `Impl` is complete.
4. Defines `Impl` fully in the `.cpp`.

## Consequences

**Positive:**
- **ABI stability:** `sizeof(Widget)` = 8 bytes regardless of `Impl` contents.
- **Compile-time firewall:** Heavy headers (`<mutex>`, etc.) stay in `.cpp` files.
- **Encapsulation:** Users cannot depend on implementation details.

**Negative:**
- One heap allocation per object (for `Impl`).
- One pointer indirection per method call.
- Five special members must be explicitly declared (destructor + move pair + optionally copy pair).

**Mitigations:**
- The heap allocation cost is amortized — these objects are created once and used
  throughout the application lifetime.
- The pointer indirection is negligible compared to the mutex operations inside.

## Alternatives Considered

| Alternative | Rejected because |
|---|---|
| No Pimpl (direct members) | ABI-breaking on internal changes; slow compile |
| `shared_ptr<Impl>` Pimpl | Shared mutation semantics — breaks value semantics (see Item 22 Q24) |
| Abstract interface (virtual) | Requires heap allocation anyway; vtable dispatch overhead is comparable |
