# Command Design Pattern

## What is the Command Pattern?

The Command Pattern is a behavioral design pattern that **turns a request into a stand-alone object** containing all information about the request. This transformation lets you:
- Pass requests as method arguments
- Delay or queue a request's execution
- Support undoable operations
- Log requests
- Parameterize objects with operations

Think of it like a **remote control** - each button is a command object that knows how to execute a specific action (turn on TV, change channel, adjust volume) without the remote needing to know the details of how these actions work.

## Real-World Analogy

Imagine you're at a **restaurant**:
- You (the client) tell the **waiter** what you want
- The waiter writes down your order on a **slip of paper** (the command object)
- The waiter doesn't cook the food; they just pass the slip to the **kitchen** (the receiver)
- The kitchen prepares your order
- Later, if there's a problem, the waiter can use that slip to cancel your order (undo)

The order slip is the command - it encapsulates all the information needed to fulfill the request!

## Problem it Solves

**Without Command Pattern:**
```cpp
// Tightly coupled - Button knows about Light directly
class Button {
    Light* light;
public:
    void press() {
        light->turnOn();  // Button depends on Light implementation
    }
};
```

**Problems:**
- Button is tightly coupled to Light
- Hard to add undo functionality
- Can't queue or log operations
- Button can only control one type of device

**With Command Pattern:**
```cpp
// Loosely coupled - Button works with any command
class Button {
    Command* command;
public:
    void press() {
        command->execute();  // Button doesn't know what it does!
    }
};
```

## Structure

### High-Level Design Diagram

```
┌─────────────┐
│   Client    │  Creates ConcreteCommand and sets Receiver
└──────┬──────┘
       │
       │ creates
       ▼
┌─────────────────────┐
│     Invoker         │  Stores and executes commands
│  ┌────────────┐     │
│  │ command    │─────┼──────┐
│  └────────────┘     │      │
│  + execute()        │      │
└─────────────────────┘      │
                             │ uses
                             ▼
                   ┌──────────────────┐
                   │  <<interface>>   │
                   │     Command      │
                   │                  │
                   │  + execute()     │
                   │  + undo()        │
                   └────────┬─────────┘
                            │
                   ┌────────┴────────┐
                   │                 │
          ┌────────▼─────────┐ ┌────▼──────────────┐
          │ ConcreteCommand1 │ │ ConcreteCommand2  │
          │                  │ │                   │
          │ - receiver       │ │ - receiver        │
          │ + execute()      │ │ + execute()       │
          │ + undo()         │ │ + undo()          │
          └────────┬─────────┘ └────┬──────────────┘
                   │                │
                   │ uses           │ uses
                   ▼                ▼
              ┌──────────────────────────┐
              │      Receiver            │
              │                          │
              │  + action1()             │
              │  + action2()             │
              └──────────────────────────┘
```

### Components

1. **Command (Interface)**: Declares the interface for executing operations
2. **ConcreteCommand**: Implements Command and defines binding between Receiver and action
3. **Receiver**: Knows how to perform the actual work
4. **Invoker**: Asks the command to execute the request
5. **Client**: Creates ConcreteCommand and sets its Receiver

## Simple C++ Example: Smart Home System

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Command Interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
};

// Receiver 1: Light
class Light {
private:
    std::string location_;
    bool isOn_ = false;
    
public:
    Light(const std::string& location) : location_(location) {}
    
    void turnOn() {
        isOn_ = true;
        std::cout << location_ << " light is ON" << std::endl;
    }
    
    void turnOff() {
        isOn_ = false;
        std::cout << location_ << " light is OFF" << std::endl;
    }
};

// Receiver 2: Fan
class Fan {
private:
    std::string location_;
    int speed_ = 0;  // 0 = off, 1 = low, 2 = medium, 3 = high
    
public:
    Fan(const std::string& location) : location_(location) {}
    
    void setSpeed(int speed) {
        speed_ = speed;
        std::cout << location_ << " fan speed set to " << speed << std::endl;
    }
    
    int getSpeed() const { return speed_; }
};

// Concrete Command 1: Light On
class LightOnCommand : public Command {
private:
    Light* light_;
    
public:
    LightOnCommand(Light* light) : light_(light) {}
    
    void execute() override {
        light_->turnOn();
    }
    
    void undo() override {
        light_->turnOff();
    }
    
    std::string getName() const override {
        return "Light ON";
    }
};

// Concrete Command 2: Light Off
class LightOffCommand : public Command {
private:
    Light* light_;
    
public:
    LightOffCommand(Light* light) : light_(light) {}
    
    void execute() override {
        light_->turnOff();
    }
    
    void undo() override {
        light_->turnOn();
    }
    
    std::string getName() const override {
        return "Light OFF";
    }
};

