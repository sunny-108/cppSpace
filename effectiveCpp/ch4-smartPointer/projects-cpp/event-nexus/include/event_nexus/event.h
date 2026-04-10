#pragma once

/// @file event.h
/// @brief Type-safe event base and typed event wrapper.
///
/// Smart pointer usage:
///   - shared_ptr<const T> for event payloads (zero-copy multicast to subscribers)
///   - unique_ptr for type-erased event storage

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>

namespace nexus {

/// Unique numeric ID for each event instance.
using EventId = std::uint64_t;

/// Thread-safe monotonic event ID generator.
inline EventId nextEventId() noexcept {
    static std::atomic<EventId> counter{0};
    return counter.fetch_add(1, std::memory_order_relaxed);
}

// ─── EventBase ──────────────────────────────────────────────────────
/// Type-erased base class for all events.
/// Carries metadata; payload lives in the derived TypedEvent<T>.
class EventBase {
public:
    EventBase() : id_(nextEventId()), timestamp_(Clock::now()) {}
    virtual ~EventBase() = default;

    EventBase(const EventBase&) = delete;
    EventBase& operator=(const EventBase&) = delete;
    EventBase(EventBase&&) noexcept = default;
    EventBase& operator=(EventBase&&) noexcept = default;

    [[nodiscard]] EventId id() const noexcept { return id_; }

    [[nodiscard]] auto timestamp() const noexcept { return timestamp_; }

    /// Runtime type identifier for dispatching.
    [[nodiscard]] virtual std::type_index type() const noexcept = 0;

    /// Human-readable name (for logging / diagnostics).
    [[nodiscard]] virtual std::string typeName() const = 0;

private:
    using Clock = std::chrono::steady_clock;
    EventId id_;
    Clock::time_point timestamp_;
};

// ─── TypedEvent<T> ──────────────────────────────────────────────────
/// Concrete event carrying a shared, immutable payload of type T.
///
/// The payload is stored as shared_ptr<const T> so multiple subscribers
/// receive the same data without copying — zero-copy multicast.
template <typename T>
class TypedEvent final : public EventBase {
public:
    /// Construct from a shared payload (preferred — avoids extra copy).
    explicit TypedEvent(std::shared_ptr<const T> payload)
        : payload_(std::move(payload)) {}

    /// Convenience: construct payload in-place via make_shared.
    template <typename... Args>
    explicit TypedEvent(std::in_place_t, Args&&... args)
        : payload_(std::make_shared<const T>(std::forward<Args>(args)...)) {}

    [[nodiscard]] std::type_index type() const noexcept override {
        return std::type_index(typeid(T));
    }

    [[nodiscard]] std::string typeName() const override { return typeid(T).name(); }

    /// Access the immutable payload.
    [[nodiscard]] const T& payload() const noexcept { return *payload_; }

    /// Access the shared ownership of the payload.
    [[nodiscard]] std::shared_ptr<const T> sharedPayload() const noexcept { return payload_; }

private:
    std::shared_ptr<const T> payload_;
};

// ─── Factory helper ─────────────────────────────────────────────────
/// Creates a TypedEvent<T> wrapped in unique_ptr<EventBase>.
/// Uses make_unique (Item 21) — no raw new.
template <typename T, typename... Args>
std::unique_ptr<EventBase> makeEvent(Args&&... args) {
    auto payload = std::make_shared<const T>(std::forward<Args>(args)...);
    return std::make_unique<TypedEvent<T>>(std::move(payload));
}

}  // namespace nexus
