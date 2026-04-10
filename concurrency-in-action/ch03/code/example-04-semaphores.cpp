/*
    Semaphores
    Best for: Limiting access to resource pool.
    Advantages: Control resource access count
    Use for: Database connections, thread pools etc.
*/

#include <semaphore> //C++20
#include <thread>
#include <vector>

std::counting_semaphore<5> seamaphore{5};
