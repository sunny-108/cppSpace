#pragma once

/// @file event_dispatcher.h
/// @brief Routes events to matching subscribers by type.
///
/// Smart pointer usage:
///   - weak_ptr<ISubscriber> for subscriber references (Item 20).
///   - Automatic cleanup of expired subscribers (dead weak_ptrs).
///   - Pimpl idiom (Item 22) hides internal data structures.

#include <event_nexus/event.h>
#include <event_nexus/subscriber.h>

#include <memory>

namespace nexus {

class EventDispatcher {
public:
    EventDispatcher();

    // ── Item 22: Pimpl special members ──
    ~EventDispatcher();
    EventDispatcher(EventDispatcher&&) noexcept;
    EventDispatcher& operator=(EventDispatcher&&) noexcept;
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    /// Register a subscriber. The dispatcher stores a weak_ptr —
    /// when the caller drops their shared_ptr, the subscriber auto-expires.
    void subscribe(std::shared_ptr<ISubscriber> subscriber);

    /// Dispatch an event to all matching (alive) subscribers.
    /// Returns the number of subscribers that received the event.
    std::size_t dispatch(const EventBase& event);

    /// Remove all expired weak_ptrs. Called automatically during dispatch,
    /// but can also be called manually if memory is a concern.
    std::size_t purgeExpired();

    /// Total number of registered subscribers (including expired).
    [[nodiscard]] std::size_t subscriberCount() const noexcept;

    /// Number of live (non-expired) subscribers.
    [[nodiscard]] std::size_t activeSubscriberCount() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace nexus
