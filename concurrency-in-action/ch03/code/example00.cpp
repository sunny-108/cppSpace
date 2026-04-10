//Showing Race Condition

#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <shared_mutex>
#include <string>
#include <map>

class ThreadSafeMap{

    std::map<std::string, int> data;
    mutable std::shared_mutex mtx;  // Use shared_timed_mutex for C++14

    public:
    
    //multiple reader can get access
    int get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto it = data.find(key);
        return (it != data.end()) ? it->second: -1;
    }

    //Only one writer
    void set(const std::string& key, int value){
        std::unique_lock<std::shared_mutex> lock(mtx);
        data[key] = value;
    }

};


int main(){
    std::cout << "=== ThreadSafeMap Testing Scenarios ===\n\n";

    ThreadSafeMap safeMap;

    // ===========================================
    // Scenario 1: Basic Functionality
    // ===========================================
    std::cout << "1. Basic Set/Get Operations:\n";
    safeMap.set("apple", 10);
    safeMap.set("banana", 20);
    safeMap.set("cherry", 30);

    std::cout << "apple: " << safeMap.get("apple") << std::endl;
    std::cout << "banana: " << safeMap.get("banana") << std::endl;
    std::cout << "cherry: " << safeMap.get("cherry") << std::endl;
    std::cout << "grape: " << safeMap.get("grape") << " (not found)\n\n";

    // ===========================================
    // Scenario 2: Multiple Readers (Should work concurrently)
    // ===========================================
    std::cout << "2. Multiple Readers Test:\n";
    auto reader = [&safeMap](int id) {
        for (int i = 0; i < 5; ++i) {
            int value = safeMap.get("apple");
            std::cout << "Reader " << id << " got: " << value << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::thread reader1(reader, 1);
    std::thread reader2(reader, 2);
    std::thread reader3(reader, 3);

    reader1.join();
    reader2.join();
    reader3.join();
    std::cout << "All readers finished concurrently!\n\n";

    // ===========================================
    // Scenario 3: Writer Blocking Readers
    // ===========================================
    std::cout << "3. Writer Blocking Readers:\n";
    std::atomic<bool> readers_done{false};

    auto slow_reader = [&safeMap, &readers_done](int id) {
        while (!readers_done) {
            int value = safeMap.get("apple");
            std::cout << "Reader " << id << " reading: " << value << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::cout << "Reader " << id << " finished\n";
    };

    std::thread reader4(slow_reader, 4);
    std::thread reader5(slow_reader, 5);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "Starting writer (should block readers)...\n";
    auto start = std::chrono::high_resolution_clock::now();

    safeMap.set("apple", 999);  // This should block readers

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Writer finished after " << duration.count() << "ms\n";

    readers_done = true;
    reader4.join();
    reader5.join();
    std::cout << "Final apple value: " << safeMap.get("apple") << "\n\n";

    // ===========================================
    // Scenario 4: Performance Comparison - Safe vs Unsafe
    // ===========================================
    std::cout << "4. Performance: ThreadSafeMap vs Unsafe Map\n";

    // Unsafe map for comparison
    std::map<std::string, int> unsafeMap;
    std::mutex unsafeMutex;

    const int iterations = 10000;

    // Test ThreadSafeMap
    auto safe_writer = [&safeMap](int start) {
        for (int i = 0; i < iterations; ++i) {
            safeMap.set("key" + std::to_string(start + i), i);
        }
    };

    auto safe_reader = [&safeMap]() {
        for (int i = 0; i < iterations; ++i) {
            safeMap.get("key" + std::to_string(i));
        }
    };

    // Test unsafe map
    auto unsafe_writer = [&unsafeMap, &unsafeMutex](int start) {
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(unsafeMutex);
            unsafeMap["key" + std::to_string(start + i)] = i;
        }
    };

    auto unsafe_reader = [&unsafeMap, &unsafeMutex]() {
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(unsafeMutex);
            auto it = unsafeMap.find("key" + std::to_string(i));
        }
    };

    // Time ThreadSafeMap
    start = std::chrono::high_resolution_clock::now();
    std::thread safe_w1(safe_writer, 0);
    std::thread safe_w2(safe_writer, iterations);
    std::thread safe_r1(safe_reader);
    std::thread safe_r2(safe_reader);

    safe_w1.join();
    safe_w2.join();
    safe_r1.join();
    safe_r2.join();
    end = std::chrono::high_resolution_clock::now();
    auto safe_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Time unsafe map
    start = std::chrono::high_resolution_clock::now();
    std::thread unsafe_w1(unsafe_writer, 0);
    std::thread unsafe_w2(unsafe_writer, iterations);
    std::thread unsafe_r1(unsafe_reader);
    std::thread unsafe_r2(unsafe_reader);

    unsafe_w1.join();
    unsafe_w2.join();
    unsafe_r1.join();
    unsafe_r2.join();
    end = std::chrono::high_resolution_clock::now();
    auto unsafe_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "ThreadSafeMap time: " << safe_duration.count() << "ms\n";
    std::cout << "Unsafe map time: " << unsafe_duration.count() << "ms\n";
    std::cout << "Overhead: " << (safe_duration.count() - unsafe_duration.count()) << "ms\n\n";

    // ===========================================
    // Scenario 5: Demonstrating Race Condition (with unsafe access)
    // ===========================================
    std::cout << "5. Race Condition Demo (Unsafe Access):\n";

    std::map<std::string, int> raceMap;
    int race_counter = 0;

    auto race_writer = [&raceMap, &race_counter]() {
        for (int i = 0; i < 1000; ++i) {
            raceMap["counter"] = ++race_counter;
        }
    };

    auto race_reader = [&raceMap, &race_counter]() {
        for (int i = 0; i < 1000; ++i) {
            int val = raceMap["counter"];
            if (val != race_counter) {
                std::cout << "Race detected! Map has " << val
                         << " but counter is " << race_counter << std::endl;
            }
        }
    };

    std::thread race_w1(race_writer);
    std::thread race_w2(race_writer);
    std::thread race_r1(race_reader);
    std::thread race_r2(race_reader);

    race_w1.join();
    race_w2.join();
    race_r1.join();
    race_r2.join();

    std::cout << "Race condition test completed\n";
    std::cout << "Final counter: " << race_counter << std::endl;
    std::cout << "Map counter: " << raceMap["counter"] << std::endl;

    std::cout << "\n=== All Tests Completed ===\n";

    return 0;
}

