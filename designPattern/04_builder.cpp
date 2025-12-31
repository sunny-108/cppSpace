/**
 * BUILDER PATTERN
 * 
 * Purpose: Separate the construction of a complex object from its representation,
 *          allowing the same construction process to create different representations.
 * 
 * Use Cases:
 * - Building complex objects (houses, cars, computers)
 * - Constructing database queries
 * - Creating HTTP requests
 * - Building meal orders
 * - Generating reports
 * 
 * Key Concepts:
 * - Builder interface
 * - Concrete builders
 * - Product
 * - Director (optional)
 * - Fluent interface (method chaining)
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ===== Product =====
class Computer {
private:
    std::string cpu_;
    std::string gpu_;
    int ram_;  // in GB
    int storage_;  // in GB
    std::string motherboard_;
    std::string powerSupply_;
    bool hasWifi_;
    bool hasRGB_;
    
public:
    // TODO: Add setters for all components
    
    void display() const {
        std::cout << "Computer Configuration:" << std::endl;
        std::cout << "  CPU: " << cpu_ << std::endl;
        std::cout << "  GPU: " << gpu_ << std::endl;
        std::cout << "  RAM: " << ram_ << "GB" << std::endl;
        std::cout << "  Storage: " << storage_ << "GB" << std::endl;
        std::cout << "  Motherboard: " << motherboard_ << std::endl;
        std::cout << "  Power Supply: " << powerSupply_ << std::endl;
        std::cout << "  WiFi: " << (hasWifi_ ? "Yes" : "No") << std::endl;
        std::cout << "  RGB: " << (hasRGB_ ? "Yes" : "No") << std::endl;
    }
};

// ===== Builder Interface =====
class ComputerBuilder {
protected:
    std::unique_ptr<Computer> computer_;
    
public:
    ComputerBuilder() : computer_(std::make_unique<Computer>()) {}
    virtual ~ComputerBuilder() = default;
    
    // TODO: Add virtual methods for building each component
    // virtual ComputerBuilder& buildCPU() = 0;
    // virtual ComputerBuilder& buildGPU() = 0;
    // ... etc
    
    std::unique_ptr<Computer> getResult() {
        return std::move(computer_);
    }
};

// TODO: Implement GamingComputerBuilder
class GamingComputerBuilder : public ComputerBuilder {
public:
    // TODO: Implement all build methods for a gaming PC
    // High-end CPU, powerful GPU, lots of RAM, RGB, etc.
};

// TODO: Implement OfficeComputerBuilder
class OfficeComputerBuilder : public ComputerBuilder {
public:
    // TODO: Implement all build methods for an office PC
    // Mid-range CPU, integrated GPU, moderate RAM, no RGB, etc.
};

// TODO: Implement BudgetComputerBuilder
class BudgetComputerBuilder : public ComputerBuilder {
public:
    // TODO: Implement all build methods for a budget PC
};

// ===== Director (Optional) =====
class ComputerDirector {
private:
    ComputerBuilder* builder_;
    
public:
    void setBuilder(ComputerBuilder* builder) {
        builder_ = builder;
    }
    
    // TODO: Implement construct() method that calls builder methods in order
};

// ===== EXERCISE 2: HTTP Request Builder =====

class HttpRequest {
private:
    std::string method_;
    std::string url_;
    std::vector<std::pair<std::string, std::string>> headers_;
    std::vector<std::pair<std::string, std::string>> queryParams_;
    std::string body_;
    int timeout_;
    
public:
    // TODO: Add setters and a display() method
};

// TODO: Implement HttpRequestBuilder with fluent interface
class HttpRequestBuilder {
private:
    std::unique_ptr<HttpRequest> request_;
    
public:
    HttpRequestBuilder() : request_(std::make_unique<HttpRequest>()) {}
    
    // TODO: Implement fluent methods:
    // HttpRequestBuilder& setMethod(const std::string& method);
    // HttpRequestBuilder& setUrl(const std::string& url);
    // HttpRequestBuilder& addHeader(const std::string& key, const std::string& value);
    // HttpRequestBuilder& addQueryParam(const std::string& key, const std::string& value);
    // HttpRequestBuilder& setBody(const std::string& body);
    // HttpRequestBuilder& setTimeout(int timeout);
    // std::unique_ptr<HttpRequest> build();
};

// ===== EXERCISE 3: Meal Builder =====

class Meal {
private:
    std::string mainCourse_;
    std::string sideDish_;
    std::string drink_;
    std::string dessert_;
    bool isTakeout_;
    double price_;
    
public:
    // TODO: Add setters and display() method
};

// TODO: Create MealBuilder interface
// TODO: Implement VegetarianMealBuilder
// TODO: Implement KidsMealBuilder
// TODO: Implement LuxuryMealBuilder

// ===== EXERCISE 4: SQL Query Builder =====

class SqlQuery {
private:
    std::string table_;
    std::vector<std::string> selectColumns_;
    std::vector<std::pair<std::string, std::string>> whereClauses_;
    std::vector<std::string> orderByColumns_;
    int limit_;
    
public:
    // TODO: Add setters and a toString() method that builds the SQL query
};

// TODO: Implement SqlQueryBuilder with fluent interface

int main() {
    std::cout << "=== Builder Pattern Exercise ===\n\n";
    
    std::cout << "--- Computer Builder Test ---" << std::endl;
    // TODO: Build a gaming computer
    // GamingComputerBuilder gamingBuilder;
    // gamingBuilder.buildCPU().buildGPU().buildRAM()...;
    // auto gamingPC = gamingBuilder.getResult();
    // gamingPC->display();
    
    // TODO: Use director to build an office computer
    // ComputerDirector director;
    // OfficeComputerBuilder officeBuilder;
    // director.setBuilder(&officeBuilder);
    // director.construct();
    // auto officePC = officeBuilder.getResult();
    // officePC->display();
    
    std::cout << "\n--- HTTP Request Builder Test ---" << std::endl;
    // TODO: Build an HTTP request with fluent interface
    // auto request = HttpRequestBuilder()
    //     .setMethod("POST")
    //     .setUrl("https://api.example.com/users")
    //     .addHeader("Content-Type", "application/json")
    //     .addQueryParam("limit", "10")
    //     .setBody("{\"name\": \"John\"}")
    //     .setTimeout(5000)
    //     .build();
    // request->display();
    
    std::cout << "\n--- Meal Builder Test ---" << std::endl;
    // TODO: Build different types of meals
    
    std::cout << "\n--- SQL Query Builder Test ---" << std::endl;
    // TODO: Build a SQL query
    // auto query = SqlQueryBuilder()
    //     .select({"id", "name", "email"})
    //     .from("users")
    //     .where("age", ">", "18")
    //     .where("status", "=", "active")
    //     .orderBy("name")
    //     .limit(10)
    //     .build();
    // std::cout << query->toString() << std::endl;
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete the Computer builder implementation with all concrete builders
 * 2. Implement the Director class
 * 3. Implement the HTTP Request builder with fluent interface
 * 4. Implement the Meal builder system
 * 5. Implement the SQL Query builder
 * 6. Add validation to prevent invalid configurations
 * 7. BONUS: Implement a reset() method to reuse builders
 * 8. BONUS: Add a clone() method to create variations of existing builds
 * 
 * DISCUSSION QUESTIONS:
 * - When would you use Builder instead of a constructor with many parameters?
 * - What are the benefits of the fluent interface?
 * - When is a Director useful vs building directly?
 * - How does Builder differ from Abstract Factory?
 */
