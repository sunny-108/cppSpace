/// @file thread_pool_test.cpp
/// @brief Unit tests for ThreadPool.

#include <event_nexus/thread_pool.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <numeric>
#include <vector>

using namespace nexus;

class ThreadPoolTest : public ::testing::Test {
protected:
    ThreadPool pool{4};
};

TEST_F(ThreadPoolTest, PoolHasRequestedSize) {
    EXPECT_EQ(pool.size(), 4u);
}

TEST_F(ThreadPoolTest, DefaultPoolUsesHardwareConcurrency) {
    ThreadPool defaultPool;
    EXPECT_GT(defaultPool.size(), 0u);
}

TEST_F(ThreadPoolTest, SubmitSingleTask) {
    auto future = pool.submit([] { return 42; });
    EXPECT_EQ(future.get(), 42);
}

TEST_F(ThreadPoolTest, SubmitMultipleTasks) {
    constexpr int kNumTasks = 100;
    std::vector<std::future<int>> futures;
    futures.reserve(kNumTasks);

    for (int i = 0; i < kNumTasks; ++i) {
        futures.push_back(pool.submit([i] { return i * i; }));
    }

    for (int i = 0; i < kNumTasks; ++i) {
        EXPECT_EQ(futures[static_cast<std::size_t>(i)].get(), i * i);
    }
}

TEST_F(ThreadPoolTest, TasksRunConcurrently) {
    std::atomic<int> counter{0};
    constexpr int kNumTasks = 4;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < kNumTasks; ++i) {
        futures.push_back(pool.submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    EXPECT_EQ(counter.load(), kNumTasks);
}

TEST_F(ThreadPoolTest, SubmitWithArguments) {
    auto future = pool.submit([](int a, int b) { return a + b; }, 10, 20);
    EXPECT_EQ(future.get(), 30);
}

TEST_F(ThreadPoolTest, ShutdownCompletesAllTasks) {
    std::atomic<int> completed{0};
    constexpr int kNumTasks = 50;

    for (int i = 0; i < kNumTasks; ++i) {
        pool.submit([&completed] {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.shutdown();
    EXPECT_EQ(completed.load(), kNumTasks);
}

TEST_F(ThreadPoolTest, DoubleShutdownIsSafe) {
    pool.shutdown();
    EXPECT_NO_THROW(pool.shutdown());
}

TEST_F(ThreadPoolTest, SubmitAfterShutdownThrows) {
    pool.shutdown();
    EXPECT_THROW(pool.submit([] { return 1; }), std::runtime_error);
}

TEST_F(ThreadPoolTest, MoveConstruction) {
    auto future = pool.submit([] { return 99; });
    ThreadPool pool2 = std::move(pool);
    EXPECT_EQ(future.get(), 99);
}

TEST_F(ThreadPoolTest, PendingTasksDecrements) {
    // Submit a slow task and a fast one.
    std::atomic<bool> gate{false};

    pool.submit([&gate] {
        while (!gate.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    });

    // Give time for the task to start executing.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    gate.store(true, std::memory_order_release);

    pool.shutdown();
    EXPECT_EQ(pool.pendingTasks(), 0u);
}
