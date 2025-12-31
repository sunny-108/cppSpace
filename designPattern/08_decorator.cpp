/**
 * DECORATOR PATTERN
 * 
 * Purpose: Attach additional responsibilities to an object dynamically.
 *          Decorators provide a flexible alternative to subclassing for 
 *          extending functionality.
 * 
 * Use Cases:
 * - Adding features to GUI components
 * - Stream processing (input/output streams)
 * - Adding toppings to food items
 * - Text formatting
 * - Caching layers
 * 
 * Key Concepts:
 * - Component interface
 * - Concrete component
 * - Decorator base class
 * - Concrete decorators
 * - Recursive composition
 */

#include <iostream>
#include <memory>
#include <string>

// ===== EXERCISE 1: Coffee Shop System =====

// Component Interface
class Beverage {
public:
    virtual ~Beverage() = default;
    virtual std::string getDescription() const = 0;
    virtual double cost() const = 0;
};

// Concrete Components
class Espresso : public Beverage {
public:
    std::string getDescription() const override {
        return "Espresso";
    }
    
    double cost() const override {
        return 1.99;
    }
};

// TODO: Implement HouseBlend
class HouseBlend : public Beverage {
public:
    // TODO: Implement getDescription() and cost()
};

// TODO: Implement DarkRoast
class DarkRoast : public Beverage {
public:
    // TODO: Implement getDescription() and cost()
};

// Decorator Base Class
class CondimentDecorator : public Beverage {
protected:
    std::unique_ptr<Beverage> beverage_;
    
public:
    CondimentDecorator(std::unique_ptr<Beverage> beverage) 
        : beverage_(std::move(beverage)) {}
};

// TODO: Implement Milk decorator
class Milk : public CondimentDecorator {
public:
    Milk(std::unique_ptr<Beverage> beverage) 
        : CondimentDecorator(std::move(beverage)) {}
    
    // TODO: Implement getDescription() that adds "Milk" to the description
    // TODO: Implement cost() that adds milk cost to the base cost
};

// TODO: Implement Mocha decorator
class Mocha : public CondimentDecorator {
public:
    // TODO: Add mocha to description and cost
};

// TODO: Implement Whip decorator
class Whip : public CondimentDecorator {
public:
    // TODO: Add whipped cream to description and cost
};

// TODO: Implement Soy decorator
class Soy : public CondimentDecorator {
public:
    // TODO: Add soy milk to description and cost
};

// ===== EXERCISE 2: Text Editor Features =====

class TextComponent {
public:
    virtual ~TextComponent() = default;
    virtual std::string render() const = 0;
};

class PlainText : public TextComponent {
private:
    std::string text_;
    
public:
    PlainText(const std::string& text) : text_(text) {}
    
    std::string render() const override {
        return text_;
    }
};

class TextDecorator : public TextComponent {
protected:
    std::unique_ptr<TextComponent> component_;
    
public:
    TextDecorator(std::unique_ptr<TextComponent> component) 
        : component_(std::move(component)) {}
};

// TODO: Implement BoldDecorator (wraps text in <b></b>)
class BoldDecorator : public TextDecorator {
public:
    // TODO: Add bold formatting
};

// TODO: Implement ItalicDecorator (wraps text in <i></i>)
class ItalicDecorator : public TextDecorator {
public:
    // TODO: Add italic formatting
};

// TODO: Implement UnderlineDecorator
class UnderlineDecorator : public TextDecorator {
public:
    // TODO: Add underline formatting
};

// TODO: Implement ColorDecorator (wraps in span with color)
class ColorDecorator : public TextDecorator {
private:
    std::string color_;
    
public:
    // TODO: Add color styling
};

// ===== EXERCISE 3: Data Stream Processing =====

class DataStream {
public:
    virtual ~DataStream() = default;
    virtual std::string read() = 0;
    virtual void write(const std::string& data) = 0;
};

class FileStream : public DataStream {
private:
    std::string filename_;
    std::string buffer_;
    
public:
    FileStream(const std::string& filename) : filename_(filename) {}
    
    std::string read() override {
        return buffer_;
    }
    
    void write(const std::string& data) override {
        buffer_ = data;
        std::cout << "Writing to file " << filename_ << ": " << data << std::endl;
    }
};

class StreamDecorator : public DataStream {
protected:
    std::unique_ptr<DataStream> stream_;
    
public:
    StreamDecorator(std::unique_ptr<DataStream> stream) 
        : stream_(std::move(stream)) {}
};

// TODO: Implement CompressionDecorator
class CompressionDecorator : public StreamDecorator {
public:
    // TODO: Compress data on write, decompress on read
};

// TODO: Implement EncryptionDecorator
class EncryptionDecorator : public StreamDecorator {
public:
    // TODO: Encrypt data on write, decrypt on read
};

// TODO: Implement BufferingDecorator
class BufferingDecorator : public StreamDecorator {
public:
    // TODO: Add buffering capability
};

// ===== EXERCISE 4: Pizza Shop =====

class Pizza {
public:
    virtual ~Pizza() = default;
    virtual std::string getDescription() const = 0;
    virtual double getCost() const = 0;
};

