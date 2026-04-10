#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <string>

class ProdCons{

    std::mutex mtx;
    std::mutex pmtx;
    std::condition_variable cv;
    std::queue<int> data_q;
    bool isFinished = false;

    public:

    void print(std::string msg){
        std::lock_guard<std::mutex> lock(pmtx);
        std::cout<<"-> "<< msg <<std::endl;
    }

    void producer()
    {
        for(int i=0; i<5; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            {
                std::lock_guard<std::mutex> lock(mtx);
                data_q.push(i);
            }
            print("Produced data: " + std::to_string(i));
            cv.notify_one();
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            isFinished = true;
        }
            cv.notify_one();
    }

    void consumer()
    {
        while(true)
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [&](){
                return !data_q.empty() || isFinished;
            });
            while(data_q.empty() != true){
                auto val = data_q.front();
                data_q.pop();
                print("consumed data: " + std::to_string(val));
            }

            if(data_q.empty() && isFinished){
                break;
            }
        }

    }
};

int main()
{
    ProdCons pcObj;
    std::thread t1(&ProdCons::producer, &pcObj);
    std::thread t2(&ProdCons::consumer, &pcObj);
    t1.join();
    t2.join();
}