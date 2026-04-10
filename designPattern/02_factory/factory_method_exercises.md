# Factory Method - Exercises & MCQs

## Part 1: Multiple Choice Questions (MCQs)

**1. What category of design pattern does the Factory Method belong to?**
A) Structural
B) Behavioral
C) **Creational**
D) Architectural

**2. What is the primary purpose of the Factory Method pattern?**
A) To ensure a class only has one instance.
B) To define a family of algorithms and make them interchangeable.
C) To delegate the instantiation of objects to subclasses.
D) To convert the interface of a class into another interface clients expect.

**3. Which SOLID principle is most directly supported by the Factory Method pattern?**
A) Single Responsibility Principle
B) Open/Closed Principle
C) Liskov Substitution Principle
D) Interface Segregation Principle

**4. In C++, what is the recommended return type for a Factory Method to prevent memory leaks and ensure clear ownership?**
A) A raw pointer (e.g., `Product*`)
B) A reference (e.g., `Product&`)
C) A smart pointer (e.g., `std::unique_ptr<Product>`)
D) A void pointer (e.g., `void*`)

**5. What is the difference between a "Simple Factory" and the "Factory Method" pattern?**
A) They are exactly the same thing.
B) Simple Factory uses a single class with a switch statement; Factory Method uses inheritance and polymorphism.
C) Factory Method can only create one type of object; Simple Factory can create many.
D) Simple Factory is thread-safe, while Factory Method is not.

**6. In the Factory Method pattern, who is responsible for deciding which concrete product to instantiate?**
A) The Client code
B) The Concrete Creator (Subclass)
C) The Product Interface
D) The Base Creator class

**7. Can a Factory Method be declared as `static` in C++ if it needs to be overridden by subclasses?**
A) Yes, static methods can be overridden in C++.
B) No, static methods cannot be `virtual` and therefore cannot be overridden polymorphically.
C) Yes, but only if the return type is `void`.
D) No, because static methods cannot return pointers.

**8. What is a common drawback of the Factory Method pattern?**
A) It makes the code run significantly slower.
B) It can lead to an explosion of subclasses if you need a new creator for every new product.
C) It violates the Dependency Inversion Principle.
D) It forces you to use global variables.

**9. If a Base Creator class provides a default implementation for the factory method, what is this called?**
A) Abstract Factory
B) Default Factory Method
C) Singleton Factory
D) Prototype Factory

**10. When should you NOT use the Factory Method pattern?**
A) When you don't know beforehand the exact types of objects your code needs to work with.
B) When you want to provide users of your library a way to extend its internal components.
C) When object creation is simple, straightforward, and unlikely to change.
D) When you want to save system resources by reusing existing objects instead of rebuilding them.

### MCQ Answers

| Q  | Answer | Reason |
|----|--------|--------|
| 1  | **C** – Creational | Factory Method is concerned with object creation. |
| 2  | **C** – Delegate instantiation to subclasses | The concrete creator decides which product to instantiate. |
| 3  | **B** – Open/Closed Principle | New products are added by creating new subclasses, not modifying existing code. |
| 4  | **C** – `std::unique_ptr<Product>` | Smart pointers express clear ownership and prevent memory leaks. |
| 5  | **B** – Simple Factory uses a switch/if; Factory Method uses inheritance | Simple Factory centralises creation logic; Factory Method delegates it via polymorphism. |
| 6  | **B** – The Concrete Creator (Subclass) | Each concrete creator overrides the factory method to return its own product. |
| 7  | **B** – No, `static` cannot be `virtual` | Static methods are resolved at compile time and cannot be overridden polymorphically. |
| 8  | **B** – Subclass explosion | Each new product type may require a new concrete creator class. |
| 9  | **B** – Default Factory Method | The base creator provides a sensible default that subclasses may optionally override. |
| 10 | **C** – When creation is simple and unlikely to change | The added complexity of the pattern is not justified for trivial object creation. |

---

## Part 2: Practical Exercises

### Exercise 1: Code Review

Review the following C++ code implementing a document creator. Identify at least **3 major design or C++ specific flaws** and explain how to fix them.