// TODO: Implement base pizzas: MargheritaPizza, PepperoniPizza, VeggiePizza

class PizzaDecorator : public Pizza {
protected:
    std::unique_ptr<Pizza> pizza_;
    
public:
    PizzaDecorator(std::unique_ptr<Pizza> pizza) : pizza_(std::move(pizza)) {}
};

// TODO: Implement toppings as decorators:
// - CheeseTopping
// - MushroomTopping
// - OliveTopping
// - BaconTopping
// - PepperTopping

// ===== EXERCISE 5: Window/GUI Components =====

class Window {
public:
    virtual ~Window() = default;
    virtual void draw() const = 0;
    virtual std::string getDescription() const = 0;
};

class SimpleWindow : public Window {
public:
    void draw() const override {
        std::cout << "[Simple Window]" << std::endl;
    }
    
    std::string getDescription() const override {
        return "Simple Window";
    }
};

class WindowDecorator : public Window {
protected:
    std::unique_ptr<Window> window_;
    
public:
    WindowDecorator(std::unique_ptr<Window> window) : window_(std::move(window)) {}
};

// TODO: Implement ScrollbarDecorator
class ScrollbarDecorator : public WindowDecorator {
public:
    // TODO: Add scrollbar to window
};

// TODO: Implement BorderDecorator
class BorderDecorator : public WindowDecorator {
public:
    // TODO: Add border to window
};

// TODO: Implement TitleBarDecorator
class TitleBarDecorator : public WindowDecorator {
private:
    std::string title_;
    
public:
    // TODO: Add title bar to window
};

int main() {
    std::cout << "=== Decorator Pattern Exercise ===\n\n";
    
    std::cout << "--- Coffee Shop Test ---" << std::endl;
    // TODO: Create decorated beverages
    // auto beverage1 = std::make_unique<Espresso>();
    // std::cout << beverage1->getDescription() << " $" << beverage1->cost() << std::endl;
    
    // auto beverage2 = std::make_unique<Espresso>();
    // beverage2 = std::make_unique<Mocha>(std::move(beverage2));
    // beverage2 = std::make_unique<Mocha>(std::move(beverage2));
    // beverage2 = std::make_unique<Whip>(std::move(beverage2));
    // std::cout << beverage2->getDescription() << " $" << beverage2->cost() << std::endl;
    
    // auto beverage3 = std::make_unique<HouseBlend>();
    // beverage3 = std::make_unique<Soy>(std::move(beverage3));
    // beverage3 = std::make_unique<Mocha>(std::move(beverage3));
    // beverage3 = std::make_unique<Whip>(std::move(beverage3));
    // std::cout << beverage3->getDescription() << " $" << beverage3->cost() << std::endl;
    
    std::cout << "\n--- Text Editor Test ---" << std::endl;
    // TODO: Create decorated text
    // auto text = std::make_unique<PlainText>("Hello World");
    // text = std::make_unique<BoldDecorator>(std::move(text));
    // text = std::make_unique<ItalicDecorator>(std::move(text));
    // text = std::make_unique<ColorDecorator>(std::move(text), "red");
    // std::cout << text->render() << std::endl;
    
    std::cout << "\n--- Data Stream Test ---" << std::endl;
    // TODO: Create decorated stream
    // auto stream = std::make_unique<FileStream>("data.txt");
    // stream = std::make_unique<CompressionDecorator>(std::move(stream));
    // stream = std::make_unique<EncryptionDecorator>(std::move(stream));
    // stream->write("Sensitive data");
    // std::cout << "Read: " << stream->read() << std::endl;
    
    std::cout << "\n--- Pizza Shop Test ---" << std::endl;
    // TODO: Create decorated pizzas
    // auto pizza = std::make_unique<MargheritaPizza>();
    // pizza = std::make_unique<CheeseTopping>(std::move(pizza));
    // pizza = std::make_unique<MushroomTopping>(std::move(pizza));
    // pizza = std::make_unique<OliveTopping>(std::move(pizza));
    // std::cout << pizza->getDescription() << " $" << pizza->getCost() << std::endl;
    
    std::cout << "\n--- Window Decorator Test ---" << std::endl;
    // TODO: Create decorated windows
    // auto window = std::make_unique<SimpleWindow>();
    // window = std::make_unique<TitleBarDecorator>(std::move(window), "My App");
    // window = std::make_unique<BorderDecorator>(std::move(window));
    // window = std::make_unique<ScrollbarDecorator>(std::move(window));
    // window->draw();
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete all beverage and condiment decorators
 * 2. Implement text formatting decorators
 * 3. Implement data stream decorators
 * 4. Implement pizza topping decorators
 * 5. Implement window/GUI decorators
 * 6. Add size variations (small, medium, large) to beverages
 * 7. BONUS: Implement a decorator that can be removed (reversible)
 * 8. BONUS: Add caching decorator that memoizes expensive operations
 * 9. BONUS: Implement decorator counting/statistics
 * 
 * DISCUSSION QUESTIONS:
 * - How is Decorator different from inheritance?
 * - What are the downsides of deep decorator chains?
 * - How does Decorator follow the Open/Closed Principle?
 * - When would you use Decorator vs Proxy?
 */
