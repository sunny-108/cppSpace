/*
    Implement a parallel file processor that:

    - Reads multiple files concurrently
    - Throws exceptions for files that don't exist
    - Collects all exceptions and reports them

    Create both thread-based and task-based versions.
*/

#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>


class FileProcessor {
private:
    std::mutex mu;
public:
    // Returns content of files that succeeded
    // Throws aggregated exception for files that failed
    
    std::vector<std::string> processFilesThreadBased(const std::vector<std::string>& filenames);
    std::vector<std::string> processFilesTaskBased(const std::vector<std::string>& filenames);

    std::string readFile(std::string fileName){
        std::lock_guard<std::mutex> lock(mu);
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + fileName);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();
        return content;
    }
};

std::vector<std::string> FileProcessor::processFilesTaskBased(const std::vector<std::string>& filenames){
    std::vector<std::future<std::string>> fileContent; 
    for(auto fileName : filenames){
        fileContent.emplace_back(std::async(&FileProcessor::readFile, this, fileName));
        //readFile is a member function - You need to specify which object's
        //readFile to call. Use &FileProcessor::readFile and pass this as the first argument:
    }
    
    std::vector<std::string> errors;
    for(auto& result : fileContent){
        try{
            std::cout<<"-> "<<result.get() << std::endl;
        } catch(const std::exception& e) {
            errors.push_back(e.what());
        }
    }
    return errors;
}

std::vector<std::string> FileProcessor::processFilesThreadBased(const std::vector<std::string>& filenames){
    std::vector<std::thread> threads;
    std::vector<std::string> results;
    std::vector<std::string> errors;
    std::mutex resultsMutex;
    
    for (const auto& fileName: filenames){
        threads.emplace_back([this, fileName, &results, &errors, &resultsMutex](){
            try {
                std::string content = readFile(fileName);
                std::lock_guard<std::mutex> lock(resultsMutex);
                results.push_back(content);
            } catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(resultsMutex);
                errors.push_back(e.what());
            }
        });
    }
    
    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Throw aggregated exception if there were errors
    if (!errors.empty()) {
        std::string aggregated = "Errors occurred:\n";
        for (const auto& err : errors) {
            aggregated += "  - " + err + "\n";
        }
        throw std::runtime_error(aggregated);
    }
    
    return results;
}

void execute(){
    FileProcessor processor;
    std::vector<std::string> files = {"file1.txt", "file2.txt", "missing.txt"};
    auto errors = processor.processFilesTaskBased(files);
    for(auto error : errors){
        std::cout<<" error: "<<error<<std::endl;
    }
}

int main(){
    execute();
}