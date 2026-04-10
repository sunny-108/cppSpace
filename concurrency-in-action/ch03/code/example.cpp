//Deadlock example

#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <chrono>
#include <vector>

std::mutex m1, m2;
std::mutex printmutex;

void print(const std::string &msg){
    std::lock_guard<std::mutex> lock(printmutex);
    std::cout<<"\n -> "<<msg<<std::endl;
}

void thread_func01(){
    print("thread_func01 - trying to get lock on both mutex [m1] and [m2]....");
    std::lock(m1, m2);
    print("thread_func01 - both mutex m1 and m2 locked");
    std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
    print("thread_func01 - now lock_guard taken ownership of both mutex m1 and m2");
    print("thread_func01 - Operation Completed releasing both mutex");
}

void thread_func02(){
    print("thread_func02 - trying to get lock on both mutex [m1] and [m2]....");
    std::lock(m1, m2);
    print("thread_func02 - both mutex m1 and m2 locked");
    std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
    print("thread_func02 - now lock_guard taken ownership of both mutex m1 and m2");
    print("thread_func02 - Operation Completed releasing both mutex");
}

void threadsFunc(){
    for(int i=0; i<10; ++i){
        std::thread t(print, std::to_string(i));

        t.detach();
    }
}

void poolOfThreads(){
    std::vector<std::thread> threadPool;
    for(int i=0; i<10; ++i){
        threadPool.emplace_back([i](){
            std::cout<<"creating thread : "<<i <<std::endl;
        });
    }

    for(auto& t : threadPool){
        t.join();
    }
}

int main(){
    // std::thread t1(thread_func01);
    // std::thread t2(thread_func02);
    // t1.join();
    // t2.join();
    //poolOfThreads();
    threadsFunc();
}