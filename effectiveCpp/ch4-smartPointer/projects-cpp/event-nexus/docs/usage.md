# API Usage Guide

## Core concepts

| Concept | Type | Lifetime |
|---|---|---|
| **Event** | `TypedEvent<T>` via `makeEvent<T>(...)` | Owned by bus during transit; shared across subscribers |
| **Subscriber** | `shared_ptr<ISubscriber>` via `bus.subscribe<T>(callback)` | You own the subscription — dropping it unsubscribes |
| **EventBus** | `nexus::EventBus` | Application lifetime |

## Subscribe to events

```cpp
#include <event_nexus/event_nexus.h>

struct ChatMessage {
    std::string user;
    std::string text;
};

nexus::EventBus bus(4);

// Type-safe subscription — returns shared_ptr (RAII handle)
auto sub = bus.subscribe<ChatMessage>([](const ChatMessage& msg) {
    std::cout << msg.user << ": " << msg.text << "\n";
});
```

## Publish events

```cpp
// Synchronous — blocks until all subscribers are notified
bus.publishSync<ChatMessage>("Alice", "Hello!");

// Asynchronous — returns immediately, delivered on thread pool
bus.publishAsync<ChatMessage>("Bob", "Hi there!");
```

## RAII unsubscription

```cpp
{
    auto sub = bus.subscribe<ChatMessage>([](const ChatMessage&) { /* ... */ });
    // sub is alive — receives events
}
// sub destroyed here — weak_ptr in bus auto-expires, no more deliveries
```

## Multiple subscribers for same event

```cpp
auto logger = bus.subscribe<ChatMessage>([](const ChatMessage& msg) {
    log(msg.text);
});

auto counter = bus.subscribe<ChatMessage>([&count](const ChatMessage&) {
    ++count;
});
// Both receive every ChatMessage
```

## Pre-built events

```cpp
auto event = nexus::makeEvent<ChatMessage>("System", "Server starting...");
bus.publish(std::move(event), nexus::DeliveryMode::Sync);
```

## Diagnostics

```cpp
std::cout << "Total subscribers: " << bus.subscriberCount() << "\n";
std::cout << "Active (alive): " << bus.activeSubscriberCount() << "\n";

auto purged = bus.purgeExpired();  // manually clean up dead weak_ptrs
```

## Shutdown

```cpp
bus.shutdown();  // finishes queued async tasks, then joins worker threads
```
