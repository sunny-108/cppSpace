/**
 * STRATEGY PATTERN
 * 
 * Purpose: Define a family of algorithms, encapsulate each one, and make them 
 *          interchangeable. Strategy lets the algorithm vary independently from 
 *          clients that use it.
 * 
 * Use Cases:
 * - Sorting algorithms selection
 * - Payment processing methods
 * - Compression algorithms
 * - Validation strategies
 * - Navigation routes
 * - File export formats
 * 
 * Key Concepts:
 * - Strategy interface
 * - Concrete strategies
 * - Context class
 * - Runtime algorithm selection
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

// ===== EXERCISE 1: Payment Processing System =====

// Strategy Interface
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual void pay(double amount) = 0;
    virtual std::string getPaymentType() const = 0;
};

// TODO: Implement CreditCardPayment
class CreditCardPayment : public PaymentStrategy {
private:
    std::string cardNumber_;
    std::string cvv_;
    std::string expiryDate_;
    
public:
    // TODO: Implement constructor and pay() method
};

// TODO: Implement PayPalPayment
class PayPalPayment : public PaymentStrategy {
private:
    std::string email_;
    std::string password_;
    
public:
    // TODO: Implement constructor and pay() method
};

// TODO: Implement CryptoPayment
class CryptoPayment : public PaymentStrategy {
private:
    std::string walletAddress_;
    std::string cryptoType_;
    
public:
    // TODO: Implement constructor and pay() method
};

// Context
class ShoppingCart {
private:
    std::vector<std::pair<std::string, double>> items_;
    std::unique_ptr<PaymentStrategy> paymentStrategy_;
    
public:
    void addItem(const std::string& item, double price) {
        items_.push_back({item, price});
    }
    
    void setPaymentStrategy(std::unique_ptr<PaymentStrategy> strategy) {
        paymentStrategy_ = std::move(strategy);
    }
    
    void checkout() {
        double total = 0.0;
        std::cout << "\n=== Shopping Cart ===" << std::endl;
        for (const auto& [item, price] : items_) {
            std::cout << item << ": $" << price << std::endl;
            total += price;
        }
        std::cout << "Total: $" << total << std::endl;
        
        if (paymentStrategy_) {
            paymentStrategy_->pay(total);
        } else {
            std::cout << "Error: No payment method selected!" << std::endl;
        }
    }
};

// ===== EXERCISE 2: Sorting Strategy =====

template<typename T>
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<T>& data) = 0;
    virtual std::string getName() const = 0;
};

// TODO: Implement BubbleSort
template<typename T>
class BubbleSort : public SortStrategy<T> {
public:
    // TODO: Implement bubble sort algorithm
};

// TODO: Implement QuickSort
template<typename T>
class QuickSort : public SortStrategy<T> {
public:
    // TODO: Implement quick sort algorithm
};

// TODO: Implement MergeSort
template<typename T>
class MergeSort : public SortStrategy<T> {
public:
    // TODO: Implement merge sort algorithm
};

template<typename T>
class Sorter {
private:
    std::unique_ptr<SortStrategy<T>> strategy_;
    
public:
    void setStrategy(std::unique_ptr<SortStrategy<T>> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void sort(std::vector<T>& data) {
        if (strategy_) {
            std::cout << "Sorting with " << strategy_->getName() << "..." << std::endl;
            strategy_->sort(data);
        }
    }
};

// ===== EXERCISE 3: Compression Strategy =====

class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    virtual std::string compress(const std::string& data) = 0;
    virtual std::string decompress(const std::string& data) = 0;
    virtual std::string getAlgorithm() const = 0;
};

// TODO: Implement ZipCompression
class ZipCompression : public CompressionStrategy {
public:
    // TODO: Simulate ZIP compression/decompression
};

// TODO: Implement RarCompression
class RarCompression : public CompressionStrategy {
public:
    // TODO: Simulate RAR compression/decompression
};

// TODO: Implement GzipCompression
class GzipCompression : public CompressionStrategy {
public:
    // TODO: Simulate GZIP compression/decompression
};

class FileCompressor {
private:
    std::unique_ptr<CompressionStrategy> strategy_;
    
public:
    // TODO: Implement setStrategy(), compressFile(), decompressFile()
};

// ===== EXERCISE 4: Navigation Strategy =====

struct Location {
    double latitude;
    double longitude;
    
    Location(double lat, double lon) : latitude(lat), longitude(lon) {}
};

class RouteStrategy {
public:
    virtual ~RouteStrategy() = default;
    virtual void calculateRoute(const Location& start, const Location& end) = 0;
    virtual std::string getRouteType() const = 0;
};

// TODO: Implement WalkingRoute
class WalkingRoute : public RouteStrategy {
public:
    // TODO: Calculate walking route (shortest path)
};

// TODO: Implement DrivingRoute
class DrivingRoute : public RouteStrategy {
public:
    // TODO: Calculate driving route (fastest, considering traffic)
};

// TODO: Implement BicycleRoute
class BicycleRoute : public RouteStrategy {
public:
    // TODO: Calculate bicycle route (bike lanes preferred)
};

// TODO: Implement PublicTransportRoute
class PublicTransportRoute : public RouteStrategy {
public:
    // TODO: Calculate public transport route
};

class Navigator {
private:
    std::unique_ptr<RouteStrategy> strategy_;
    
public:
    // TODO: Implement setRouteStrategy() and navigate()
};

// ===== EXERCISE 5: Validation Strategy =====

class ValidationStrategy {
public:
    virtual ~ValidationStrategy() = default;
    virtual bool validate(const std::string& input) = 0;
    virtual std::string getErrorMessage() const = 0;
};

// TODO: Implement EmailValidator
class EmailValidator : public ValidationStrategy {
public:
    // TODO: Validate email format
};

// TODO: Implement PhoneValidator
class PhoneValidator : public ValidationStrategy {
public:
    // TODO: Validate phone number format
};

// TODO: Implement PasswordValidator
class PasswordValidator : public ValidationStrategy {
private:
    int minLength_;
    bool requireSpecialChar_;
    bool requireNumber_;
    
public:
    // TODO: Validate password strength
};

class InputValidator {
private:
    std::unique_ptr<ValidationStrategy> strategy_;
    
public:
    // TODO: Implement setStrategy() and validate()
};

// ===== EXERCISE 6: Export Strategy =====

struct Report {
    std::string title;
    std::string content;
    std::vector<std::pair<std::string, std::string>> data;
};

class ExportStrategy {
public:
    virtual ~ExportStrategy() = default;
    virtual std::string exportReport(const Report& report) = 0;
};

// TODO: Implement PDFExport, CSVExport, JSONExport, XMLExport

int main() {
    std::cout << "=== Strategy Pattern Exercise ===\n\n";
    
    std::cout << "--- Payment Strategy Test ---" << std::endl;
    // TODO: Test payment strategies
    // ShoppingCart cart;
    // cart.addItem("Laptop", 999.99);
    // cart.addItem("Mouse", 29.99);
    
    // cart.setPaymentStrategy(std::make_unique<CreditCardPayment>("1234-5678-9012-3456", "123", "12/25"));
    // cart.checkout();
    
    // cart.setPaymentStrategy(std::make_unique<PayPalPayment>("user@example.com", "password"));
    // cart.checkout();
    
    std::cout << "\n--- Sorting Strategy Test ---" << std::endl;
    // TODO: Test sorting strategies
    // std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    // Sorter<int> sorter;
    
    // sorter.setStrategy(std::make_unique<BubbleSort<int>>());
    // auto data1 = data;
    // sorter.sort(data1);
    
    // sorter.setStrategy(std::make_unique<QuickSort<int>>());
    // auto data2 = data;
    // sorter.sort(data2);
    
    std::cout << "\n--- Compression Strategy Test ---" << std::endl;
    // TODO: Test compression strategies
    // FileCompressor compressor;
    // std::string originalData = "This is some data to compress!";
    
    // compressor.setStrategy(std::make_unique<ZipCompression>());
    // auto compressed = compressor.compressFile(originalData);
    
    std::cout << "\n--- Navigation Strategy Test ---" << std::endl;
    // TODO: Test navigation strategies
    // Navigator navigator;
    // Location home(40.7128, -74.0060);
    // Location work(40.7589, -73.9851);
    
    // navigator.setRouteStrategy(std::make_unique<WalkingRoute>());
    // navigator.navigate(home, work);
    
    // navigator.setRouteStrategy(std::make_unique<DrivingRoute>());
    // navigator.navigate(home, work);
    
    std::cout << "\n--- Validation Strategy Test ---" << std::endl;
    // TODO: Test validation strategies
    // InputValidator validator;
    
    // validator.setStrategy(std::make_unique<EmailValidator>());
    // bool valid = validator.validate("user@example.com");
    
    std::cout << "\n--- Export Strategy Test ---" << std::endl;
    // TODO: Test export strategies
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete all payment strategy implementations
 * 2. Implement all sorting strategy algorithms
 * 3. Implement compression strategies (can be simulated)
 * 4. Implement navigation route strategies
 * 5. Implement validation strategies with proper regex/logic
 * 6. Implement export strategies for different formats
 * 7. BONUS: Add a strategy factory to create strategies by name
 * 8. BONUS: Implement strategy chaining (apply multiple strategies)
 * 9. BONUS: Add performance metrics to compare strategies
 * 
 * DISCUSSION QUESTIONS:
 * - When should you use Strategy vs inheritance?
 * - How does Strategy enable the Open/Closed Principle?
 * - What's the relationship between Strategy and Dependency Injection?
 * - How do you choose the right strategy at runtime?
 */
