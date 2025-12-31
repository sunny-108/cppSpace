/*
    Create a utility that detects which launch policy was used by checking future status.
*/

#include <future>
#include <iostream>
#include <chrono>
#include <fstream>

enum class LaunchPolicy {
    Async,
    Deferred,
    Timeout,
    Unknown
};

std::ostream& operator<<(std::ostream &os, LaunchPolicy policy){
    switch(policy){
        case LaunchPolicy::Async: return os << "Async";
        case LaunchPolicy::Deferred: return os << "Deferred";
        case LaunchPolicy::Timeout: return os << "Timeout";
        case LaunchPolicy::Unknown: return os << "Unknown";
    }
    return os;
}

class PolicyDetector {
public:
    template<typename F>
    static LaunchPolicy detectPolicy(F& f);
    
    static void demonstrateAllPolicies();
};

template<typename F>
LaunchPolicy PolicyDetector::detectPolicy(F& f){
    auto status = f.wait_for(std::chrono::seconds(5));
    
    if(status == std::future_status::deferred){
        return LaunchPolicy::Deferred;
    } else if (status == std::future_status::ready){
        return LaunchPolicy::Async;
    } else if (status == std::future_status::timeout) {
        return LaunchPolicy::Timeout;
    } else {
        return LaunchPolicy::Unknown;
    }
}

void PolicyDetector::demonstrateAllPolicies(){

    auto f1 = std::async(std::launch::async, [](){
        long sum = 0;
        for(int i=0; i<1000; ++i)
            sum += i;
        return sum;
    });

    auto f2 = std::async(std::launch::deferred, [](){
        long sum = 0;
        for(int i=0; i<1000; ++i)
            sum += i;
        return sum;
    });

    std::cout<< "LaunchPolicy for f1 = "<< detectPolicy(f1) << std::endl;
    std::cout<< "LaunchPolicy for f2 = "<< detectPolicy(f2) << std::endl;

    std::cout<<" result from f1 "<<f1.get() << std::endl;
    std::cout<<" result from f2 "<<f2.get() << std::endl;

}

int main(){

    PolicyDetector::demonstrateAllPolicies();

}

