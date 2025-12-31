/*
    Extend ThreadGuard to support both joining and detaching policies.
*/

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

enum class ThreadPolicy {
    Join,
    Detach
};

// Forward declare operator<< so it can be used in the class
std::ostream& operator<<(std::ostream& os, ThreadPolicy policy);

class ConfigurableThreadGuard {
public:
    ConfigurableThreadGuard(std::thread t, ThreadPolicy policy) : thread_ (std::move(t)), policy_ (policy) {}
    ~ConfigurableThreadGuard(){
        std::cout<<"\n in destructor Policy = " << getPolicy();
        if (thread_.joinable()){
            if(policy_ == ThreadPolicy::Detach){
                thread_.detach();
            } else {
                thread_.join(); 
            }
        }
        std::cout<<"\n in destructor is thread joinable  = " << thread_.joinable() <<std::endl;
    }
    
    void changePolicy(ThreadPolicy newPolicy) {
        policy_ = newPolicy;
    }
    ThreadPolicy getPolicy() const {
        return policy_;
    }
    
    ConfigurableThreadGuard(const ConfigurableThreadGuard&) = delete;
    ConfigurableThreadGuard& operator=(const ConfigurableThreadGuard&) = delete;
    
private:
    std::thread thread_;
    ThreadPolicy policy_;
};

std::ostream& operator<< (std::ostream& os, ThreadPolicy policy){
    switch(policy){
        case ThreadPolicy::Detach : return os << "Detach";
        case ThreadPolicy::Join : return os << "Join";
        default: return os << "Unknown";
    }
}

void unsafeVersion(){

}

int main() {

    ConfigurableThreadGuard t(std::thread([](){
        std::cout<<"\n thread starting work "<<std::endl;
        try{
            std::this_thread::sleep_for(std::chrono::seconds(1));
            throw std::runtime_error("Error! code 42");
        } catch (const std::exception& e){
            std::cout << "Caught: " << e.what() << std::endl;
        }
        std::cout<<"\n thread completed work "<<std::endl;
    }), ThreadPolicy::Join);

    std::cout<<"\n current Policy = " << t.getPolicy();
    t.changePolicy(ThreadPolicy::Detach);
    std::cout<<"\n current Policy = " << t.getPolicy();
    
    // Demonstrate the problem: without this sleep, detached thread dies with main
    std::cout<<"\n Main sleeping to let detached thread finish..."<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout<<"\n Main exiting now"<<std::endl;
}