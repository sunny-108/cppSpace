#pragma once

/// @file subscriber.h
/// @brief Subscriber interface and type-safe handler wrapper.
///
/// Smart pointer usage:
///   - Subscribers are managed via shared_ptr (callers) / weak_ptr (EventBus).
///   - enable_shared_from_this for safe self-referencing in async callbacks.

#include <event_nexus/event.h>

#include <functional>
#include <memory>
#include <typeindex>

namespace nexus {

// ─── ISubscriber ────────────────────────────────────────────────────
/// Type-erased subscriber interface.
/// The EventBus stores weak_ptr<ISubscriber> — subscribers can die at any time.
class ISubscriber : public std::enable_shared_from_this<ISubscriber> {
public:
    virtual ~ISubscriber() = default;

    /// Called by the bus to deliver an event.
    virtual void onEvent(const EventBase& event) = 0;

    /// Which event type this subscriber listens to.
    [[nodiscard]] virtual std::type_index eventType() const noexcept = 0;
};

// ─── TypedSubscriber<T> ─────────────────────────────────────────────
/// Concrete subscriber that invokes a std::function<void(const T&)> callback.
///
/// Uses shared_ptr + enable_shared_from_this:
///   - The EventBus holds weak_ptr<ISubscriber> (Item 20 — dangling detection).
///   - The owner holds shared_ptr<TypedSubscriber<T>> (Item 19 — shared ownership).
///   - When the owner drops its shared_ptr, the weak_ptr in the bus expires.
template <typename T>
class TypedSubscriber final : public ISubscriber {
public:
    using Callback = std::function<void(const T&)>;

    explicit TypedSubscriber(Callback cb) : callback_(std::move(cb)) {}

    void onEvent(const EventBase& event) override {
        // Safe downcast — dispatcher guarantees type match.
        const auto& typed = static_cast<const TypedEvent<T>&>(event);
        if (callback_) {
            callback_(typed.payload());
        }
    }

    [[nodiscard]] std::type_index eventType() const noexcept override {
        return std::type_index(typeid(T));
    }

private:
    Callback callback_;
};

// ─── Factory ────────────────────────────────────────────────────────
/// Creates a subscriber wrapped in shared_ptr (ready for bus registration).
/// Returns shared_ptr — caller keeps ownership; bus gets weak_ptr.
template <typename T>
std::shared_ptr<ISubscriber> makeSubscriber(std::function<void(const T&)> callback) {
    return std::make_shared<TypedSubscriber<T>>(std::move(callback));
}

}  // namespace nexus
