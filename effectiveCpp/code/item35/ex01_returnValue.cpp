// Calculate the sum of numbers from 1 to N using 4 threads/tasks, 
//each handling a quarter of the range.

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <future>

// Thread-based version
void parallelSumThreadBased(long start, long end, long &sum){
    for(long i=start; i <= end; ++i){
        sum = sum + i;
    }
}

// Task-based version
long parallelSumTaskBased(long start, long end){
    long sum = 0;
    for(long i=start; i <= end; ++i){
        sum = sum + i;
    }
    return sum;
}

void threadBased(){
    std::cout<<"\n ****************** Thread based ******************\n"<<std::endl;
    std::vector<long> result = {0, 0, 0, 0};
    long N = 10000;
    std::thread t1(parallelSumThreadBased, 1, N/4, std::ref(result[0]));
    std::thread t2(parallelSumThreadBased, N/4+1, N/2, std::ref(result[1]));
    std::thread t3(parallelSumThreadBased, N/2+1, 3*N/4, std::ref(result[2]));
    std::thread t4(parallelSumThreadBased, 3*N/4+1, N, std::ref(result[3]));
   
    if(t1.joinable()) t1.join();
    if(t2.joinable()) t2.join();
    if(t3.joinable()) t3.join();
    if(t4.joinable()) t4.join();

    long long sum = 0;
    for (int i=0; i<4; ++i){
        std::cout<<" result for thread t"<<i+1<<" = "<<result.at(i)<<std::endl;
        sum = sum + result.at(i);
    }
    std::cout<<" sum = "<<sum<<std::endl;
}

void taskbased(){
    std::cout<<"\n ****************** Task based ******************\n"<<std::endl;
    long N = 10000;
    std::vector<long> result = {0, 0, 0, 0};
    std::vector<std::future<long>> futures;
  
    
    futures.emplace_back(std::async(std::launch::async, parallelSumTaskBased, 1, N/4));
    futures.emplace_back(std::async(std::launch::async, parallelSumTaskBased, N/4+1, N/2));
    futures.emplace_back(std::async(parallelSumTaskBased, N/2+1, 3*N/4));
    futures.emplace_back(std::async(parallelSumTaskBased, 3*N/4+1, N));
    
    long long sum = 0;
    for(int i=0; i<4; ++i){
        result[i] = futures[i].get();
        std::cout<<" result for thread t"<<i+1<<" = "<<result.at(i)<<std::endl;
        sum = sum + result.at(i);
    }
    std::cout<<" sum = "<<sum<<std::endl;
}

int main(){
    auto startTime_thread = std::chrono::high_resolution_clock::now();
    threadBased();
    auto endTime_thread = std::chrono::high_resolution_clock::now();
    long duration_thread = std::chrono::duration_cast<std::chrono::nanoseconds> (endTime_thread - startTime_thread).count();
    std::cout<< "time taken by Thread based = "<<duration_thread<<" ns"<<std::endl;


    auto startTime_task = std::chrono::high_resolution_clock::now();
    taskbased();
    auto endTime_task = std::chrono::high_resolution_clock::now();
    long duration_task = std::chrono::duration_cast<std::chrono::nanoseconds> (endTime_task - startTime_task).count();
    std::cout<< "time taken by Task based = "<<duration_task<<" ns"<<std::endl;
}