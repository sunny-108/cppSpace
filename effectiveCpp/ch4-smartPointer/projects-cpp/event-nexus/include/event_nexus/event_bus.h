#pragma once

/// @file event_bus.h
/// @brief Central pub/sub hub: combines EventDispatcher + ThreadPool.
///
/// Smart pointer usage:
///   - Pimpl idiom (Item 22) — unique_ptr<Impl>.
///   - shared_ptr for subscriber registration (Item 19).
///   - weak_ptr for non-owning subscriber tracking (Item 20).
///   - make_unique / make_shared throughout (Item 21).
///   - unique_ptr for event ownership transfer (Item 18).

#include <event_nexus/event.h>
#include <event_nexus/subscriber.h>

#include <cstddef>
#include <functional>
#include <memory>

namespace nexus {

/// Delivery mode for events.
enum class DeliveryMode {
    Sync,   ///< Deliver on the calling thread (blocking).
    Async,  ///< Deliver on a thread pool worker (non-blocking).
};

class EventBus {
public:
    /// Create an event bus with the given thread pool size.
    /// @param workerThreads Number of threads for async delivery (0 = hardware_concurrency).
    explicit EventBus(std::size_t workerThreads = 0);

    // ── Item 22: Pimpl special members ──
    ~EventBus();
    EventBus(EventBus&&) noexcept;
    EventBus& operator=(EventBus&&) noexcept;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // ── Subscribe ────────────────────────────────────────────────────

    /// Register a type-safe subscriber. Returns the subscriber for lifetime management.
    ///
    /// Usage:
    ///   auto sub = bus.subscribe<MyPayload>([](const MyPayload& p) { ... });
    ///   // sub keeps the subscription alive; dropping sub unsubscribes.
    template <typename T>
    std::shared_ptr<ISubscriber> subscribe(std::function<void(const T&)> callback);

    /// Register a pre-built subscriber.
    void subscribe(std::shared_ptr<ISubscriber> subscriber);

    // ── Publish ──────────────────────────────────────────────────────

    /// Publish an event synchronously (blocks until all subscribers are notified).
    template <typename T, typename... Args>
    void publishSync(Args&&... args);

    /// Publish an event asynchronously (returns immediately; delivery on worker threads).
    template <typename T, typename... Args>
    void publishAsync(Args&&... args);

    /// Publish a pre-built event.
    void publish(std::unique_ptr<EventBase> event, DeliveryMode mode = DeliveryMode::Async);

    // ── Diagnostics ──────────────────────────────────────────────────
    [[nodiscard]] std::size_t subscriberCount() const noexcept;
    [[nodiscard]] std::size_t activeSubscriberCount() const noexcept;
    std::size_t purgeExpired();
    void shutdown();

private:
    void publishSyncImpl(std::unique_ptr<EventBase> event);
    void publishAsyncImpl(std::unique_ptr<EventBase> event);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ─── Template implementations ───────────────────────────────────────

template <typename T>
std::shared_ptr<ISubscriber> EventBus::subscribe(std::function<void(const T&)> callback) {
    auto sub = makeSubscriber<T>(std::move(callback));
    subscribe(sub);
    return sub;
}

template <typename T, typename... Args>
void EventBus::publishSync(Args&&... args) {
    auto event = makeEvent<T>(std::forward<Args>(args)...);
    publishSyncImpl(std::move(event));
}

template <typename T, typename... Args>
void EventBus::publishAsync(Args&&... args) {
    auto event = makeEvent<T>(std::forward<Args>(args)...);
    publishAsyncImpl(std::move(event));
}

}  // namespace nexus
