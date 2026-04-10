# Design Pattern Interview Questions

Common interview questions and answers about design patterns in C++.

## General Questions

### Q1: What is a design pattern?
**Answer:** A design pattern is a reusable solution to a commonly occurring problem in software design. It's a template or blueprint for how to solve a problem that can be used in many different situations. Design patterns are best practices formalized by experienced software engineers.

**Key points:**
- Not finished code, but a description/template
- Proven solutions to recurring problems
- Improve code readability and maintainability
- Facilitate communication among developers

---

### Q2: What are the three main categories of design patterns?
**Answer:**

1. **Creational Patterns**: Deal with object creation mechanisms
   - Singleton, Factory Method, Abstract Factory, Builder, Prototype

2. **Structural Patterns**: Deal with object composition and relationships
   - Adapter, Bridge, Composite, Decorator, Facade, Proxy, Flyweight

3. **Behavioral Patterns**: Deal with object interaction and responsibility
   - Observer, Strategy, Command, State, Template Method, Iterator, Mediator, etc.

---

### Q3: What are SOLID principles and how do they relate to design patterns?
**Answer:**

**SOLID Principles:**
- **S** - Single Responsibility Principle
- **O** - Open/Closed Principle
- **L** - Liskov Substitution Principle
- **I** - Interface Segregation Principle
- **D** - Dependency Inversion Principle

**Relationship to patterns:**
- Strategy pattern follows Open/Closed (open for extension, closed for modification)
- Factory patterns follow Dependency Inversion (depend on abstractions)
- Decorator follows Single Responsibility (each decorator has one purpose)
- Observer follows Open/Closed (add observers without changing subject)

---

## Creational Patterns

### Q4: When should you use Singleton? What are its drawbacks?
**Answer:**

**Use when:**
- Exactly one instance is needed (logging, configuration, connection pools)
- Global access point is required
- Instance creation is expensive

**Drawbacks:**
- Global state (makes testing difficult)
- Violates Single Responsibility Principle
- Tight coupling
- Difficult to subclass
- Thread-safety concerns
- Hidden dependencies

**Better alternatives:**
- Dependency injection
- Service locator pattern
- Pass instances explicitly

---

### Q5: Explain the difference between Factory Method and Abstract Factory.
**Answer:**

**Factory Method:**
- Creates **one** type of product
- Uses inheritance (subclasses decide what to create)
- One factory method per factory class
- Example: `DocumentFactory.createDocument()` returns one document

**Abstract Factory:**
- Creates **families** of related products
- Uses composition (factory object creates multiple products)
- Multiple factory methods in one factory
- Example: `GUIFactory` creates Button, Checkbox, TextField together

```cpp
// Factory Method
class Application {
    virtual Document* createDocument() = 0;  // One product
};

// Abstract Factory
class GUIFactory {
    virtual Button* createButton() = 0;      // Multiple products
    virtual Checkbox* createCheckbox() = 0;
    virtual TextField* createTextField() = 0;
};
```

---

### Q6: When would you use Builder pattern over a constructor?
**Answer:**

**Use Builder when:**
- Object has many optional parameters (avoids telescoping constructors)
- Construction requires multiple steps
- Need different representations of the same object
- Want immutable objects with complex construction

**Example scenario:**
```cpp
// Bad: Telescoping constructor
Computer(cpu, gpu, ram, storage, motherboard, psu, wifi, rgb);

// Good: Builder
Computer computer = ComputerBuilder()
    .setCPU("Intel i9")
    .setRAM(32)
    .setStorage(1000)
    .build();
```

**Advantages:**
- More readable
- Optional parameters handled elegantly
- Validates before construction
- Fluent interface

---

## Structural Patterns

### Q7: Explain the difference between Decorator and Adapter patterns.
**Answer:**

**Decorator:**
- **Purpose**: Add responsibilities to objects dynamically
- **Changes**: Behavior/responsibilities
- **Interface**: Keeps same interface
- **Example**: Adding milk, mocha to coffee

**Adapter:**
- **Purpose**: Make incompatible interfaces work together
- **Changes**: Interface
- **Interface**: Converts one interface to another
- **Example**: Making MP4Player work with MediaPlayer interface

```cpp
// Decorator: Same interface, adds behavior
Beverage* coffee = new Espresso();
coffee = new Mocha(coffee);  // Still a Beverage

// Adapter: Different interface, makes compatible
MediaPlayer* player = new MP4Adapter(new MP4Player());
player->play("video.mp4");  // MP4Player now works as MediaPlayer
```

