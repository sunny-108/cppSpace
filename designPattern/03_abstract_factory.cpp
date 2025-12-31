/**
 * ABSTRACT FACTORY PATTERN
 * 
 * Purpose: Provide an interface for creating families of related or dependent 
 *          objects without specifying their concrete classes.
 * 
 * Use Cases:
 * - Cross-platform UI frameworks (Windows, Mac, Linux widgets)
 * - Game rendering engines (DirectX, OpenGL, Vulkan)
 * - Database connectors (MySQL, PostgreSQL, MongoDB)
 * - Theme systems
 * 
 * Key Concepts:
 * - Abstract product families
 * - Concrete product families
 * - Abstract factory interface
 * - Concrete factories
 */

#include <iostream>
#include <memory>
#include <string>

// ===== Abstract Products =====
class Button {
public:
    virtual ~Button() = default;
    virtual void paint() = 0;
};

class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual void paint() = 0;
};

class TextField {
public:
    virtual ~TextField() = default;
    virtual void paint() = 0;
};

// ===== Windows Product Family =====
// TODO: Implement WindowsButton, WindowsCheckbox, WindowsTextField

// ===== Mac Product Family =====
// TODO: Implement MacButton, MacCheckbox, MacTextField

// ===== Linux Product Family =====
// TODO: Implement LinuxButton, LinuxCheckbox, LinuxTextField

// ===== Abstract Factory =====
class GUIFactory {
public:
    virtual ~GUIFactory() = default;
    virtual std::unique_ptr<Button> createButton() = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() = 0;
    virtual std::unique_ptr<TextField> createTextField() = 0;
};

// TODO: Implement WindowsFactory, MacFactory, LinuxFactory

// ===== Client Code =====
class Application {
private:
    std::unique_ptr<Button> button_;
    std::unique_ptr<Checkbox> checkbox_;
    std::unique_ptr<TextField> textField_;
    
public:
    Application(std::unique_ptr<GUIFactory> factory) {
        button_ = factory->createButton();
        checkbox_ = factory->createCheckbox();
        textField_ = factory->createTextField();
    }
    
    void paint() {
        button_->paint();
        checkbox_->paint();
        textField_->paint();
    }
};

// ===== EXERCISE 2: Database Factory =====

// TODO: Create abstract products: Connection, Command, DataReader

// TODO: Create concrete products for MySQL, PostgreSQL, and SQLite

// TODO: Create DatabaseFactory interface with methods to create Connection, Command, DataReader

// TODO: Implement MySQLFactory, PostgreSQLFactory, SQLiteFactory

// ===== EXERCISE 3: Game Rendering Factory =====

// TODO: Create abstract products: Renderer, Texture, Shader

// TODO: Create concrete products for DirectX, OpenGL, Vulkan

// TODO: Create RenderingFactory interface

// TODO: Implement DirectXFactory, OpenGLFactory, VulkanFactory

// ===== EXERCISE 4: Theme System =====

class Color {
public:
    virtual ~Color() = default;
    virtual std::string getHex() const = 0;
};

class Font {
public:
    virtual ~Font() = default;
    virtual std::string getName() const = 0;
};

class Icon {
public:
    virtual ~Icon() = default;
    virtual void display() = 0;
};

// TODO: Create DarkTheme and LightTheme product families

// TODO: Create ThemeFactory interface and concrete factories

int main() {
    std::cout << "=== Abstract Factory Pattern Exercise ===\n\n";
    
    std::cout << "--- GUI Factory Test ---" << std::endl;
    
    // TODO: Detect platform and create appropriate factory
    std::unique_ptr<GUIFactory> factory;
    
    #ifdef _WIN32
        std::cout << "Creating Windows UI..." << std::endl;
        // factory = std::make_unique<WindowsFactory>();
    #elif __APPLE__
        std::cout << "Creating Mac UI..." << std::endl;
        // factory = std::make_unique<MacFactory>();
    #else
        std::cout << "Creating Linux UI..." << std::endl;
        // factory = std::make_unique<LinuxFactory>();
    #endif
    
    // TODO: Create and paint application
    // Application app(std::move(factory));
    // app.paint();
    
    std::cout << "\n--- Database Factory Test ---" << std::endl;
    // TODO: Test database factory
    // auto dbFactory = std::make_unique<MySQLFactory>();
    // auto connection = dbFactory->createConnection();
    // auto command = dbFactory->createCommand();
    // connection->open("localhost:3306/mydb");
    // command->execute("SELECT * FROM users");
    
    std::cout << "\n--- Theme Factory Test ---" << std::endl;
    // TODO: Test theme factory
    // auto themeFactory = std::make_unique<DarkThemeFactory>();
    // auto primaryColor = themeFactory->createPrimaryColor();
    // auto font = themeFactory->createFont();
    // std::cout << "Primary color: " << primaryColor->getHex() << std::endl;
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete the GUI factory implementation for all three platforms
 * 2. Implement the Database factory system
 * 3. Implement the Game Rendering factory
 * 4. Implement the Theme system factory
 * 5. Add a new widget type (e.g., Menu) to all GUI families
 * 6. BONUS: Implement a factory selector that chooses the factory at runtime based on config
 * 7. BONUS: Add validation to ensure all products in a family are compatible
 * 
 * DISCUSSION QUESTIONS:
 * - How is Abstract Factory different from Factory Method?
 * - What happens when you need to add a new product to the family?
 * - How does this pattern ensure consistency across product families?
 * - What are the trade-offs of using Abstract Factory?
 */
