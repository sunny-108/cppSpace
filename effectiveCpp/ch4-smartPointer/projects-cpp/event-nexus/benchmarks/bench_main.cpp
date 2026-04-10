/// @file bench_main.cpp
/// @brief Google Benchmark harness for EventBus throughput.
///
/// Build:  cmake -DBUILD_BENCHMARKS=ON ..
/// Run:    ./bench_event_bus --benchmark_format=json > results/baseline.json

#include <event_nexus/event_bus.h>

#include <benchmark/benchmark.h>

#include <string>

using namespace nexus;

struct BenchPayload {
    int id;
    double value;
    std::string label;
};

// ─── Sync publish with 1 subscriber ─────────────────────────────────
static void BM_SyncPublish_1Sub(benchmark::State& state) {
    EventBus bus(1);
    auto sub = bus.subscribe<BenchPayload>([](const BenchPayload&) {});

    for (auto _ : state) {
        bus.publishSync<BenchPayload>(1, 3.14, "bench");
    }
    bus.shutdown();
}
BENCHMARK(BM_SyncPublish_1Sub);

// ─── Sync publish with N subscribers ────────────────────────────────
static void BM_SyncPublish_NSub(benchmark::State& state) {
    EventBus bus(1);
    auto numSubs = state.range(0);

    std::vector<std::shared_ptr<ISubscriber>> subs;
    for (int64_t i = 0; i < numSubs; ++i) {
        subs.push_back(bus.subscribe<BenchPayload>([](const BenchPayload&) {}));
    }

    for (auto _ : state) {
        bus.publishSync<BenchPayload>(1, 3.14, "bench");
    }
    bus.shutdown();
}
BENCHMARK(BM_SyncPublish_NSub)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

// ─── Async publish throughput ───────────────────────────────────────
static void BM_AsyncPublish(benchmark::State& state) {
    auto numThreads = static_cast<std::size_t>(state.range(0));
    EventBus bus(numThreads);
    auto sub = bus.subscribe<BenchPayload>([](const BenchPayload&) {});

    for (auto _ : state) {
        bus.publishAsync<BenchPayload>(1, 3.14, "bench");
    }
    bus.shutdown();
}
BENCHMARK(BM_AsyncPublish)->Arg(1)->Arg(2)->Arg(4)->Arg(8);

// ─── Subscribe / unsubscribe churn ──────────────────────────────────
static void BM_SubscribeUnsubscribe(benchmark::State& state) {
    EventBus bus(1);

    for (auto _ : state) {
        auto sub = bus.subscribe<BenchPayload>([](const BenchPayload&) {});
        benchmark::DoNotOptimize(sub);
        sub.reset();
    }
    bus.shutdown();
}
BENCHMARK(BM_SubscribeUnsubscribe);

// ─── Event creation (makeEvent) ─────────────────────────────────────
static void BM_MakeEvent(benchmark::State& state) {
    for (auto _ : state) {
        auto event = makeEvent<BenchPayload>(1, 3.14, "bench");
        benchmark::DoNotOptimize(event.get());
    }
}
BENCHMARK(BM_MakeEvent);

BENCHMARK_MAIN();
