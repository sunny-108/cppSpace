#include <iostream>
#include <memory>

using namespace std;

class Person{
public:
    std::string name;

    Person(std::string n): name(n) {
        cout<<"\n person created: "<< name <<endl;
    }

    ~Person(){
        cout<<"\n person destroyed: "<< name <<endl;
    }
};

int main(){
    //create a shared pointer

    std::shared_ptr<Person> ptr1 = std::make_shared<Person>("Sunny");
    cout<<"reference count = "<<ptr1.use_count()<<endl;

    {
        //share ownership with ptr2
        std::shared_ptr<Person> ptr2 = ptr1;
        std::cout << "Reference count: " << ptr1.use_count() << "\n";  // 2
        std::cout << "Both point to: " << ptr1->name << "\n";
        //ptr2 outofscope
    }

    std::shared_ptr<Person> ptr3 = ptr1;
    std::cout << "Reference count: " << ptr1.use_count() << "\n";  // 2
    std::cout << "Both ptr3&ptr1 point to: " << ptr1->name << "\n";

    ptr1.reset();
    std::cout << "Reference count after ptr1.reset(): " << ptr3.use_count() << "\n";  // 1
    std::cout << "ptr3 still points to: " << ptr3->name << "\n";

    // Check if ptr1 is valid before using it
    if (ptr1) {
        std::cout << "ptr1 is valid, Reference count: " << ptr1.use_count() << "\n";
        std::cout << "ptr1 points to: " << ptr1->name << "\n";
    } else {
        std::cout << "ptr1 is null after reset(), cannot access members\n";
    }
}