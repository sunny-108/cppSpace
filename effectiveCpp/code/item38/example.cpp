#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex cout_mutex;

void doWork(int tid=0){
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout<<"\n started working [ "<<tid<<" ] - "<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout<<"\n workCompleted [ "<<tid<<" ] - "<<std::endl;
}

void blockingExample(){

    std::cout<<"\n Blocking Example ******************* "<<std::endl;
    auto start = std::chrono::steady_clock::now();
    {
        auto future = std::async(std::launch::async, doWork, 1);

        std::cout << "Future created, doing other work..." << std::endl;
        // Don't call get() or wait()
    } // Destructor will wait for three seconds

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Blocked for " << duration.count() << " seconds" << std::endl;

}

void nonBlockingExample(){
    std::cout<<"\n NON Blocking Example ******************* "<<std::endl;
    auto start = std::chrono::steady_clock::now();
    {
        std::packaged_task<void()> task([](){ doWork(2); });
        auto future = task.get_future();
        std::thread t(std::move(task));
        t.detach();

        std::cout<<"\n future created - doing other work !"<<std::endl;
        future.get();
    } //Destructor will not wait - end immediately 

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Blocked for " << duration.count() << " milliseconds" << std::endl;
}

int main(){
    blockingExample();
    nonBlockingExample();
}