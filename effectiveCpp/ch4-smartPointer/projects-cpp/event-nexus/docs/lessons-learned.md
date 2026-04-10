# Lessons Learned

Key takeaways from building event-nexus — a project designed to exercise every
smart pointer pattern from *Effective Modern C++* Chapter 4.

---

## 1. weak_ptr is the Natural Choice for Observer Lists

In any pub/sub system, the bus must reference subscribers without owning them.
Raw pointers leave dangling references; `shared_ptr` prevents subscriber destruction.
`weak_ptr` is the only correct answer — `lock()` atomically detects if the subscriber
is still alive.

**Gotcha:** Always call subscriber callbacks *outside* the dispatcher's mutex.
If a callback publishes another event, it re-enters the dispatcher — holding the
mutex would cause a deadlock.

## 2. Pimpl Eliminates Header Dependencies

Before Pimpl, the `EventDispatcher` header had to include `<mutex>`,
`<unordered_map>`, `<vector>`, and `<typeindex>`. After Pimpl, the header includes
nothing but `<memory>` and the event/subscriber headers. Client compile times dropped
because those heavy standard library headers are now in the `.cpp` only.

**Lesson:** Pimpl is not just about ABI stability — it's a compile-time firewall.

## 3. The Destructor-Move Suppression Trap (Item 22)

Declaring `~EventBus();` in the header suppresses implicit move operations.
Forgetting to declare move constructor and move assignment made `EventBus`
non-movable — which only showed up as a confusing error message at usage sites,
not at the class definition.

**Rule:** When using Pimpl with `unique_ptr`, always declare all five:
destructor + move ctor + move assign (+ optionally copy ctor + copy assign).

## 4. unique_ptr → shared_ptr for Async Capture

For synchronous dispatch, events flow as `unique_ptr` — clean single ownership.
But for async dispatch, the event must be captured in a lambda stored in
`std::function`, which requires copyability. The solution is the Item 18 → 19
conversion: `shared_ptr<EventBase>(std::move(uniqueEvent))`.

**Lesson:** `unique_ptr` is the right default; convert to `shared_ptr` only at
the boundary where shared ownership is actually needed (async dispatch, caches).

## 5. shared_ptr<const T> Prevents Mutation Races

Making event payloads `const` at the type level (`shared_ptr<const T>`) is a
compile-time guarantee that no subscriber can mutate the shared data. This
eliminates an entire class of concurrency bugs without runtime checks.

## 6. make_shared vs. make_unique — The Right Default

Every allocation in this project uses `make_unique` or `make_shared`. The only
exception would be objects with private constructors (passkey idiom), which
didn't arise here.

**Zero raw `new` in the entire codebase** — this is achievable and worth enforcing.

## 7. Stress Tests Catch What Unit Tests Miss

The concurrent subscribe-while-publishing stress test revealed that dispatching
subscribers while simultaneously modifying the subscriber list required careful
lock scoping. The fix: copy live `shared_ptr`s under the lock, then call
callbacks outside the lock.

**Lesson:** Always run stress tests under ThreadSanitizer during development.
