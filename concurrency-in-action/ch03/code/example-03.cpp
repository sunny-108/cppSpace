#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <string>

void sleep_for_sec(int t){
    std::this_thread::sleep_for(std::chrono::seconds(t));
}
static std::mutex cout_mu;
void print(const std::string &msg){
    std::lock_guard<std::mutex> lock(cout_mu);
    std::cout<<"\n -> "<<msg<<std::endl;
}

class DatabaseConnection{
    public:
    void connect(){
        print(" executing connect .... ");
        sleep_for_sec(1);
        print(" connect - task completed ");
    }
};

std::shared_ptr<DatabaseConnection> connection_ptr;
std::once_flag connection_init_flag;

void getConnection(const std:: string& tid){
    print("thread started working tid = " + tid);
    std::call_once(connection_init_flag, [&tid](){
        print(" Executing call once : tid = " + tid);
        connection_ptr.reset(new DatabaseConnection());
        connection_ptr->connect();
    });
    print("thread completed working tid = " + tid);
}

int main(){
    
    std::thread t1(getConnection, std::string("t1"));
    std::thread t2(getConnection, std::string("t2"));
    std::thread t3(getConnection, std::string("t3"));

    t1.join();
    t2.join();
    t3.join();
    print(" main completed !!");
}