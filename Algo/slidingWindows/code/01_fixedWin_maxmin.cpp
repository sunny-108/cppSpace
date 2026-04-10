/*
    Fixed window max min
*/

#include <iostream>
#include <vector>
#include <random>

const unsigned winsize = 2;

std::vector<int> generateRandomNumbers(int count = 10, int min = 0, int max = 100) {
    std::vector<int> numbers;
    numbers.reserve(count);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    
    for (int i = 0; i < count; ++i) {
        numbers.push_back(dis(gen));
    }
    
    return numbers;
}

void getMaxFixedWindow(std::vector<int> v){
    auto n = v.size();
    if (n < winsize) {
        std::cout << "Error: Vector size is smaller than window size" << std::endl;
        return;
    }
    
    // Calculate initial window sum
    int sum = 0; 
    for(size_t i = 0; i < winsize; ++i){
        sum += v[i];
    }
    int max_sum = sum;
    
    // Slide the window
    for(size_t i = winsize; i < n; ++i){
        sum = sum - v[i-winsize] + v[i];
        max_sum = std::max(max_sum, sum);
    }
    std::cout << "Max = " << max_sum << std::endl;
}


void getMinFixedWindow(std::vector<int> v){
    auto n = v.size();
    if (n < winsize) {
        std::cout << "Error: Vector size is smaller than window size" << std::endl;
        return;
    }
    
    // Calculate initial window sum
    int sum = 0;
    for(size_t i = 0; i < winsize; ++i){
        sum += v[i];
    }
    int min_sum = sum;
    
    // Slide the window
    for(size_t i = winsize; i < n; ++i){
        sum = sum - v[i-winsize] + v[i];
        min_sum = std::min(min_sum, sum);
    }
    std::cout << "Min = " << min_sum << std::endl;
}

int main (){
    // Generate 10 random numbers between 0 and 100
    auto numbers = generateRandomNumbers();
    
    std::cout << "Random numbers: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    getMaxFixedWindow(numbers);
    getMinFixedWindow(numbers);
    
    return 0;
}