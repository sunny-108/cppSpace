#include <iostream>
#include <thread>

class RaceConditionDemo {
    static constexpr int iterations = 100000;
    static constexpr int numThreads = 10;
public:
    // Broken: Uses volatile (NOT thread-safe)
    static void brokenCounter(){
        int counter = 0;

        auto increment = [&counter, n = iterations](){
            for(int i=0; i<n; i++) ++counter;
        };
        std::vector<std::thread> threads;
        for(int i=0; i<numThreads; ++i){
            threads.emplace_back(increment);
        }

        for(auto& t : threads){
            t.join();
        }

        std::cout << "Volatile counter: " << counter << std::endl;
    }
    
    // Fixed: Uses std::atomic (thread-safe)
    static void fixedCounter(){
        std::atomic<int> counter {0};
        
        auto increment = [&counter, n = iterations](){
            for(int i=0; i<n; i++) ++counter;
        };
        std::vector<std::thread> threads;
        for(int i=0; i<numThreads; ++i){
            threads.emplace_back(increment);
        }

        for(auto& t : threads){
            t.join();
        }

        std::cout << "Atomic counter: " << counter << std::endl;
    }
    
    // Compare and show lost updates
    static void demonstrateRace(){
        brokenCounter();
        fixedCounter();
    }
};

int main(){
    RaceConditionDemo::demonstrateRace();
}