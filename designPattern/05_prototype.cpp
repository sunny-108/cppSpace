/**
 * PROTOTYPE PATTERN
 * 
 * Purpose: Specify the kinds of objects to create using a prototypical instance,
 *          and create new objects by copying this prototype.
 * 
 * Use Cases:
 * - Creating objects that are expensive to initialize
 * - Cloning complex configurations
 * - Game object spawning
 * - Document templates
 * - Cache implementations
 * 
 * Key Concepts:
 * - Clone method
 * - Deep vs shallow copy
 * - Copy constructor
 * - Prototype registry
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ===== Basic Prototype Example =====

class Shape {
public:
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual void draw() const = 0;
    virtual std::string getType() const = 0;
};

// TODO: Implement Circle class
class Circle : public Shape {
private:
    int x_, y_;
    int radius_;
    std::string color_;
    
public:
    Circle(int x, int y, int radius, const std::string& color)
        : x_(x), y_(y), radius_(radius), color_(color) {}
    
    // TODO: Implement clone() method (return a deep copy)
    
    // TODO: Implement draw() and getType()
};

// TODO: Implement Rectangle class
class Rectangle : public Shape {
private:
    int x_, y_;
    int width_, height_;
    std::string color_;
    
public:
    // TODO: Implement constructor, clone(), draw(), getType()
};

// TODO: Implement Triangle class

// ===== Prototype Registry =====

class ShapeRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<Shape>> prototypes_;
    
public:
    void registerPrototype(const std::string& id, std::unique_ptr<Shape> prototype) {
        prototypes_[id] = std::move(prototype);
    }
    
    // TODO: Implement getPrototype() that returns a clone
    std::unique_ptr<Shape> getPrototype(const std::string& id) const {
        // TODO: Find prototype and return a clone
        return nullptr;
    }
};

// ===== EXERCISE 2: Game Character Prototype =====

class Weapon {
public:
    std::string name;
    int damage;
    
    Weapon(const std::string& n, int d) : name(n), damage(d) {}
};

class Armor {
public:
    std::string name;
    int defense;
    
    Armor(const std::string& n, int d) : name(n), defense(d) {}
};

class GameCharacter {
protected:
    std::string name_;
    int health_;
    int mana_;
    std::unique_ptr<Weapon> weapon_;
    std::unique_ptr<Armor> armor_;
    std::vector<std::string> abilities_;
    
public:
    virtual ~GameCharacter() = default;
    
    // TODO: Implement proper copy constructor for deep copy
    
    // TODO: Implement clone() method
    virtual std::unique_ptr<GameCharacter> clone() const = 0;
    
    virtual void displayStats() const {
        std::cout << "Character: " << name_ << std::endl;
        std::cout << "  Health: " << health_ << std::endl;
        std::cout << "  Mana: " << mana_ << std::endl;
        if (weapon_) {
            std::cout << "  Weapon: " << weapon_->name << " (Damage: " << weapon_->damage << ")" << std::endl;
        }
        if (armor_) {
            std::cout << "  Armor: " << armor_->name << " (Defense: " << armor_->defense << ")" << std::endl;
        }
        std::cout << "  Abilities: ";
        for (const auto& ability : abilities_) {
            std::cout << ability << " ";
        }
        std::cout << std::endl;
    }
};

// TODO: Implement Warrior, Mage, Archer classes with clone()

// ===== EXERCISE 3: Document Template System =====

class Section {
public:
    std::string title;
    std::string content;
    std::vector<std::string> items;
    
    // TODO: Implement copy constructor for deep copy
};

class DocumentTemplate {
protected:
    std::string title_;
    std::string author_;
    std::vector<Section> sections_;
    std::unordered_map<std::string, std::string> metadata_;
    
public:
    virtual ~DocumentTemplate() = default;
    
    // TODO: Implement clone() method with deep copy
    virtual std::unique_ptr<DocumentTemplate> clone() const = 0;
    
    void addSection(const Section& section) {
        sections_.push_back(section);
    }
    
    void setMetadata(const std::string& key, const std::string& value) {
        metadata_[key] = value;
    }
    
    virtual void display() const {
        std::cout << "Document: " << title_ << std::endl;
        std::cout << "Author: " << author_ << std::endl;
        std::cout << "Sections: " << sections_.size() << std::endl;
    }
};

// TODO: Implement ReportTemplate, ProposalTemplate, InvoiceTemplate

// ===== EXERCISE 4: Cache with Prototype =====

template<typename T>
class PrototypeCache {
private:
    std::unordered_map<std::string, std::shared_ptr<T>> cache_;
    
public:
    // TODO: Implement addPrototype()
    void addPrototype(const std::string& key, std::shared_ptr<T> prototype) {
        cache_[key] = prototype;
    }
    
    // TODO: Implement getClone() that returns a clone of cached object
    std::unique_ptr<T> getClone(const std::string& key) {
        // TODO: Clone the cached object and return it
        return nullptr;
    }
    
    bool hasPrototype(const std::string& key) const {
        return cache_.find(key) != cache_.end();
    }
};

// ===== EXERCISE 5: Deep vs Shallow Copy Example =====

class Node {
public:
    int data;
    std::unique_ptr<Node> next;
    
    Node(int d) : data(d), next(nullptr) {}
    
    // TODO: Implement shallow copy
    std::unique_ptr<Node> shallowCopy() const {
        // TODO: Only copy the data, share the next pointer
        return nullptr;
    }
    
    // TODO: Implement deep copy
    std::unique_ptr<Node> deepCopy() const {
        // TODO: Recursively copy all nodes
        return nullptr;
    }
};

int main() {
    std::cout << "=== Prototype Pattern Exercise ===\n\n";
    
    std::cout << "--- Shape Prototype Test ---" << std::endl;
    // TODO: Create prototypes and registry
    // ShapeRegistry registry;
    // registry.registerPrototype("circle", std::make_unique<Circle>(0, 0, 10, "red"));
    // registry.registerPrototype("rectangle", std::make_unique<Rectangle>(0, 0, 20, 30, "blue"));
    
    // TODO: Clone shapes from registry
    // auto shape1 = registry.getPrototype("circle");
    // auto shape2 = registry.getPrototype("circle");
    // shape1->draw();
    // shape2->draw();
    
    std::cout << "\n--- Game Character Prototype Test ---" << std::endl;
    // TODO: Create character templates
    // auto warriorTemplate = std::make_unique<Warrior>("Warrior", 100, 20);
    // warriorTemplate->setWeapon(std::make_unique<Weapon>("Sword", 50));
    
    // TODO: Clone characters
    // auto warrior1 = warriorTemplate->clone();
    // auto warrior2 = warriorTemplate->clone();
    // warrior1->displayStats();
    
    std::cout << "\n--- Document Template Test ---" << std::endl;
    // TODO: Create and clone document templates
    
    std::cout << "\n--- Prototype Cache Test ---" << std::endl;
    // TODO: Test prototype cache
    // PrototypeCache<Shape> cache;
    // cache.addPrototype("default_circle", std::make_shared<Circle>(0, 0, 5, "green"));
    // auto clonedShape = cache.getClone("default_circle");
    
    std::cout << "\n--- Deep vs Shallow Copy Test ---" << std::endl;
    // TODO: Demonstrate difference between deep and shallow copy
    // Create a linked list and show shallow vs deep copy behavior
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete the Shape hierarchy with clone() implementations
 * 2. Implement the ShapeRegistry properly
 * 3. Implement GameCharacter prototypes with deep copy
 * 4. Implement DocumentTemplate system
 * 5. Implement the PrototypeCache template class
 * 6. Demonstrate deep vs shallow copy with linked list
 * 7. BONUS: Add a prototype manager that tracks all instances
 * 8. BONUS: Implement serialization/deserialization for prototypes
 * 
 * DISCUSSION QUESTIONS:
 * - When is cloning more efficient than creating new objects?
 * - What are the pitfalls of shallow copying?
 * - How does Prototype relate to copy constructors in C++?
 * - When would you use Prototype instead of Factory?
 */
