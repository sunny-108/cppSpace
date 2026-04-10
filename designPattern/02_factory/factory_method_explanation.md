# Factory Method Design Pattern

## What is the Factory Method? (Simple Explanation)

Imagine you own a logistics company. Initially, your company only handles road transportation, so all your software is built around a `Truck` class. 

Later, your company becomes highly successful and expands to sea transportation. Now you need to add a `Ship` class. If your code is filled with `new Truck()` everywhere, adding `Ship` will require changing the entire codebase. 

The **Factory Method** solves this by replacing direct object construction calls (using `new`) with calls to a special *factory method*. 

Instead of the main code saying "Create a Truck", it says "Create a Transport vehicle". The exact type of vehicle created depends on which specific department (subclass) is handling the request: the `RoadLogistics` department will create a `Truck`, and the `SeaLogistics` department will create a `Ship`.

### Key Takeaways:
1. **Delegation:** It delegates the responsibility of initializing a class from the client to a specific factory subclass.
2. **Open/Closed Principle:** You can introduce new types of products into the program without breaking existing client code.
3. **Loose Coupling:** The creator class doesn't need to know the exact class of the object it creates, only the interface it implements.

---

## C++ Example

Here is how you would implement the logistics example in modern C++:

```cpp
#include <iostream>
#include <memory>
#include <string>

// 1. The Product Interface
class Transport {
public:
    virtual ~Transport() = default;
    virtual void deliver() const = 0;
};

// 2. Concrete Products
class Truck : public Transport {
public:
    void deliver() const override {
        std::cout << "Delivering cargo by land in a box.\n";
    }
};

class Ship : public Transport {
public:
    void deliver() const override {
        std::cout << "Delivering cargo by sea in a container.\n";
    }
};

// 3. The Creator (Base Factory)
class Logistics {
public:
    virtual ~Logistics() = default;

    // The Factory Method
    virtual std::unique_ptr<Transport> createTransport() const = 0;

    // Core business logic that relies on the factory method
    void planDelivery() const {
        std::cout << "Logistics: Planning delivery...\n";
        std::unique_ptr<Transport> transport = createTransport();
        transport->deliver();
    }
};

// 4. Concrete Creators
class RoadLogistics : public Logistics {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Truck>();
    }
};

class SeaLogistics : public Logistics {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Ship>();
    }
};

// Client Code
int main() {
    std::unique_ptr<Logistics> roadLogistics = std::make_unique<RoadLogistics>();
    roadLogistics->planDelivery(); 
    // Output: Logistics: Planning delivery...
    //         Delivering cargo by land in a box.

    std::unique_ptr<Logistics> seaLogistics = std::make_unique<SeaLogistics>();
    seaLogistics->planDelivery();
    // Output: Logistics: Planning delivery...
    //         Delivering cargo by sea in a container.

    return 0;
}
```

---

## Programming Assignments

To master the Factory Method, try implementing the following real-world scenarios.

### Assignment 1: Cross-Platform UI Framework (Medium)
**Scenario:** You are building a cross-platform GUI library. You need to render buttons, but a Windows button looks and behaves differently than a macOS button or a Web HTML button.
**Task:**
1. Create a `Button` interface with a `render()` and `onClick()` method.
2. Create concrete products: `WindowsButton`, `MacButton`, and `WebButton`.
3. Create a `Dialog` base class (the Creator) with a factory method `createButton()`, and a core method `renderWindow()` that calls `createButton()` and then renders it.
4. Create concrete creators: `WindowsDialog`, `MacDialog`, and `WebDialog`.
5. Write a `main` function that reads an environment variable or config string (e.g., `"OS=Windows"`) and instantiates the correct Dialog type.

### Assignment 2: Database Connection Pool (Advanced)
**Scenario:** Your backend server needs to connect to different types of databases (MySQL, PostgreSQL, MongoDB) depending on the microservice configuration. Creating connections is expensive, so you need a connection pool.
**Task:**
1. Create an `IDBConnection` interface with methods `connect()`, `query(string)`, and `disconnect()`.
2. Implement concrete connections: `MySQLConnection`, `PostgresConnection`.
3. Create a `DatabaseFactory` base class with a pure virtual `createConnection()` method.
4. Implement `MySQLFactory` and `PostgresFactory`.
5. **Advanced Twist:** Implement a `ConnectionPool` class that takes a `DatabaseFactory` in its constructor. The pool should use the factory to pre-instantiate 5 connections and store them in a thread-safe queue. When a client requests a connection, it pops one from the queue. When done, it pushes it back.
6. Ensure your implementation is thread-safe using `std::mutex` and `std::condition_variable`.
