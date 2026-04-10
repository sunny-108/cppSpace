/**
 * FACTORY METHOD PATTERN
 * 
 * Purpose: Define an interface for creating objects, but let subclasses decide 
 *          which class to instantiate.
 * 
 * Use Cases:
 * - Document management systems (PDF, Word, Excel)
 * - UI frameworks (buttons, dialogs for different platforms)
 * - Game character creation
 * - Plugin systems
 * 
 * Key Concepts:
 * - Product interface
 * - Concrete products
 * - Creator with factory method
 * - Concrete creators
 */

#include <iostream>
#include <memory>
#include <string>

// Product Interface
class Document {
public:
    virtual ~Document() = default;
    virtual void open() = 0;
    virtual void save() = 0;
    virtual std::string getType() const = 0;
};

// TODO: Implement Concrete Products
class PDFDocument : public Document {
public:
    // TODO: Implement open(), save(), getType()
};

class WordDocument : public Document {
public:
    // TODO: Implement open(), save(), getType()
};

class ExcelDocument : public Document {
public:
    // TODO: Implement open(), save(), getType()
};

// Creator (Abstract Factory)
class Application {
public:
    virtual ~Application() = default;
    
    // Factory Method
    virtual std::unique_ptr<Document> createDocument() = 0;
    
    // Uses the factory method
    void newDocument() {
        auto doc = createDocument();
        doc->open();
        std::cout << "New document created" << std::endl;
    }
};

// TODO: Implement Concrete Creators
class PDFApplication : public Application {
public:
    // TODO: Implement createDocument() to return PDFDocument
};

class WordApplication : public Application {
public:
    // TODO: Implement createDocument() to return WordDocument
};

class ExcelApplication : public Application {
public:
    // TODO: Implement createDocument() to return ExcelDocument
};

// ===== EXERCISE 2: Game Character Factory =====

// TODO: Create a Character interface with methods: attack(), defend(), getRole()

// TODO: Create concrete character classes: Warrior, Mage, Archer

// TODO: Create CharacterCreator abstract class with createCharacter() factory method

// TODO: Create concrete creators: WarriorCreator, MageCreator, ArcherCreator

// ===== EXERCISE 3: UI Button Factory (Cross-platform) =====

class Button {
public:
    virtual ~Button() = default;
    virtual void render() = 0;
    virtual void onClick() = 0;
};

// TODO: Implement WindowsButton and MacButton

class Dialog {
public:
    virtual ~Dialog() = default;
    
    // Factory method
    virtual std::unique_ptr<Button> createButton() = 0;
    
    void render() {
        auto button = createButton();
        button->render();
        button->onClick();
    }
};

// TODO: Implement WindowsDialog and MacDialog

int main() {
    std::cout << "=== Factory Method Pattern Exercise ===\n\n";
    
    std::cout << "--- Document Factory Test ---" << std::endl;
    // TODO: Test document factories
    // std::unique_ptr<Application> pdfApp = std::make_unique<PDFApplication>();
    // pdfApp->newDocument();
    
    // std::unique_ptr<Application> wordApp = std::make_unique<WordApplication>();
    // wordApp->newDocument();
    
    std::cout << "\n--- Character Factory Test ---" << std::endl;
    // TODO: Test character factories
    // std::unique_ptr<CharacterCreator> warriorCreator = std::make_unique<WarriorCreator>();
    // auto warrior = warriorCreator->createCharacter();
    // warrior->attack();
    
    std::cout << "\n--- UI Button Factory Test ---" << std::endl;
    // TODO: Test button factories based on platform
    #ifdef _WIN32
        // std::unique_ptr<Dialog> dialog = std::make_unique<WindowsDialog>();
    #else
        // std::unique_ptr<Dialog> dialog = std::make_unique<MacDialog>();
    #endif
    // dialog->render();
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete the Document factory implementation
 * 2. Implement the game Character factory system
 * 3. Implement the cross-platform UI Button factory
 * 4. Add a new document type (e.g., TextDocument) without modifying existing code
 * 5. BONUS: Add error handling for invalid document types
 * 6. BONUS: Implement a registry-based factory that can dynamically register new types
 * 
 * DISCUSSION QUESTIONS:
 * - How does Factory Method follow the Open/Closed Principle?
 * - When would you use Factory Method vs Simple Factory?
 * - How does this pattern help with testing?
 */
