/*
    Implement parallel matrix multiplication using both approaches:

    - Thread-based with manual thread management
    - Task-based with std::async

    Compare the code complexity and performance.
*/

#include <iostream>
#include <thread>
#include <future>
#include <vector>

using Matrix = std::vector<std::vector<int>>;

class MatrixMultiplier {
public:
    Matrix multiplyThreadBased(const Matrix& A, const Matrix& B);
    Matrix multiplyTaskBased(const Matrix& A, const Matrix& B);
};

Matrix MatrixMultiplier::multiplyThreadBased(const Matrix& A, const Matrix& B){
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    // Create result matrix initialized to 0
    Matrix result(rowsA, std::vector<int>(colsB, 0));
    
    // Create threads - one thread per row
    std::vector<std::thread> threads;
    
    for (int i = 0; i < rowsA; i++) {
        threads.emplace_back([&A, &B, &result, i, colsA, colsB]() {
            for (int j = 0; j < colsB; j++) {
                for (int k = 0; k < colsA; k++) {
                    result[i][j] += A[i][k] * B[k][j];
                }
            }
        });
    }
    
    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
    
    return result;
}

Matrix MatrixMultiplier::multiplyTaskBased(const Matrix& A, const Matrix& B){
    int rowsA = A.size();
    std::cout << " "<< rowsA <<std::endl;
    int colsA = A[0].size();
    int colsB = B[0].size();

    // Create result matrix initialized to 0
    Matrix result(rowsA, std::vector<int>(colsB, 0));

    std::vector<std::future<void>> taskList;
    for(int i=0; i<rowsA; ++i){
        taskList.emplace_back(std::async(std::launch::async, [&A, &B, &result, i, colsA, colsB](){
            for (int j = 0; j < colsB; j++) {
                for (int k = 0; k < colsA; k++) {
                    result[i][j] += A[i][k] * B[k][j];
                }
            }
        }));
    }

    for(auto& fut : taskList){
        fut.wait();
    }

    return result;
}

void printMatrix(const Matrix& mat, const std::string& name) {
    std::cout << name << ":\n";
    for (const auto& row : mat) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    MatrixMultiplier multiplier;
    
    // Create test matrices
    Matrix A = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    Matrix B = {
        {7, 8},
        {9, 10},
        {11, 12}
    };
    
    std::cout << "Matrix Multiplication Test\n";
    std::cout << "===========================\n\n";
    
    printMatrix(A, "Matrix A (2x3)");
    printMatrix(B, "Matrix B (3x2)");
    
    // Thread-based multiplication
    auto resultThread = multiplier.multiplyThreadBased(A, B);
    printMatrix(resultThread, "Result (Thread-based)");
    
    // Task-based multiplication
    auto resultTask = multiplier.multiplyTaskBased(A, B);
    printMatrix(resultTask, "Result (Task-based)");
    
    return 0;
}