// Concrete Command 3: Fan Speed
class FanSpeedCommand : public Command {
private:
    Fan* fan_;
    int speed_;
    int previousSpeed_ = 0;
    
public:
    FanSpeedCommand(Fan* fan, int speed) : fan_(fan), speed_(speed) {}
    
    void execute() override {
        previousSpeed_ = fan_->getSpeed();
        fan_->setSpeed(speed_);
    }
    
    void undo() override {
        fan_->setSpeed(previousSpeed_);
    }
    
    std::string getName() const override {
        return "Fan Speed " + std::to_string(speed_);
    }
};

// Invoker: Remote Control
class RemoteControl {
private:
    std::vector<std::unique_ptr<Command>> commandHistory_;
    
public:
    void executeCommand(std::unique_ptr<Command> command) {
        std::cout << "\n[Executing: " << command->getName() << "]" << std::endl;
        command->execute();
        commandHistory_.push_back(std::move(command));
    }
    
    void undoLastCommand() {
        if (commandHistory_.empty()) {
            std::cout << "\n[No commands to undo]" << std::endl;
            return;
        }
        
        auto& lastCommand = commandHistory_.back();
        std::cout << "\n[Undoing: " << lastCommand->getName() << "]" << std::endl;
        lastCommand->undo();
        commandHistory_.pop_back();
    }
    
    void showHistory() const {
        std::cout << "\n=== Command History ===" << std::endl;
        for (const auto& cmd : commandHistory_) {
            std::cout << "  - " << cmd->getName() << std::endl;
        }
    }
};

// Client Code
int main() {
    // Create receivers
    Light livingRoomLight("Living Room");
    Light bedroomLight("Bedroom");
    Fan livingRoomFan("Living Room");
    
    // Create invoker
    RemoteControl remote;
    
    // Execute commands
    remote.executeCommand(std::make_unique<LightOnCommand>(&livingRoomLight));
    remote.executeCommand(std::make_unique<LightOnCommand>(&bedroomLight));
    remote.executeCommand(std::make_unique<FanSpeedCommand>(&livingRoomFan, 2));
    
    remote.showHistory();
    
    // Undo operations
    remote.undoLastCommand();  // Undo fan speed
    remote.undoLastCommand();  // Undo bedroom light
    
    remote.showHistory();
    
    return 0;
}
```

### Output:
```
[Executing: Light ON]
Living Room light is ON

[Executing: Light ON]
Bedroom light is ON

[Executing: Fan Speed 2]
Living Room fan speed set to 2

=== Command History ===
  - Light ON
  - Light ON
  - Fan Speed 2

[Undoing: Fan Speed 2]
Living Room fan speed set to 0

[Undoing: Light ON]
Bedroom light is OFF

=== Command History ===
  - Light ON
