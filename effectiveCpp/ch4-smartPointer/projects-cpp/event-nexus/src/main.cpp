/// @file main.cpp
/// @brief Demo application: stock-trading event system.
///
/// Demonstrates:
///   - Creating an EventBus with async delivery.
///   - Type-safe publish / subscribe.
///   - Subscriber auto-cleanup via weak_ptr when shared_ptr is dropped.
///   - Sync vs async event delivery.

#include <event_nexus/event_nexus.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// ─── Domain event payloads ──────────────────────────────────────────

struct PriceUpdate {
    std::string symbol;
    double price;
    double volume;
};

struct TradeExecuted {
    std::string symbol;
    double price;
    int quantity;
    std::string side;  // "BUY" or "SELL"
};

struct SystemAlert {
    std::string severity;  // "INFO", "WARN", "ERROR"
    std::string message;
};

// ─── Helpers ────────────────────────────────────────────────────────

void printSeparator(const std::string& title) {
    std::cout << "\n══════════════════════════════════════════════\n"
              << "  " << title
              << "\n══════════════════════════════════════════════\n";
}

// ─── Main ───────────────────────────────────────────────────────────

int main() {
    using namespace nexus;

    std::cout << "event-nexus demo — Stock Trading Event System\n";

    // Create event bus with 4 worker threads for async delivery.
    EventBus bus(4);

    // ── 1. Register subscribers ─────────────────────────────────────
    printSeparator("1. Registering subscribers");

    // Price logger — logs all price updates.
    auto priceLogger = bus.subscribe<PriceUpdate>([](const PriceUpdate& p) {
        std::cout << "[PriceLogger] " << p.symbol << " = $" << p.price
                  << " (vol: " << p.volume << ")\n";
    });

    // Trade monitor — logs all executed trades.
    auto tradeMonitor = bus.subscribe<TradeExecuted>([](const TradeExecuted& t) {
        std::cout << "[TradeMonitor] " << t.side << " " << t.quantity << " " << t.symbol
                  << " @ $" << t.price << "\n";
    });

    // Alert handler — logs system alerts.
    auto alertHandler = bus.subscribe<SystemAlert>([](const SystemAlert& a) {
        std::cout << "[AlertHandler] [" << a.severity << "] " << a.message << "\n";
    });

    // Risk checker — also listens to trades (multiple subscribers for same event).
    auto riskChecker = bus.subscribe<TradeExecuted>([](const TradeExecuted& t) {
        double notional = t.price * t.quantity;
        if (notional > 100000.0) {
            std::cout << "[RiskChecker] ⚠ LARGE TRADE: $" << notional << " on " << t.symbol
                      << "\n";
        }
    });

    std::cout << "Registered " << bus.activeSubscriberCount() << " subscribers.\n";

    // ── 2. Synchronous publish ──────────────────────────────────────
    printSeparator("2. Synchronous events");

    bus.publishSync<PriceUpdate>("AAPL", 178.50, 1'200'000.0);
    bus.publishSync<PriceUpdate>("GOOGL", 141.20, 800'000.0);
    bus.publishSync<TradeExecuted>("AAPL", 178.50, 500, "BUY");
    bus.publishSync<SystemAlert>("INFO", "Market session opened");

    // ── 3. Asynchronous publish ─────────────────────────────────────
    printSeparator("3. Asynchronous events");

    bus.publishAsync<TradeExecuted>("GOOGL", 141.25, 1000, "BUY");
    bus.publishAsync<TradeExecuted>("TSLA", 250.00, 200, "SELL");
    bus.publishAsync<PriceUpdate>("TSLA", 249.80, 3'500'000.0);

    // Give async events time to be delivered.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // ── 4. Subscriber auto-cleanup (weak_ptr expiration) ────────────
    printSeparator("4. Subscriber auto-cleanup");

    std::cout << "Active subscribers before drop: " << bus.activeSubscriberCount() << "\n";

    // Drop the risk checker — its shared_ptr is released.
    // The EventBus holds only a weak_ptr, so it auto-expires.
    riskChecker.reset();

    std::cout << "Active subscribers after dropping riskChecker: "
              << bus.activeSubscriberCount() << "\n";

    // This trade won't trigger the risk checker anymore.
    bus.publishSync<TradeExecuted>("NVDA", 900.00, 500, "BUY");

    // Purge expired weak_ptrs and report.
    auto purged = bus.purgeExpired();
    std::cout << "Purged " << purged << " expired subscriber(s).\n";
    std::cout << "Active subscribers after purge: " << bus.activeSubscriberCount() << "\n";

    // ── 5. Scoped subscriber lifetime ───────────────────────────────
    printSeparator("5. Scoped subscriber (RAII)");

    {
        // This subscriber only lives within this scope.
        auto scopedSub = bus.subscribe<PriceUpdate>([](const PriceUpdate& p) {
            std::cout << "[ScopedSub] Saw " << p.symbol << " @ $" << p.price << "\n";
        });

        bus.publishSync<PriceUpdate>("MSFT", 415.30, 600'000.0);
        std::cout << "Active subscribers inside scope: " << bus.activeSubscriberCount() << "\n";
    }
    // scopedSub is destroyed here — weak_ptr in bus auto-expires.

    bus.publishSync<PriceUpdate>("MSFT", 415.50, 700'000.0);
    std::cout << "Active subscribers outside scope: " << bus.activeSubscriberCount() << "\n";

    // ── 6. Shutdown ─────────────────────────────────────────────────
    printSeparator("6. Shutdown");
    bus.shutdown();
    std::cout << "EventBus shut down cleanly.\n";

    return 0;
}
