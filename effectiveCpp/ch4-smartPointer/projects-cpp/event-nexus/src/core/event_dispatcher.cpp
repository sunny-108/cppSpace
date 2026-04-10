/// @file event_dispatcher.cpp
/// @brief EventDispatcher Pimpl implementation.
///
/// Demonstrates Item 20 (weak_ptr) in action:
///   - Subscribers are stored as weak_ptr<ISubscriber>.
///   - On dispatch, each weak_ptr is lock()'d — if expired, it's purged.
///   - No dangling references, no preventing subscriber destruction.

#include <event_nexus/event_dispatcher.h>

#include <algorithm>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace nexus {

// ─── Impl ───────────────────────────────────────────────────────────
struct EventDispatcher::Impl {
    // Map: event type → list of weak subscribers.
    // weak_ptr ensures the bus never keeps dead subscribers alive.
    using SubscriberList = std::vector<std::weak_ptr<ISubscriber>>;
    std::unordered_map<std::type_index, SubscriberList> registry;
    mutable std::mutex mutex;

    /// Purge expired entries from a single subscriber list.
    static std::size_t purgeList(SubscriberList& list) {
        auto before = list.size();
        list.erase(
            std::remove_if(list.begin(), list.end(),
                           [](const std::weak_ptr<ISubscriber>& wp) { return wp.expired(); }),
            list.end());
        return before - list.size();
    }
};

// ─── Constructor ────────────────────────────────────────────────────
EventDispatcher::EventDispatcher()
    : impl_(std::make_unique<Impl>()) {}

// ─── Item 22: special members ───────────────────────────────────────
EventDispatcher::~EventDispatcher() = default;
EventDispatcher::EventDispatcher(EventDispatcher&&) noexcept = default;
EventDispatcher& EventDispatcher::operator=(EventDispatcher&&) noexcept = default;

// ─── subscribe ──────────────────────────────────────────────────────
void EventDispatcher::subscribe(std::shared_ptr<ISubscriber> subscriber) {
    if (!subscriber) return;

    auto eventType = subscriber->eventType();

    std::lock_guard lock(impl_->mutex);
    // Store weak_ptr — the bus does NOT own the subscriber (Item 20).
    impl_->registry[eventType].emplace_back(subscriber);
}

// ─── dispatch ───────────────────────────────────────────────────────
std::size_t EventDispatcher::dispatch(const EventBase& event) {
    std::size_t delivered = 0;
    auto eventType = event.type();

    // Build a list of live subscribers under the lock, then call them outside the lock
    // to avoid holding the mutex during potentially slow subscriber callbacks.
    std::vector<std::shared_ptr<ISubscriber>> liveSubscribers;

    {
        std::lock_guard lock(impl_->mutex);
        auto it = impl_->registry.find(eventType);
        if (it == impl_->registry.end()) {
            return 0;
        }

        auto& weakList = it->second;
        liveSubscribers.reserve(weakList.size());

        for (auto& wp : weakList) {
            // Item 20: lock() atomically checks and acquires shared_ptr.
            if (auto sp = wp.lock()) {
                liveSubscribers.push_back(std::move(sp));
            }
        }

        // Opportunistic cleanup of expired weak_ptrs.
        Impl::purgeList(weakList);
    }

    // Deliver outside the lock — subscribers may take arbitrary time.
    for (auto& sub : liveSubscribers) {
        sub->onEvent(event);
        ++delivered;
    }

    return delivered;
}

// ─── purgeExpired ───────────────────────────────────────────────────
std::size_t EventDispatcher::purgeExpired() {
    std::size_t purged = 0;
    std::lock_guard lock(impl_->mutex);
    for (auto& [type, list] : impl_->registry) {
        purged += Impl::purgeList(list);
    }
    return purged;
}

// ─── Diagnostics ────────────────────────────────────────────────────
std::size_t EventDispatcher::subscriberCount() const noexcept {
    std::lock_guard lock(impl_->mutex);
    std::size_t count = 0;
    for (const auto& [type, list] : impl_->registry) {
        count += list.size();
    }
    return count;
}

std::size_t EventDispatcher::activeSubscriberCount() const noexcept {
    std::lock_guard lock(impl_->mutex);
    std::size_t count = 0;
    for (const auto& [type, list] : impl_->registry) {
        for (const auto& wp : list) {
            if (!wp.expired()) ++count;
        }
    }
    return count;
}

}  // namespace nexus
