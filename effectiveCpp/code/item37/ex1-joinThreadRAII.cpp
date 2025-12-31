/*
    Implement a basic RAII wrapper for std::thread that automatically joins on destruction.
*/

#include <iostream>
#include <thread>
#include <atomic>


class ThreadGuard {
public:
    explicit ThreadGuard(std::thread t) : thread_ (std::move(t)){}
    ~ThreadGuard(){
        if(thread_.joinable()){
            thread_.join();
        }
        // give some time to thread
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
    ThreadGuard(ThreadGuard&&) noexcept = delete;
    ThreadGuard& operator=(ThreadGuard&&) noexcept = delete;
    
private:
    std::thread thread_;
};


class ThreadWithstatus{

public:
enum class status {NOT_STARTED, RUNNING, WAITING, COMPLETED};
/*
    In the template context template<typename Func> explicit ThreadWithStatus(Func&& f), 
    the && creates a forwarding reference (not an rvalue reference):
    If you pass an lvalue → Func deduces to T&
    If you pass an rvalue → Func deduces to T

    // WITHOUT std::forward (BAD):
    thread_ = std::thread([func = f]() {  // Always copies f, even if it was rvalue
        func();
    });

    // WITH std::forward (GOOD):
    thread_ = std::thread([func = std::forward<Func>(f)]() {
        func();  // Moves if f was rvalue, copies if f was lvalue
    });
*/
    template<typename Func>
    explicit ThreadWithstatus(Func&& f) 
        : status_(status::NOT_STARTED)
    {
        thread_ = std::thread([this, func = std::forward<Func>(f)](){
            status_ = status::RUNNING;
            func();
            status_ = status::COMPLETED;
        });
    }

    ~ThreadWithstatus(){
        if(thread_.joinable()){
            thread_.join();
        }
        // give some time to thread
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    status getStatus() const {
        return status_.load();
    }

    bool isJoinable() const {
        return thread_.joinable();
    }

    ThreadWithstatus(const ThreadWithstatus&) = delete;
    ThreadWithstatus& operator=(const ThreadWithstatus&) = delete;

private:
    std::atomic<status> status_;
    std::thread thread_;

};

// Overload << operator for status enum
std::ostream& operator<<(std::ostream& os, ThreadWithstatus::status s) {
    switch(s) {
        case ThreadWithstatus::status::NOT_STARTED:
            return os << "NOT_STARTED";
        case ThreadWithstatus::status::RUNNING:
            return os << "RUNNING";
        case ThreadWithstatus::status::WAITING:
            return os << "WAITING";
        case ThreadWithstatus::status::COMPLETED:
            return os << "COMPLETED";
        default:
            return os << "UNKNOWN";
    }
}

void doWork(){
    std::cout<<"\n doing work "<<std::endl;
}

void jThread(){
    ThreadGuard t(std::thread([](){
        std::cout<<"\n doing thread work () "<<std::endl;
    }));

    ThreadWithstatus ts(doWork);
    std::cout<<" status = "<<ts.getStatus()<<std::endl;
    // Note: Can't check status directly since thread_ is private
    // and we can't access it after moving into ThreadGuard
    std::cout<<" Thread is managed by RAII wrapper "<<std::endl;
}

int main(){
    jThread();

}