---

### Q8: When would you choose Decorator over inheritance?
**Answer:**

**Choose Decorator when:**
- Need to add responsibilities at runtime
- Multiple optional features can be combined
- Class is sealed/final
- Avoid class explosion (too many subclasses)

**Example:**
```cpp
// Inheritance approach: Need 8 classes for all combinations!
class Coffee {}
class CoffeeWithMilk : Coffee {}
class CoffeeWithMocha : Coffee {}
class CoffeeWithMilkAndMocha : Coffee {}
// ... etc

// Decorator approach: Just 3 decorator classes!
Coffee* drink = new Coffee();
drink = new Milk(drink);
drink = new Mocha(drink);  // Any combination possible
```

---

## Behavioral Patterns

### Q9: Explain Observer pattern and provide a real-world example.
**Answer:**

**Observer Pattern:**
- Defines one-to-many dependency between objects
- When subject changes state, all observers are notified
- Implements publish-subscribe mechanism

**Real-world example: Stock Market**
```cpp
class StockMarket {  // Subject
    vector<Observer*> observers;
    void notify() {
        for (auto obs : observers)
            obs->update(stockPrice);
    }
};

class Investor : public Observer {  // Observer
    void update(double price) {
        if (price < targetPrice) buy();
    }
};
```

**Use cases:**
- Event handling systems
- MVC architecture (model notifies views)
- Real-time data feeds
- Notification systems

---

### Q10: What's the difference between Strategy and State patterns?
**Answer:**

**Strategy Pattern:**
- **Purpose**: Define family of interchangeable algorithms
- **Who decides**: Client chooses strategy
- **Changes**: Behavior/algorithm changes
- **Awareness**: Strategies are independent

**State Pattern:**
- **Purpose**: Object behavior changes based on internal state
- **Who decides**: Object itself changes state
- **Changes**: State transitions automatically
- **Awareness**: States may know about each other

```cpp
// Strategy: Client decides
PaymentProcessor* processor = new CreditCardPayment();
cart.setPaymentStrategy(processor);  // Client chooses

// State: Object decides based on internal state
TCPConnection conn;
conn.open();   // Changes from Closed to Open state
conn.send();   // Behavior depends on current state
```

---

### Q11: How does Command pattern enable Undo/Redo functionality?
**Answer:**

**Key concepts:**
1. Encapsulate requests as objects
2. Store command history in stack
3. Each command implements execute() and undo()

**Implementation:**
```cpp
class Command {
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class TextEditor {
    stack<Command*> undoStack;
    stack<Command*> redoStack;
    
    void executeCommand(Command* cmd) {
        cmd->execute();
        undoStack.push(cmd);
        clearRedoStack();  // New command clears redo
    }
    
    void undo() {
        Command* cmd = undoStack.top();
        undoStack.pop();
        cmd->undo();
        redoStack.push(cmd);
    }
    
    void redo() {
        Command* cmd = redoStack.top();
        redoStack.pop();
        cmd->execute();
        undoStack.push(cmd);
    }
};
```

**Benefits:**
- Complete history tracking
- Macro commands (group operations)
- Transaction rollback
- Audit trails

---

## Advanced Questions

### Q12: How do you make Singleton thread-safe in C++?
**Answer:**

**C++11 and later (Recommended):**
```cpp
class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Magic statics (thread-safe)
        return instance;
    }
private:
    Singleton() {}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};
```

**Pre-C++11 (Double-Checked Locking):**
```cpp
class Singleton {
private:
    static Singleton* instance;
    static mutex mtx;
    Singleton() {}
public:
    static Singleton* getInstance() {
        if (instance == nullptr) {
            lock_guard<mutex> lock(mtx);
            if (instance == nullptr) {
                instance = new Singleton();
            }
        }
        return instance;
    }
};
```

**C++11 magic statics** guarantee thread-safe initialization, making it the preferred approach.

---

### Q13: What's the difference between Proxy and Decorator?
**Answer:**

**Proxy:**
- **Purpose**: Control access to an object
- **Creates**: May create the real object lazily
- **Relationship**: Same interface as real object
- **Focus**: Access control, lazy loading, caching

**Decorator:**
- **Purpose**: Add responsibilities
- **Creates**: Wraps existing object
- **Relationship**: Same interface, adds functionality
- **Focus**: Enhancement, feature addition

**Examples:**

