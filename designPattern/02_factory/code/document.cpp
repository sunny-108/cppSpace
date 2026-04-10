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
    void open() override {
        std::cout<<"\n open pdf file "<<std::endl;
    }

    void save() override {
        std::cout<<"\n save pdf file "<<std::endl;
    }

    std::string getType() const override {
        return "pdf";
    }
};

class WordDocument : public Document {
public:
    // TODO: Implement open(), save(), getType()
     void open() override {
        std::cout<<"\n open word file "<<std::endl;
    }

    void save() override {
        std::cout<<"\n save word file "<<std::endl;
    }

    std::string getType() const override {
        return "word";
    }
};

class ExcelDocument : public Document {
public:
    // TODO: Implement open(), save(), getType()
     void open() override {
        std::cout<<"\n open excell file "<<std::endl;
    }

    void save() override {
        std::cout<<"\n save excell file "<<std::endl;
    }

    std::string getType() const override {
        return "excell";
    }
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
    std::unique_ptr<Document> createDocument() override {
        return std::make_unique<PDFDocument>();
    }
};

class WordApplication : public Application {
public:
    std::unique_ptr<Document> createDocument() override {
        return std::make_unique<WordDocument>();
    }
};

class ExcelApplication : public Application {
public:
    std::unique_ptr<Document> createDocument() override {
        return std::make_unique<ExcelDocument>();
    }
};


int main() {
    std::cout << "=== Factory Method Pattern Exercise ===\n\n";
    
    std::cout << "--- Document Factory Test ---" << std::endl;
    // TODO: Test document factories
    std::unique_ptr<Application> pdfApp = std::make_unique<PDFApplication>();
    pdfApp->newDocument();
    
    std::unique_ptr<Application> wordApp = std::make_unique<WordApplication>();
    wordApp->newDocument();
    
    
    return 0;
}
