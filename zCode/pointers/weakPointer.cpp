#include <iostream>
#include <memory>

class Person {
public:
    std::string name;
    
    Person(std::string n) : name(n) {
        std::cout << name << " created\n";
    }
    
    ~Person() {
        std::cout << name << " destroyed\n";
    }
};

int main() {
    std::weak_ptr<Person> weakPtr;
    
    {
        // Create shared pointer
        std::shared_ptr<Person> sharedPtr = std::make_shared<Person>("Bob");
        std::cout << "Shared count: " << sharedPtr.use_count() << "\n";  // 1
        
        // Create weak pointer from shared pointer
        weakPtr = sharedPtr;
        std::cout << "Shared count after weak: " << sharedPtr.use_count() << "\n";  // Still 1!
        
        // To use weak_ptr, convert it to shared_ptr with lock()
        if (auto tempPtr = weakPtr.lock()) {
            std::cout << "Object exists! Name: " << tempPtr->name << "\n";
            std::cout << "Shared count during lock: " << tempPtr.use_count() << "\n";  // 2
        }
        
        // sharedPtr goes out of scope here - object will be destroyed
    }
    
    std::cout << "\nAfter shared_ptr destroyed:\n";
    
    // Try to access through weak pointer
    if (auto tempPtr = weakPtr.lock()) {
        std::cout << "Object still exists: " << tempPtr->name << "\n";
    } else {
        std::cout << "Object has been destroyed - weak_ptr is expired\n";
    }
    
    // Check if weak_ptr is expired
    std::cout << "Is expired? " << (weakPtr.expired() ? "Yes" : "No") << "\n";
    
    return 0;
}