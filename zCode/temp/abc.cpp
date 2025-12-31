
#include <future>
#include <iostream>

bool doWork() {
    std::cout<<"doWork executed on thread = "<<std::this_thread::get_id()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return true;
}

int main(){

    auto future = std::async(std::launch::deferred, doWork);  // Default policy
    std::cout<<"parent thread-id = "<<std::this_thread::get_id()<<std::endl;
    // This might wait FOREVER if deferred!
    if (future.wait_for(std::chrono::seconds(2)) == std::future_status::ready) {
        std::cout << "Done!" << std::endl;
    } else {
        for(int i=0; i<10; ++i){
            std::cout << "Still waiting..." << std::endl;  // Might print this!
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if(i==7){
                auto result = future.get();
                std::cout<<"result = "<<result<<std::endl;
                if(result) break;
            }
        }
    }

}
