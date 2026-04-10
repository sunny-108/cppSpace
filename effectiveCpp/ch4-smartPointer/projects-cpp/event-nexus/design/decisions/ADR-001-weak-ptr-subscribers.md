# ADR-001: Use weak_ptr for Subscriber Tracking

## Status
Accepted

## Context
The EventDispatcher needs to maintain a list of subscribers for each event type.
Subscribers are created by application code and may be destroyed at any time
(e.g., when a UI component closes, a module unloads, or a scope exits).

The dispatcher must NOT prevent subscribers from being destroyed (no zombie objects),
and must NOT have dangling pointers to destroyed subscribers (no use-after-free).

## Decision
Store `std::weak_ptr<ISubscriber>` in the dispatcher's registry. The subscriber's
owner holds a `shared_ptr<ISubscriber>` returned by `subscribe()`. Dropping the
`shared_ptr` causes the `weak_ptr` to expire.

On each dispatch:
1. Lock the mutex.
2. For each `weak_ptr` matching the event type, call `lock()`.
3. Collect live `shared_ptr`s.
4. Unlock the mutex.
5. Deliver events via the collected `shared_ptr`s.

## Consequences

**Positive:**
- No explicit unsubscribe API needed — drop the `shared_ptr`, stop receiving.
- No dangling references — `lock()` atomically detects expired subscribers.
- RAII-friendly — subscription lifetime is tied to a scope variable.

**Negative:**
- Expired `weak_ptr`s accumulate until purged (during dispatch or explicit `purgeExpired()`).
- `lock()` involves atomic operations on every dispatch — minor overhead.
- `shared_ptr` is required for subscribers — raw objects or `unique_ptr` subscribers are not supported.

## Alternatives Considered

| Alternative | Rejected because |
|---|---|
| Raw pointers | Dangling on subscriber destruction — UB |
| `shared_ptr` (bus owns) | Bus keeps dead subscribers alive — zombie processing |
| Explicit unsubscribe + ID | Requires subscriber to remember ID, call unsubscribe — error-prone |
| Signals/slots pattern | More complex; still needs weak_ptr or similar for lifetime |
