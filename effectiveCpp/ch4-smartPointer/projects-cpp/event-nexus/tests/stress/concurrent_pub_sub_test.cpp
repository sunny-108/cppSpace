/// @file concurrent_pub_sub_test.cpp
/// @brief Stress tests for concurrent publish/subscribe operations.
///
/// Run with ThreadSanitizer to verify thread safety:
///   cmake -DENABLE_TSAN=ON ..
///   make stress_tests && ./stress_tests

#include <event_nexus/event_bus.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace nexus;

struct StressPayload {
    int producerId;
    int sequenceNum;
};

// ─── Concurrent publishers, single subscriber ──────────────────────

TEST(ConcurrentPubSub, MultiplePublishersSingleSubscriber) {
    constexpr int kNumProducers = 8;
    constexpr int kEventsPerProducer = 500;
    constexpr int kTotalEvents = kNumProducers * kEventsPerProducer;

    EventBus bus(4);
    std::atomic<int> receivedCount{0};

    auto sub = bus.subscribe<StressPayload>([&receivedCount](const StressPayload&) {
        receivedCount.fetch_add(1, std::memory_order_relaxed);
    });

    std::vector<std::thread> producers;
    producers.reserve(kNumProducers);

    for (int p = 0; p < kNumProducers; ++p) {
        producers.emplace_back([&bus, p] {
            for (int i = 0; i < kEventsPerProducer; ++i) {
                bus.publishSync<StressPayload>(p, i);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }

    EXPECT_EQ(receivedCount.load(), kTotalEvents);
    bus.shutdown();
}

// ─── Concurrent publishers + async delivery ─────────────────────────

TEST(ConcurrentPubSub, AsyncMultipleProducers) {
    constexpr int kNumProducers = 4;
    constexpr int kEventsPerProducer = 200;
    constexpr int kTotalEvents = kNumProducers * kEventsPerProducer;

    EventBus bus(4);
    std::atomic<int> receivedCount{0};

    auto sub = bus.subscribe<StressPayload>([&receivedCount](const StressPayload&) {
        receivedCount.fetch_add(1, std::memory_order_relaxed);
    });

    std::vector<std::thread> producers;
    producers.reserve(kNumProducers);

    for (int p = 0; p < kNumProducers; ++p) {
        producers.emplace_back([&bus, p] {
            for (int i = 0; i < kEventsPerProducer; ++i) {
                bus.publishAsync<StressPayload>(p, i);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }

    // Wait for all async events to be delivered.
    bus.shutdown();

    EXPECT_EQ(receivedCount.load(), kTotalEvents);
}

// ─── Concurrent subscribe and publish ───────────────────────────────

TEST(ConcurrentPubSub, ConcurrentSubscribeAndPublish) {
    constexpr int kNumIterations = 200;

    EventBus bus(4);
    std::atomic<int> deliveryCount{0};
    std::atomic<bool> done{false};

    // Publisher thread: continuously publishes events.
    std::thread publisher([&bus, &done] {
        for (int i = 0; !done.load(std::memory_order_acquire); ++i) {
            bus.publishSync<StressPayload>(0, i);
            if (i > 5000) break;  // safety limit
        }
    });

    // Subscriber thread: continuously subscribes and drops.
    std::thread subscriber([&bus, &deliveryCount, &done] {
        for (int i = 0; i < kNumIterations; ++i) {
            auto sub = bus.subscribe<StressPayload>([&deliveryCount](const StressPayload&) {
                deliveryCount.fetch_add(1, std::memory_order_relaxed);
            });
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            // sub dropped here — tests concurrent weak_ptr expiration
        }
        done.store(true, std::memory_order_release);
    });

    publisher.join();
    subscriber.join();

    // We don't check exact count — this test verifies no crashes/data races.
    EXPECT_GE(deliveryCount.load(), 0);
    bus.shutdown();
}

// ─── Subscriber dropping during delivery ────────────────────────────

TEST(ConcurrentPubSub, SubscriberDroppingDuringDelivery) {
    EventBus bus(4);
    std::atomic<int> count{0};

    std::vector<std::shared_ptr<ISubscriber>> subscribers;
    for (int i = 0; i < 10; ++i) {
        subscribers.push_back(
            bus.subscribe<StressPayload>([&count](const StressPayload&) {
                count.fetch_add(1, std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }));
    }

    // Start publishing.
    std::thread publisher([&bus] {
        for (int i = 0; i < 50; ++i) {
            bus.publishSync<StressPayload>(0, i);
        }
    });

    // Concurrently drop half the subscribers.
    std::thread dropper([&subscribers] {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        for (int i = 0; i < 5; ++i) {
            subscribers[static_cast<std::size_t>(i)].reset();
        }
    });

    publisher.join();
    dropper.join();

    EXPECT_GT(count.load(), 0);
    bus.shutdown();
}

// ─── High-throughput benchmark-style test ───────────────────────────

TEST(ConcurrentPubSub, HighThroughput) {
    constexpr int kTotalEvents = 10'000;

    EventBus bus(8);
    std::atomic<int> received{0};

    auto sub = bus.subscribe<StressPayload>([&received](const StressPayload&) {
        received.fetch_add(1, std::memory_order_relaxed);
    });

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < kTotalEvents; ++i) {
        bus.publishSync<StressPayload>(0, i);
    }

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_EQ(received.load(), kTotalEvents);

    // Print throughput for informational purposes.
    double eventsPerSec = static_cast<double>(kTotalEvents) /
                          (static_cast<double>(elapsed.count()) / 1'000'000.0);
    std::cout << "[Throughput] " << kTotalEvents << " events in " << elapsed.count()
              << " us (" << static_cast<int>(eventsPerSec) << " events/sec)\n";

    bus.shutdown();
}
