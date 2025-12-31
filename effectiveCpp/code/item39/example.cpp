#include <iostream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <future>


void promiseFuture(){
    std::promise<void> prom;

    std::thread waiter([fut = prom.get_future()](){
        std::cout<<"\n waiting for signal ..... "<<std::endl;
        fut.wait(); // just wait
        std::cout<<"\n Signal recived .......... "<<std::endl;
    });

    std::thread signaler([&prom](){
        std::cout<<"\n Signaler doing work ---- "<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"\n Signaler task completed - going to give signal !! "<<std::endl;
        prom.set_value(); //signal
        std::cout<<"\n signal done !!!!! "<<std::endl;
    });

    waiter.join();
    signaler.join();
}

void thruCV(){
    std::condition_variable cv;
    std::mutex mu;
    bool flag = false;

    std::thread waiter([&](){
        std::cout<<"\n waiting for signal ..... "<<std::endl;
        std::unique_lock<std::mutex> lock(mu);
        cv.wait(lock, [&](){
            return flag;
        });
        std::cout<<"\n Signal recived .......... "<<std::endl;
    });

    std::thread signaler([&](){

        std::cout<<"\n Signaler doing work ---- "<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"\n Signaler task completed - going to give signal !! "<<std::endl;

        {
            std::lock_guard<std::mutex> lock(mu);
            flag = true;
        }

        cv.notify_one();

        std::cout<<"\n signal done !!!!! "<<std::endl;
    });

    waiter.join();
    signaler.join();
}

void busyWait(){
    bool flag = false;

    std::thread waiter([&](){
        std::cout<<"\n waiting for signal ..... "<<std::endl;
        while(!flag){
            std::cout<<"\n going to wait for 400 ms"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        std::cout<<"\n Signal recived .......... "<<std::endl;
    });

    std::thread signaler([&](){
        std::cout<<"\n Signaler doing work ---- "<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout<<"\n Signaler task completed - going to give signal !! "<<std::endl;
        flag = true;
        std::cout<<"\n signal done !!!!! "<<std::endl;

    });

    waiter.join();
    signaler.join();
}

int main(){
    //promiseFuture();
    //thruCV();
    busyWait();
}