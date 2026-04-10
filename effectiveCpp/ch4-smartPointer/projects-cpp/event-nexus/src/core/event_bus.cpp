/// @file event_bus.cpp
/// @brief EventBus Pimpl implementation — the central pub/sub hub.
///
/// Combines EventDispatcher (subscriber management + dispatch) with
/// ThreadPool (async delivery). Demonstrates all five smart pointer items:
///
///   Item 18: unique_ptr for event ownership transfer (publish).
///   Item 19: shared_ptr for subscriber lifetime management.
///   Item 20: weak_ptr subscriber tracking (auto-cleanup).
///   Item 21: make_unique / make_shared exclusively (no raw new).
///   Item 22: Pimpl idiom — Impl defined here, special members = default.

#include <event_nexus/event_bus.h>
#include <event_nexus/event_dispatcher.h>
#include <event_nexus/thread_pool.h>

namespace nexus {

// ─── Impl ───────────────────────────────────────────────────────────
struct EventBus::Impl {
    EventDispatcher dispatcher;
    ThreadPool pool;

    explicit Impl(std::size_t workerThreads)
        : dispatcher(), pool(workerThreads) {}
};

// ─── Constructor ────────────────────────────────────────────────────
EventBus::EventBus(std::size_t workerThreads)
    : impl_(std::make_unique<Impl>(workerThreads)) {}

// ─── Item 22: special members ───────────────────────────────────────
EventBus::~EventBus() = default;
EventBus::EventBus(EventBus&&) noexcept = default;
EventBus& EventBus::operator=(EventBus&&) noexcept = default;

// ─── Subscribe ──────────────────────────────────────────────────────
void EventBus::subscribe(std::shared_ptr<ISubscriber> subscriber) {
    impl_->dispatcher.subscribe(std::move(subscriber));
}

// ─── Publish ────────────────────────────────────────────────────────
void EventBus::publish(std::unique_ptr<EventBase> event, DeliveryMode mode) {
    switch (mode) {
        case DeliveryMode::Sync:
            publishSyncImpl(std::move(event));
            break;
        case DeliveryMode::Async:
            publishAsyncImpl(std::move(event));
            break;
    }
}

void EventBus::publishSyncImpl(std::unique_ptr<EventBase> event) {
    // Dispatch directly on the calling thread.
    impl_->dispatcher.dispatch(*event);
    // event is destroyed here — unique_ptr (Item 18).
}

void EventBus::publishAsyncImpl(std::unique_ptr<EventBase> event) {
    // Move the unique_ptr into a shared_ptr so the lambda can be copyable
    // (std::function requires copyable callables).
    // This is the unique_ptr → shared_ptr conversion from Item 18/19.
    auto sharedEvent = std::shared_ptr<EventBase>(std::move(event));

    impl_->pool.submit([this, sharedEvent = std::move(sharedEvent)]() {
        impl_->dispatcher.dispatch(*sharedEvent);
    });
}

// ─── Diagnostics ────────────────────────────────────────────────────
std::size_t EventBus::subscriberCount() const noexcept {
    return impl_->dispatcher.subscriberCount();
}

std::size_t EventBus::activeSubscriberCount() const noexcept {
    return impl_->dispatcher.activeSubscriberCount();
}

std::size_t EventBus::purgeExpired() {
    return impl_->dispatcher.purgeExpired();
}

void EventBus::shutdown() {
    impl_->pool.shutdown();
}

}  // namespace nexus