```cpp
class Document {
public:
    virtual void print() = 0;
};

class PdfDocument : public Document {
public:
    void print() override { std::cout << "Printing PDF\n"; }
};

class WordDocument : public Document {
public:
    void print() override { std::cout << "Printing Word\n"; }
};

class Application {
public:
    Document* createDocument(std::string type) {
        if (type == "PDF") {
            return new PdfDocument();
        } else if (type == "Word") {
            return new WordDocument();
        }
        return nullptr;
    }
};
```

### Exercise 2: Debugging

The following code compiles but crashes or leaks memory at runtime. Find the bug and fix it using modern C++ practices.

```cpp
#include <iostream>

class Logger {
public:
    virtual void log(const std::string& msg) = 0;
};

class FileLogger : public Logger {
public:
    void log(const std::string& msg) override { std::cout << "File: " << msg << "\n"; }
    ~FileLogger() { std::cout << "Closing file...\n"; }
};

class LoggerFactory {
public:
    virtual Logger* createLogger() = 0;
};

class FileLoggerFactory : public LoggerFactory {
public:
    Logger* createLogger() override {
        return new FileLogger();
    }
};

int main() {
    LoggerFactory* factory = new FileLoggerFactory();
    Logger* logger = factory->createLogger();
    logger->log("System started.");
  
    delete logger;
    delete factory;
    return 0;
}
```

### Exercise 3: Implementation from Scratch

**Scenario:** You are writing a game where the player can encounter different types of enemies (`Orc`, `Goblin`).

1. Create an `Enemy` interface with a `attack()` method.
2. Create a `Spawner` base class with a pure virtual `createEnemy()` factory method.
3. Implement `OrcSpawner` and `GoblinSpawner`.
4. Write a `main` function that simulates a level where a random spawner is chosen to spawn an enemy, and the enemy attacks. Use `std::unique_ptr` for memory management.

### Exercise 4: Performance Optimization

**Scenario:** You have a `ReportFactory` that creates `AnnualReport` objects. Currently, every time `createReport()` is called, the `AnnualReport` constructor reads a massive 50MB configuration file from disk, making object creation extremely slow.

**Task:**
Modify the Factory Method implementation so that the configuration file is only read **once** by the Factory, and the parsed configuration data is passed to the `AnnualReport` objects upon creation.
*Hint: The Concrete Creator can hold state (like the cached config) and pass it to the products it creates.*

---

---

## Answers & Hints

### MCQ Answers

1. C | 2. C | 3. B | 4. C | 5. B | 6. B | 7. B | 8. B | 9. B | 10. C

### Practical Exercise Hints

**Ex 1 (Code Review):**

1. **Not a Factory Method:** This is a Simple Factory (parameterized creation). It violates the Open/Closed principle because adding a new document type requires modifying the `createDocument` method.
2. **Missing Virtual Destructor:** `Document` lacks a `virtual ~Document() = default;`. Deleting a derived class through a base pointer will cause undefined behavior.
3. **Raw Pointers:** Returning `Document*` transfers ownership ambiguously. It should return `std::unique_ptr<Document>`.

**Ex 2 (Debugging):**

* **Bug:** `Logger` and `LoggerFactory` are missing virtual destructors. When `delete logger;` is called, `~FileLogger()` is never executed, causing a resource leak (the file is never closed).
* **Fix:** Add `virtual ~Logger() = default;` and `virtual ~LoggerFactory() = default;`. Better yet, refactor to use `std::unique_ptr<Logger>` and `std::unique_ptr<LoggerFactory>` to avoid manual `delete` entirely.

**Ex 4 (Performance Optimization):**

```cpp
class AnnualReportFactory : public ReportFactory {
private:
    std::shared_ptr<ConfigData> cachedConfig;
public:
    AnnualReportFactory() {
        cachedConfig = std::make_shared<ConfigData>(loadMassiveFile());
    }
  
    std::unique_ptr<Report> createReport() override {
        // Pass the already loaded config to the product
        return std::make_unique<AnnualReport>(cachedConfig);
    }
};
```
