# C++ Design Patterns - Complete Solutions

This file contains complete implementations of all design pattern exercises. Try to solve the exercises yourself before looking at these solutions!

## Table of Contents
1. [Singleton Pattern](#singleton-pattern)
2. [Factory Method Pattern](#factory-method-pattern)
3. [Abstract Factory Pattern](#abstract-factory-pattern)
4. [Builder Pattern](#builder-pattern)
5. [Prototype Pattern](#prototype-pattern)
6. [Observer Pattern](#observer-pattern)
7. [Strategy Pattern](#strategy-pattern)
8. [Decorator Pattern](#decorator-pattern)
9. [Adapter Pattern](#adapter-pattern)
10. [Command Pattern](#command-pattern)

---

## Singleton Pattern

```cpp
class Logger {
private:
    Logger() : logLevel_("INFO") {}
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    std::string logLevel_;
    mutable std::mutex mutex_;
    
public:
    static Logger& getInstance() {
        static Logger instance;  // C++11 magic statics guarantee thread-safety
        return instance;
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "[" << logLevel_ << "] " << message << std::endl;
    }
    
    void setLogLevel(const std::string& level) {
        std::lock_guard<std::mutex> lock(mutex_);
        logLevel_ = level;
    }
    
    std::string getLogLevel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return logLevel_;
    }
};

class DatabaseConnection {
private:
    DatabaseConnection() : connected_(false) {}
    
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    
    std::string connectionString_;
    bool connected_;
    
public:
    static DatabaseConnection& getInstance() {
        static DatabaseConnection instance;
        return instance;
    }
    
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
```

---

## Factory Method Pattern

```cpp
class PDFDocument : public Document {
public:
    void open() override {
        std::cout << "Opening PDF document" << std::endl;
    }
    
    void save() override {
        std::cout << "Saving PDF document" << std::endl;
    }
    
    std::string getType() const override {
        return "PDF";
    }
};

class PDFApplication : public Application {
public:
    std::unique_ptr<Document> createDocument() override {
        return std::make_unique<PDFDocument>();
    }
};

// Usage in main():
std::unique_ptr<Application> pdfApp = std::make_unique<PDFApplication>();
pdfApp->newDocument();
```

---

## Observer Pattern

```cpp
class NewsChannel : public Observer {
private:
    std::string channelName_;
    
public:
    NewsChannel(const std::string& name) : channelName_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "[" << channelName_ << "] Broadcasting: " << message << std::endl;
    }
};

class MobileApp : public Observer {
private:
    std::string appName_;
    
public:
    MobileApp(const std::string& name) : appName_(name) {}
    
    void update(const std::string& message) override {
        std::cout << "[" << appName_ << "] Push notification: " << message << std::endl;
    }
};

// Usage:
NewsAgency agency;
NewsChannel cnn("CNN");
MobileApp newsApp("NewsApp");

agency.attach(&cnn);
agency.attach(&newsApp);
agency.setNews("Breaking: Major event occurred!");
```

---

## Strategy Pattern

```cpp
class CreditCardPayment : public PaymentStrategy {
private:
    std::string cardNumber_;
    std::string cvv_;
    std::string expiryDate_;
    
public:
    CreditCardPayment(const std::string& cardNum, const std::string& cv, const std::string& expiry)
        : cardNumber_(cardNum), cvv_(cv), expiryDate_(expiry) {}
    
    void pay(double amount) override {
        std::cout << "Paid $" << amount << " using Credit Card ending in " 
                  << cardNumber_.substr(cardNumber_.length() - 4) << std::endl;
    }
    
    std::string getPaymentType() const override {
        return "Credit Card";
    }
};

class PayPalPayment : public PaymentStrategy {
private:
    std::string email_;
    std::string password_;
    
public:
    PayPalPayment(const std::string& email, const std::string& pass)
        : email_(email), password_(pass) {}
    
    void pay(double amount) override {
        std::cout << "Paid $" << amount << " using PayPal account: " << email_ << std::endl;
    }
    
    std::string getPaymentType() const override {
        return "PayPal";
    }
};

// Usage:
ShoppingCart cart;
cart.addItem("Laptop", 999.99);
cart.setPaymentStrategy(std::make_unique<CreditCardPayment>("1234-5678-9012-3456", "123", "12/25"));
cart.checkout();
```

---

## Decorator Pattern

```cpp
class Mocha : public CondimentDecorator {
public:
    Mocha(std::unique_ptr<Beverage> beverage) 
        : CondimentDecorator(std::move(beverage)) {}
    
    std::string getDescription() const override {
        return beverage_->getDescription() + ", Mocha";
    }
    
    double cost() const override {
        return beverage_->cost() + 0.20;
    }
};

class Whip : public CondimentDecorator {
public:
    Whip(std::unique_ptr<Beverage> beverage) 
        : CondimentDecorator(std::move(beverage)) {}
    
    std::string getDescription() const override {
        return beverage_->getDescription() + ", Whip";
    }
    
    double cost() const override {
        return beverage_->cost() + 0.10;
    }
};

// Usage:
auto beverage = std::make_unique<Espresso>();
beverage = std::make_unique<Mocha>(std::move(beverage));
beverage = std::make_unique<Whip>(std::move(beverage));
std::cout << beverage->getDescription() << " $" << beverage->cost() << std::endl;
// Output: Espresso, Mocha, Whip $2.29
```

---

## Command Pattern

```cpp
class InsertTextCommand : public Command {
private:
    TextDocument* document_;
    std::string text_;
    int position_;
    
public:
    InsertTextCommand(TextDocument* doc, const std::string& text, int pos)
        : document_(doc), text_(text), position_(pos) {}
    
    void execute() override {
        document_->insertText(text_, position_);
    }
    
    void undo() override {
        document_->deleteText(position_, text_.length());
    }
    
    std::string getDescription() const override {
        return "Insert '" + text_ + "' at position " + std::to_string(position_);
    }
};

void TextEditor::undo() {
    if (!undoStack_.empty()) {
        auto command = std::move(undoStack_.top());
        undoStack_.pop();
        command->undo();
        redoStack_.push(std::move(command));
    }
}

void TextEditor::redo() {
    if (!redoStack_.empty()) {
        auto command = std::move(redoStack_.top());
        redoStack_.pop();
        command->execute();
        undoStack_.push(std::move(command));
    }
}
```

---

## Additional Notes

### Compilation Tips
```bash
# Compile any exercise file:
g++ -std=c++17 -Wall -Wextra 01_singleton.cpp -o singleton
./singleton

# With optimization:
g++ -std=c++17 -O2 -Wall 01_singleton.cpp -o singleton

# With debugging symbols:
g++ -std=c++17 -g -Wall 01_singleton.cpp -o singleton
gdb ./singleton
```

### Common Patterns Summary

**When to use each pattern:**

- **Singleton**: Global state, single instance required (use sparingly!)
- **Factory Method**: Defer instantiation to subclasses
- **Abstract Factory**: Create families of related objects
- **Builder**: Construct complex objects step-by-step
- **Prototype**: Clone expensive objects
- **Observer**: One-to-many notifications
- **Strategy**: Interchangeable algorithms
- **Decorator**: Add responsibilities dynamically
- **Adapter**: Make incompatible interfaces work
- **Command**: Encapsulate requests, enable undo/redo

### Best Practices

1. **Prefer composition over inheritance**
2. **Program to interfaces, not implementations**
3. **Strive for loose coupling**
4. **Follow SOLID principles**
5. **Use smart pointers to manage lifetime**
6. **Make interfaces minimal and cohesive**
7. **Consider thread-safety when needed**