```

## When to Use Command Pattern

✅ **Use it when you need to:**
- **Undo/Redo operations** - Store command history
- **Queue operations** - Execute commands at different times
- **Log operations** - Track all actions for auditing
- **Macro operations** - Combine multiple commands
- **Decouple sender and receiver** - Invoker doesn't know about receiver details
- **Parameterize objects with actions** - Configure objects with different operations

❌ **Don't use it when:**
- Simple direct method calls are sufficient
- No need for undo/redo or logging
- Performance is critical and object creation overhead matters

## Real-World Use Cases

1. **Text Editors**: Undo/redo, macros, clipboard operations
2. **GUI Frameworks**: Button clicks, menu selections, keyboard shortcuts
3. **Database Transactions**: Commit, rollback, transaction logging
4. **Game Development**: Action replay, AI commands, input handling
5. **Job Schedulers**: Queue tasks, retry failed operations
6. **Remote Controls**: Smart home devices, TV remotes
7. **Wizards/Multi-step Processes**: Navigate forward/backward through steps

## Advanced Example: Text Editor with Macro Support

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stack>

// Command Interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Receiver
class TextEditor {
private:
    std::string content_;
    
public:
    void insertText(const std::string& text, size_t position) {
        content_.insert(position, text);
    }
    
    void deleteText(size_t position, size_t length) {
        content_.erase(position, length);
    }
    
    std::string getText() const { return content_; }
    
    void display() const {
        std::cout << "Content: \"" << content_ << "\"" << std::endl;
    }
};

// Insert Command
class InsertCommand : public Command {
private:
    TextEditor* editor_;
    std::string text_;
    size_t position_;
    
public:
    InsertCommand(TextEditor* editor, const std::string& text, size_t pos)
        : editor_(editor), text_(text), position_(pos) {}
    
    void execute() override {
        editor_->insertText(text_, position_);
    }
    
    void undo() override {
        editor_->deleteText(position_, text_.length());
    }
};

// Delete Command
class DeleteCommand : public Command {
private:
    TextEditor* editor_;
    size_t position_;
    size_t length_;
    std::string deletedText_;
    
public:
    DeleteCommand(TextEditor* editor, size_t pos, size_t len)
        : editor_(editor), position_(pos), length_(len) {}
    
    void execute() override {
        deletedText_ = editor_->getText().substr(position_, length_);
        editor_->deleteText(position_, length_);
    }
    
    void undo() override {
        editor_->insertText(deletedText_, position_);
    }
};

// Macro Command (Composite Command)
class MacroCommand : public Command {
private:
    std::vector<std::unique_ptr<Command>> commands_;
    
public:
    void addCommand(std::unique_ptr<Command> cmd) {
        commands_.push_back(std::move(cmd));
    }
    
    void execute() override {
        for (auto& cmd : commands_) {
            cmd->execute();
        }
    }
    
    void undo() override {
        // Undo in reverse order
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
    }
};

// Command Manager (Invoker)
class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    
public:
    void executeCommand(std::unique_ptr<Command> cmd) {
        cmd->execute();
        undoStack_.push(std::move(cmd));
        
        // Clear redo stack when new command is executed
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
    }
    
    void undo() {
        if (undoStack_.empty()) {
            std::cout << "Nothing to undo" << std::endl;
            return;
        }
        
        auto cmd = std::move(undoStack_.top());
        undoStack_.pop();
        
        cmd->undo();
        redoStack_.push(std::move(cmd));
    }
    
    void redo() {
        if (redoStack_.empty()) {
            std::cout << "Nothing to redo" << std::endl;
            return;
        }
        
        auto cmd = std::move(redoStack_.top());
        redoStack_.pop();
        
        cmd->execute();
        undoStack_.push(std::move(cmd));
    }
};

int main() {
    TextEditor editor;
    CommandManager manager;
    
    std::cout << "=== Text Editor Demo ===" << std::endl;
    editor.display();
    
    // Execute commands
    std::cout << "\n1. Insert 'Hello'" << std::endl;
    manager.executeCommand(std::make_unique<InsertCommand>(&editor, "Hello", 0));
    editor.display();
    
    std::cout << "\n2. Insert ' World'" << std::endl;
    manager.executeCommand(std::make_unique<InsertCommand>(&editor, " World", 5));
    editor.display();
    
    std::cout << "\n3. Insert '!'" << std::endl;
    manager.executeCommand(std::make_unique<InsertCommand>(&editor, "!", 11));
    editor.display();
    
    // Undo
    std::cout << "\n4. Undo" << std::endl;
    manager.undo();
    editor.display();
    
    std::cout << "\n5. Undo" << std::endl;
    manager.undo();
    editor.display();
    
    // Redo
    std::cout << "\n6. Redo" << std::endl;
    manager.redo();
    editor.display();
    
    // Macro command
    std::cout << "\n7. Execute Macro (Insert '!!!')" << std::endl;
    auto macro = std::make_unique<MacroCommand>();
    macro->addCommand(std::make_unique<InsertCommand>(&editor, "!", 11));
    macro->addCommand(std::make_unique<InsertCommand>(&editor, "!", 12));
    macro->addCommand(std::make_unique<InsertCommand>(&editor, "!", 13));
    manager.executeCommand(std::move(macro));
    editor.display();
    
    std::cout << "\n8. Undo Macro" << std::endl;
    manager.undo();
    editor.display();
    
    return 0;
}
```

## Key Benefits

1. **Decoupling**: Separates the object that invokes the operation from the one that knows how to perform it
2. **Extensibility**: Easy to add new commands without changing existing code (Open/Closed Principle)
3. **Undo/Redo**: Natural support for reversible operations
4. **Composite Commands**: Can combine simple commands into complex ones (macros)
5. **Logging & Auditing**: Track all operations for debugging or compliance
6. **Delayed Execution**: Queue commands for later execution
7. **Single Responsibility**: Each command handles one specific operation

## Comparison with Other Patterns

| Pattern | Purpose | Key Difference |
|---------|---------|----------------|
| **Command** | Encapsulate request as object | Focus on parameterizing actions, undo/redo |
| **Strategy** | Encapsulate algorithm | Focus on interchangeable algorithms |
| **Observer** | Notify dependents of changes | Focus on one-to-many communication |
| **Memento** | Capture object state | Focus on saving/restoring state, not actions |

## Common Pitfalls

1. **Over-engineering**: Don't use Command pattern for simple one-off operations
2. **Memory overhead**: Each command is an object - can be expensive for high-frequency operations
3. **Complex undo logic**: Some operations are hard to undo (e.g., sending an email)
4. **State management**: Commands need to store enough state for undo operations

## Summary

The Command Pattern is like creating a **to-do list for your code**. Each command is a task that knows:
- What needs to be done
- How to do it
- How to undo it

This makes your code more **flexible, testable, and maintainable**, especially when you need features like undo/redo, logging, or queuing operations.

---

## Further Reading

- Design Patterns: Elements of Reusable Object-Oriented Software (GoF)
- Head First Design Patterns
- Refactoring.Guru - Command Pattern