```cpp
// Proxy: Controls access
class ImageProxy : public Image {
    RealImage* realImage = nullptr;
    void display() {
        if (!realImage) 
            realImage = new RealImage();  // Lazy load
        realImage->display();
    }
};

// Decorator: Adds functionality
class BorderedImage : public Image {
    Image* image;
    void display() {
        drawBorder();
        image->display();  // Enhanced behavior
        drawBorder();
    }
};
```

---

### Q14: How do patterns work together? Give an example.
**Answer:**

Patterns often combine to solve complex problems:

**Example: MVC Architecture**
- **Observer**: Model notifies Views of changes
- **Strategy**: Different rendering strategies for Views
- **Composite**: View hierarchy (container views)
- **Factory**: Creating appropriate Controllers

**Example: GUI Framework**
```cpp
// Abstract Factory: Creates widget families
GUIFactory* factory = new WindowsFactory();

// Builder: Constructs complex windows
WindowBuilder builder;
Window* win = builder
    .setTitle("App")
    .setSize(800, 600)
    .addButton(factory->createButton())  // Factory
    .addMenu(factory->createMenu())
    .build();

// Decorator: Add features to widgets
Widget* button = factory->createButton();
button = new TooltipDecorator(button);
button = new ShadowDecorator(button);

// Observer: Handle events
button->attach(new ClickHandler());  // Observer

// Command: Button actions
button->setCommand(new SaveCommand());  // Command
```

---

### Q15: What are anti-patterns? Give examples.
**Answer:**

**Anti-patterns** are common bad solutions to recurring problems.

**Examples:**

1. **God Object**: Class that does everything
   - Violates Single Responsibility
   - Hard to maintain and test

2. **Spaghetti Code**: Tangled, unstructured code
   - No clear flow
   - Difficult to follow

3. **Golden Hammer**: Using one solution for everything
   - "If all you have is a hammer, everything looks like a nail"
   - Over-using patterns (e.g., Singleton everywhere)

4. **Premature Optimization**
   - Optimizing before profiling
   - "Root of all evil" - Donald Knuth

5. **Copy-Paste Programming**
   - Duplicated code everywhere
   - Violates DRY principle

6. **Magic Numbers**: Hard-coded values
   - Use constants instead

**How to avoid:**
- Follow SOLID principles
- Code reviews
- Refactoring
- Use patterns appropriately (not everywhere!)

---

## Coding Questions

### Q16: Implement a thread-safe Singleton with lazy initialization.
See Q12 for the answer.

---

### Q17: Implement Observer pattern for a simple event system.
See the Observer exercise file (06_observer.cpp) for complete implementation.

---

### Q18: Design a payment processing system using Strategy pattern.
See the Strategy exercise file (07_strategy.cpp) for complete implementation.

---

## System Design Questions

### Q19: Design a logger system. Which patterns would you use?
**Answer:**

**Patterns to use:**
1. **Singleton**: Single logger instance
2. **Strategy**: Different log output strategies (file, console, database)
3. **Decorator**: Add features (timestamps, formatting, filtering)
4. **Observer**: Multiple log handlers
5. **Factory**: Create appropriate loggers

**Design:**
```cpp
class Logger {  // Singleton
    static Logger& getInstance();
    
    void setStrategy(LogStrategy* strategy);  // Strategy
    void addHandler(LogHandler* handler);     // Observer
    
    void log(LogLevel level, const string& message) {
        auto entry = createLogEntry(level, message);  // Factory
        entry = decorator->decorate(entry);           // Decorator
        notifyHandlers(entry);                        // Observer
    }
};
```

---

### Q20: Design a document editor with undo/redo. Which patterns would you use?
**Answer:**

**Patterns to use:**
1. **Command**: Encapsulate all operations
2. **Memento**: Save document state
3. **Composite**: Document structure (sections, paragraphs)
4. **Strategy**: Different save formats
5. **Observer**: Update UI when document changes

**Design:**
```cpp
class DocumentEditor {
    Document* document;              // Composite
    CommandManager* cmdManager;      // Command pattern
    vector<Observer*> observers;     // Observer
    
    void execute(Command* cmd) {
        cmd->execute(document);
        cmdManager->addToHistory(cmd);
        notifyObservers();
    }
    
    void undo() { cmdManager->undo(); }
    void redo() { cmdManager->redo(); }
    
    void save(string format) {
        SaveStrategy* strategy = createStrategy(format);  // Strategy
        strategy->save(document);
    }
};
```

---

This covers the most important design pattern interview questions. Practice implementing these patterns and understand when to apply each one!
