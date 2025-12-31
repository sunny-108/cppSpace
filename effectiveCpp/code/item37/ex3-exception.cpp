/*
*/


#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

class ThreadGuard{

    std::thread thread_;
    std::exception_ptr exception_;

public:
    template<typename Func>
    explicit ThreadGuard(Func&& f) {
        thread_ = std::thread([this, func = std::forward<Func>(f)](){
            try{
                func();
            } catch (...) {
                exception_ = std::current_exception();
            }
        });
    }

    ~ThreadGuard(){
        std::cout<<"\n ThreadGuard destructor .... "<<std::endl;
        if(thread_.joinable()){
            thread_.join();
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout<<"\n destructor: thread_.joinable = "<< thread_.joinable() <<std::endl;
    }

    void rethrow(){
        if(exception_){
            std::rethrow_exception(exception_);
        }
    }
};

void doWork(){
    std::cout<<"\n started working .... "<<std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    throw std::runtime_error("caught runtime Error !!");
    std::cout<<"\n work completed .... "<<std::endl;
}

int main(){
    ThreadGuard t(doWork);
    try {
        t.rethrow();
    } catch(const std::exception &e) {
        std::cout<<"\n Exception: "<< e.what() <<std::endl;
    }
}