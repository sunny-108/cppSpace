/**
 * SINGLETON PATTERN
 * 
 * Purpose: Ensure a class has only one instance and provide a global point of access to it.
 * 
 * Use Cases:
 * - Logging system
 * - Configuration manager
 * - Database connection pool
 * - Thread pool manager
 * 
 * Key Concepts:
 * - Private constructor
 * - Static instance
 * - Thread-safe initialization (C++11 magic statics)
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

// TODO: Implement a thread-safe Singleton Logger class
class Logger {
private:
    // TODO: Add private constructor
    
    // TODO: Delete copy constructor and assignment operator
    
    std::string logLevel_;
    
public:
    // TODO: Implement getInstance() method that returns a reference to the single instance
    // Hint: Use C++11 magic statics for thread-safety
    
    // TODO: Implement log method
    void log(const std::string& message) {
        // TODO: Print message with format: "[LEVEL] message"
    }
    
    void setLogLevel(const std::string& level) {
        logLevel_ = level;
    }
    
    std::string getLogLevel() const {
        return logLevel_;
    }
};

// TODO: Implement a Database Connection Singleton
class DatabaseConnection {
private:
    std::string connectionString_;
    bool connected_;
    
    // TODO: Private constructor
    
public:
    // TODO: Implement getInstance()
    
    void connect(const std::string& connStr) {
        connectionString_ = connStr;
        connected_ = true;
        std::cout << "Connected to database: " << connStr << std::endl;
    }
    
    void disconnect() {
        connected_ = false;
        std::cout << "Disconnected from database" << std::endl;
    }
    
    bool isConnected() const {
        return connected_;
    }
    
    void executeQuery(const std::string& query) {
        if (connected_) {
            std::cout << "Executing query: " << query << std::endl;
        } else {
            std::cout << "Error: Not connected to database" << std::endl;
        }
    }
};

// BONUS: Implement a generic Singleton template
template<typename T>
class Singleton {
    // TODO: Implement a generic singleton base class that other classes can inherit from
};

int main() {
    std::cout << "=== Singleton Pattern Exercise ===\n\n";
    
    // Test Logger Singleton
    std::cout << "--- Logger Test ---" << std::endl;
    // TODO: Get logger instance and test logging
    // Logger& logger1 = Logger::getInstance();
    // logger1.setLogLevel("INFO");
    // logger1.log("Application started");
    
    // TODO: Verify it's the same instance
    // Logger& logger2 = Logger::getInstance();
    // logger2.log("Same instance test");
    // std::cout << "Same instance? " << (&logger1 == &logger2) << std::endl;
    
    std::cout << "\n--- Database Connection Test ---" << std::endl;
    // TODO: Get database connection instance and test
    // DatabaseConnection& db1 = DatabaseConnection::getInstance();
    // db1.connect("localhost:5432/mydb");
    // db1.executeQuery("SELECT * FROM users");
    
    // TODO: Verify singleton behavior
    // DatabaseConnection& db2 = DatabaseConnection::getInstance();
    // db2.executeQuery("INSERT INTO users VALUES (1, 'John')");
    // std::cout << "Same instance? " << (&db1 == &db2) << std::endl;
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Implement the Logger singleton with thread-safe getInstance()
 * 2. Implement the DatabaseConnection singleton
 * 3. Add proper copy/move constructor deletion
 * 4. Test that both instances are truly singletons
 * 5. BONUS: Implement the generic Singleton template class
 * 6. BONUS: Make Logger thread-safe for concurrent logging (add mutex)
 * 
 * DISCUSSION QUESTIONS:
 * - What are the downsides of using Singletons?
 * - How would you test code that uses Singletons?
 * - What's the difference between Singleton and global variables?
 */
