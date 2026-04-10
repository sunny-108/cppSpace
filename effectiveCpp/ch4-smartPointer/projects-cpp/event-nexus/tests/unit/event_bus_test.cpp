/// @file event_bus_test.cpp
/// @brief Unit tests for EventBus — subscribe, publish, subscriber lifecycle.

#include <event_nexus/event_bus.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

using namespace nexus;

// ─── Test payloads ──────────────────────────────────────────────────
struct TestPayload {
    int value;
    std::string label;
};

struct AnotherPayload {
    double x;
    double y;
};

// ─── Fixture ────────────────────────────────────────────────────────
class EventBusTest : public ::testing::Test {
protected:
    EventBus bus{2};  // 2 worker threads for async tests
};

// ─── Basic pub/sub ──────────────────────────────────────────────────

TEST_F(EventBusTest, SyncPublishDelivers) {
    int received = 0;
    auto sub = bus.subscribe<TestPayload>([&received](const TestPayload& p) {
        received = p.value;
    });

    bus.publishSync<TestPayload>(42, "hello");
    EXPECT_EQ(received, 42);
}

TEST_F(EventBusTest, MultipleSubscribersReceiveSameEvent) {
    std::atomic<int> count{0};

    auto sub1 = bus.subscribe<TestPayload>([&count](const TestPayload&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });
    auto sub2 = bus.subscribe<TestPayload>([&count](const TestPayload&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    bus.publishSync<TestPayload>(1, "test");
    EXPECT_EQ(count.load(), 2);
}

TEST_F(EventBusTest, DifferentEventTypesAreIsolated) {
    int testCount = 0;
    int anotherCount = 0;

    auto sub1 = bus.subscribe<TestPayload>([&testCount](const TestPayload&) {
        ++testCount;
    });
    auto sub2 = bus.subscribe<AnotherPayload>([&anotherCount](const AnotherPayload&) {
        ++anotherCount;
    });

    bus.publishSync<TestPayload>(1, "a");
    EXPECT_EQ(testCount, 1);
    EXPECT_EQ(anotherCount, 0);

    bus.publishSync<AnotherPayload>(3.14, 2.71);
    EXPECT_EQ(testCount, 1);
    EXPECT_EQ(anotherCount, 1);
}

// ─── Async delivery ─────────────────────────────────────────────────

TEST_F(EventBusTest, AsyncPublishDelivers) {
    std::atomic<int> received{0};

    auto sub = bus.subscribe<TestPayload>([&received](const TestPayload& p) {
        received.store(p.value, std::memory_order_release);
    });

    bus.publishAsync<TestPayload>(99, "async");

    // Wait for async delivery.
    for (int i = 0; i < 100; ++i) {
        if (received.load(std::memory_order_acquire) == 99) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(received.load(), 99);
}

// ─── Subscriber lifecycle (weak_ptr) ────────────────────────────────

TEST_F(EventBusTest, DroppedSubscriberStopsReceiving) {
    std::atomic<int> count{0};

    auto sub = bus.subscribe<TestPayload>([&count](const TestPayload&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    bus.publishSync<TestPayload>(1, "before");
    EXPECT_EQ(count.load(), 1);

    // Drop the subscriber — weak_ptr in bus expires.
    sub.reset();

    bus.publishSync<TestPayload>(2, "after");
    EXPECT_EQ(count.load(), 1);  // no additional delivery
}

TEST_F(EventBusTest, SubscriberCountTracking) {
    EXPECT_EQ(bus.subscriberCount(), 0u);

    auto sub1 = bus.subscribe<TestPayload>([](const TestPayload&) {});
    EXPECT_EQ(bus.subscriberCount(), 1u);
    EXPECT_EQ(bus.activeSubscriberCount(), 1u);

    auto sub2 = bus.subscribe<TestPayload>([](const TestPayload&) {});
    EXPECT_EQ(bus.subscriberCount(), 2u);

    sub1.reset();  // drop one subscriber
    EXPECT_EQ(bus.activeSubscriberCount(), 1u);
}

TEST_F(EventBusTest, PurgeRemovesExpiredSubscribers) {
    auto sub1 = bus.subscribe<TestPayload>([](const TestPayload&) {});
    auto sub2 = bus.subscribe<TestPayload>([](const TestPayload&) {});

    sub1.reset();
    sub2.reset();

    auto purged = bus.purgeExpired();
    EXPECT_EQ(purged, 2u);
    EXPECT_EQ(bus.subscriberCount(), 0u);
}

TEST_F(EventBusTest, ScopedSubscriberAutoExpires) {
    std::atomic<int> count{0};

    {
        auto scopedSub = bus.subscribe<TestPayload>([&count](const TestPayload&) {
            count.fetch_add(1, std::memory_order_relaxed);
        });

        bus.publishSync<TestPayload>(1, "in scope");
        EXPECT_EQ(count.load(), 1);
    }
    // scopedSub destroyed here.

    bus.publishSync<TestPayload>(2, "out of scope");
    EXPECT_EQ(count.load(), 1);  // no delivery
}

// ─── Event metadata ─────────────────────────────────────────────────

TEST_F(EventBusTest, EventHasUniqueId) {
    auto e1 = makeEvent<TestPayload>(1, "a");
    auto e2 = makeEvent<TestPayload>(2, "b");
    EXPECT_NE(e1->id(), e2->id());
}

TEST_F(EventBusTest, EventCarriesCorrectType) {
    auto event = makeEvent<TestPayload>(10, "test");
    EXPECT_EQ(event->type(), std::type_index(typeid(TestPayload)));
}

// ─── publish raw event ──────────────────────────────────────────────

TEST_F(EventBusTest, PublishPrebuiltEvent) {
    int received = 0;
    auto sub = bus.subscribe<TestPayload>([&received](const TestPayload& p) {
        received = p.value;
    });

    auto event = makeEvent<TestPayload>(77, "raw");
    bus.publish(std::move(event), DeliveryMode::Sync);
    EXPECT_EQ(received, 77);
}

// ─── Shared payload (zero-copy multicast) ───────────────────────────

TEST_F(EventBusTest, PayloadIsSharedAcrossSubscribers) {
    const TestPayload* addr1 = nullptr;
    const TestPayload* addr2 = nullptr;

    auto sub1 = bus.subscribe<TestPayload>([&addr1](const TestPayload& p) {
        addr1 = &p;
    });
    auto sub2 = bus.subscribe<TestPayload>([&addr2](const TestPayload& p) {
        addr2 = &p;
    });

    bus.publishSync<TestPayload>(1, "shared");

    // Both subscribers receive a reference to the SAME payload object
    // (zero-copy via shared_ptr<const T>).
    EXPECT_NE(addr1, nullptr);
    EXPECT_EQ(addr1, addr2);
}

// ─── Move semantics ─────────────────────────────────────────────────

TEST_F(EventBusTest, MoveConstructionWorks) {
    int received = 0;
    auto sub = bus.subscribe<TestPayload>([&received](const TestPayload& p) {
        received = p.value;
    });

    EventBus bus2 = std::move(bus);
    bus2.publishSync<TestPayload>(55, "moved");
    EXPECT_EQ(received, 55);
}

// ─── Shutdown ───────────────────────────────────────────────────────

TEST_F(EventBusTest, ShutdownIsSafe) {
    auto sub = bus.subscribe<TestPayload>([](const TestPayload&) {});
    bus.publishSync<TestPayload>(1, "before shutdown");
    EXPECT_NO_THROW(bus.shutdown());
}
