/**
 * COMMAND PATTERN
 * 
 * Purpose: Encapsulate a request as an object, thereby letting you parameterize
 *          clients with different requests, queue or log requests, and support
 *          undoable operations.
 * 
 * Use Cases:
 * - Undo/Redo functionality
 * - Macro recording
 * - Transaction systems
 * - Job queues
 * - GUI actions (menu items, buttons)
 * - Remote control systems
 * 
 * Key Concepts:
 * - Command interface
 * - Concrete commands
 * - Receiver (performs actual work)
 * - Invoker (executes commands)
 * - Command history
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stack>

// ===== EXERCISE 1: Text Editor Commands =====

// Receiver
class TextDocument {
private:
    std::string content_;
    
public:
    void insertText(const std::string& text, int position) {
        content_.insert(position, text);
        std::cout << "Inserted '" << text << "' at position " << position << std::endl;
    }
    
    void deleteText(int position, int length) {
        content_.erase(position, length);
        std::cout << "Deleted " << length << " characters from position " << position << std::endl;
    }
    
    std::string getContent() const {
        return content_;
    }
    
    void display() const {
        std::cout << "Document content: " << content_ << std::endl;
    }
};

// Command Interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
};

// TODO: Implement InsertTextCommand
class InsertTextCommand : public Command {
private:
    TextDocument* document_;
    std::string text_;
    int position_;
    
public:
    InsertTextCommand(TextDocument* doc, const std::string& text, int pos)
        : document_(doc), text_(text), position_(pos) {}
    
    // TODO: Implement execute() to insert text
    // TODO: Implement undo() to delete the inserted text
    // TODO: Implement getDescription()
};

// TODO: Implement DeleteTextCommand
class DeleteTextCommand : public Command {
private:
    TextDocument* document_;
    int position_;
    int length_;
    std::string deletedText_;
    
public:
    // TODO: Implement execute() to delete text (save deleted text for undo)
    // TODO: Implement undo() to reinsert deleted text
};

// Invoker with Undo/Redo
class TextEditor {
private:
    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    
public:
    void executeCommand(std::unique_ptr<Command> command) {
        command->execute();
        undoStack_.push(std::move(command));
        // Clear redo stack when new command is executed
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
    }
    
    // TODO: Implement undo()
    void undo() {
        // TODO: Pop from undo stack, call undo(), push to redo stack
    }
    
    // TODO: Implement redo()
    void redo() {
        // TODO: Pop from redo stack, call execute(), push to undo stack
    }
    
    bool canUndo() const {
        return !undoStack_.empty();
    }
    
    bool canRedo() const {
        return !redoStack_.empty();
    }
};

// ===== EXERCISE 2: Smart Home Remote Control =====

// Receivers
class Light {
private:
    std::string location_;
    int brightness_;
    
public:
    Light(const std::string& location) : location_(location), brightness_(0) {}
    
    void turnOn() {
        brightness_ = 100;
        std::cout << location_ << " light is ON" << std::endl;
    }
    
    void turnOff() {
        brightness_ = 0;
        std::cout << location_ << " light is OFF" << std::endl;
    }
    
    void dim(int level) {
        brightness_ = level;
        std::cout << location_ << " light dimmed to " << level << "%" << std::endl;
    }
    
    int getBrightness() const { return brightness_; }
};

class Thermostat {
private:
    int temperature_;
    
public:
    Thermostat() : temperature_(70) {}
    
    void setTemperature(int temp) {
        temperature_ = temp;
        std::cout << "Thermostat set to " << temp << "°F" << std::endl;
    }
    
    int getTemperature() const { return temperature_; }
};

class GarageDoor {
private:
    bool isOpen_;
    
public:
    GarageDoor() : isOpen_(false) {}
    
    void open() {
        isOpen_ = true;
        std::cout << "Garage door opening..." << std::endl;
    }
    
    void close() {
        isOpen_ = false;
        std::cout << "Garage door closing..." << std::endl;
    }
    
    bool getState() const { return isOpen_; }
};

// TODO: Implement LightOnCommand
class LightOnCommand : public Command {
private:
    Light* light_;
    int previousBrightness_;
    
public:
    // TODO: Turn light on, save previous brightness for undo
};

// TODO: Implement LightOffCommand
// TODO: Implement LightDimCommand
// TODO: Implement ThermostatCommand
// TODO: Implement GarageDoorOpenCommand
// TODO: Implement GarageDoorCloseCommand

// TODO: Implement MacroCommand (executes multiple commands)
class MacroCommand : public Command {
private:
    std::vector<std::unique_ptr<Command>> commands_;
    
public:
    void addCommand(std::unique_ptr<Command> command) {
        commands_.push_back(std::move(command));
    }
    
    // TODO: Execute all commands in sequence
    // TODO: Undo all commands in reverse order
};

// Remote Control
class RemoteControl {
private:
    std::vector<std::unique_ptr<Command>> commands_;
    
public:
    void setCommand(int slot, std::unique_ptr<Command> command) {
        if (slot >= commands_.size()) {
            commands_.resize(slot + 1);
        }
        commands_[slot] = std::move(command);
    }
    
    void pressButton(int slot) {
        if (slot < commands_.size() && commands_[slot]) {
            commands_[slot]->execute();
        }
    }
};

// ===== EXERCISE 3: Bank Transaction Commands =====

class BankAccount {
private:
    std::string accountNumber_;
    double balance_;
    
public:
    BankAccount(const std::string& accNum, double initialBalance)
        : accountNumber_(accNum), balance_(initialBalance) {}
    
    void deposit(double amount) {
        balance_ += amount;
        std::cout << "Deposited $" << amount << ". New balance: $" << balance_ << std::endl;
    }
    
    bool withdraw(double amount) {
        if (balance_ >= amount) {
            balance_ -= amount;
            std::cout << "Withdrew $" << amount << ". New balance: $" << balance_ << std::endl;
            return true;
        }
        std::cout << "Insufficient funds!" << std::endl;
        return false;
    }
    
    double getBalance() const { return balance_; }
};

// TODO: Implement DepositCommand (undoable)
// TODO: Implement WithdrawCommand (undoable)
// TODO: Implement TransferCommand (transfer between two accounts, undoable)

// ===== EXERCISE 4: Drawing Application Commands =====

struct Point {
    int x, y;
    Point(int x_, int y_) : x(x_), y(y_) {}
};

class Canvas {
private:
    std::vector<Point> points_;
    
public:
    void drawPoint(const Point& point) {
        points_.push_back(point);
        std::cout << "Drew point at (" << point.x << ", " << point.y << ")" << std::endl;
    }
    
    void removeLastPoint() {
        if (!points_.empty()) {
            points_.pop_back();
            std::cout << "Removed last point" << std::endl;
        }
    }
    
    void clear() {
        points_.clear();
        std::cout << "Canvas cleared" << std::endl;
    }
    
    const std::vector<Point>& getPoints() const { return points_; }
};

// TODO: Implement DrawPointCommand
// TODO: Implement DrawLineCommand
// TODO: Implement ClearCanvasCommand

int main() {
    std::cout << "=== Command Pattern Exercise ===\n\n";
    
    std::cout << "--- Text Editor Test ---" << std::endl;
    // TODO: Test text editor with undo/redo
    // TextDocument doc;
    // TextEditor editor;
    
    // editor.executeCommand(std::make_unique<InsertTextCommand>(&doc, "Hello", 0));
    // doc.display();
    
    // editor.executeCommand(std::make_unique<InsertTextCommand>(&doc, " World", 5));
    // doc.display();
    
    // editor.executeCommand(std::make_unique<DeleteTextCommand>(&doc, 5, 6));
    // doc.display();
    
    // editor.undo();
    // doc.display();
    
    // editor.undo();
    // doc.display();
    
    // editor.redo();
    // doc.display();
    
    std::cout << "\n--- Smart Home Remote Control Test ---" << std::endl;
    // TODO: Test remote control
    // RemoteControl remote;
    // Light livingRoomLight("Living Room");
    // Light bedroomLight("Bedroom");
    // GarageDoor garage;
    
    // remote.setCommand(0, std::make_unique<LightOnCommand>(&livingRoomLight));
    // remote.setCommand(1, std::make_unique<LightOffCommand>(&livingRoomLight));
    // remote.setCommand(2, std::make_unique<GarageDoorOpenCommand>(&garage));
    
    // remote.pressButton(0);  // Turn on living room light
    // remote.pressButton(2);  // Open garage door
    // remote.pressButton(1);  // Turn off living room light
    
    // TODO: Test macro command (e.g., "Good Night" - turns off all lights, closes garage)
    // auto goodNightMacro = std::make_unique<MacroCommand>();
    // goodNightMacro->addCommand(std::make_unique<LightOffCommand>(&livingRoomLight));
    // goodNightMacro->addCommand(std::make_unique<LightOffCommand>(&bedroomLight));
    // goodNightMacro->addCommand(std::make_unique<GarageDoorCloseCommand>(&garage));
    // goodNightMacro->execute();
    
    std::cout << "\n--- Bank Transaction Test ---" << std::endl;
    // TODO: Test bank transaction commands with undo
    // BankAccount account1("123456", 1000.0);
    // BankAccount account2("789012", 500.0);
    
    // auto deposit = std::make_unique<DepositCommand>(&account1, 200.0);
    // deposit->execute();
    // deposit->undo();
    
    std::cout << "\n--- Drawing Application Test ---" << std::endl;
    // TODO: Test drawing commands with undo
    // Canvas canvas;
    // auto draw1 = std::make_unique<DrawPointCommand>(&canvas, Point(10, 20));
    // auto draw2 = std::make_unique<DrawPointCommand>(&canvas, Point(30, 40));
    // draw1->execute();
    // draw2->execute();
    // draw2->undo();
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete text editor commands with undo/redo
 * 2. Implement smart home commands for all devices
 * 3. Implement macro commands for grouped actions
 * 4. Implement bank transaction commands
 * 5. Implement drawing application commands
 * 6. Add command history viewer
 * 7. BONUS: Implement command serialization (save/load command history)
 * 8. BONUS: Add command queueing system with priority
 * 9. BONUS: Implement transactional commands (all-or-nothing)
 * 
 * DISCUSSION QUESTIONS:
 * - How does Command pattern enable undo/redo?
 * - What's the relationship between Command and Memento patterns?
 * - How can Command pattern help with threading/async operations?
 * - What are the memory implications of storing command history?
 */
