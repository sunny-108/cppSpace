#include <iostream>
#include <thread>
#include <mutex>

template<typename T>
class ThreadUnsafeStack {
    std::stack<T> data;
    mutable std::mutex mtx;
    
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push(value);
    }
    
    // PROBLEM: This interface is not thread-safe!
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }
    
    T top() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.top();
    }
    
    void pop() {
        std::lock_guard<std::mutex> lock(mtx);
        data.pop();
    }

    T top_pop() {
        std::lock_guard<std::mutex> lock(mtx);
        T value = 0;
        if(!data.empty()){
            value = data.top();
            data.pop();
        } else {
            throw std::runtime_error("data is empty");
        }
        return value;
    }
};

void sleep_for_sec(const int t){
    std::this_thread::sleep_for(std::chrono::seconds(t));
}

int main(){
    // Usage (UNSAFE):
    ThreadUnsafeStack<int> stack;
    for(int i=10; i<11; ++i){
        stack.push(i);
    }
    std::thread t1([&](){         
        if(!stack.empty()) {  
            sleep_for_sec(1); 
            try{    
            int value = stack.top_pop();  
            std::cout<<"\n t1 value = "<<value<<std::endl;
            } catch (const std::exception &e){
                std::cout<<"Exception: "<<e.what()<<std::endl;
            }
        }
    });

    std::thread t2([&](){ 
        if(!stack.empty()) {       
            int value = stack.top_pop();  
            std::cout<<"\n t2 value = "<<value<<std::endl;
        }
    });

    t1.join();
    t2.join();
}
