#include <iostream>
#include <thread>
#include <string>
#include <mutex>

// A mutex just to keep the output from getting jumbled up
std::mutex printMutex;

// 1. Define the thread_local variable
// This acts like a global variable, but each thread gets its own private copy.
// It initializes to 0 for EVERY new thread that starts.
thread_local int myStepCounter = 0;

// Helper to print cleanly
void log(const std::string& funcName) {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << "[Thread " << std::this_thread::get_id() << "] " 
              << funcName << " incremented counter to: " 
              << myStepCounter << std::endl;
}

// 2. Define multiple functions that modify the SAME thread_local variable
void prepareData() {
    myStepCounter++; // 0 -> 1
    log("prepareData");
}

void processData() {
    myStepCounter++; // 1 -> 2
    log("processData");
}

void saveData() {
    myStepCounter++; // 2 -> 3
    log("saveData");
}

// 3. The worker function that calls them in sequence
void workerRoutine() {
    // Counter starts at 0 for this thread automatically
    
    prepareData();
    processData();
    saveData();
    
    // We can call them again, state is remembered
    processData(); 
}

int main() {
    // We spawn a thread (t1) that is NOT the main thread
    std::thread t1(workerRoutine);
    
    // We spawn a SECOND thread (t2) to prove they don't interfere
    // t2 will have its own 'myStepCounter' starting at 0
    std::thread t2(workerRoutine);

    t1.join();
    t2.join();

    return 0;
}
