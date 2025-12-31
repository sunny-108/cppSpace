#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Observer interface
class IObserver {
public:
    virtual void update(const string& msg) = 0;
    virtual ~IObserver() = default;
};

class ConcreteObserver : public IObserver {
private:
    string observerID;
public:
    ConcreteObserver(string id) :  observerID(id) {}
    void update(const string& msg) override{
        cout<<"\nfrom observer: "<<observerID<<"\t updated message recived = "<<msg<<endl;
    }
};


class ISubject {
public:
    virtual ~ISubject() = default;
    virtual void attach(IObserver* ob) = 0;
    virtual void detach(IObserver* ob) = 0;
    virtual void notify(const string& msg) = 0;
};

class ConcreteSubject: public ISubject{
private:
    vector<IObserver*> observersList;
    string state;

public:
    void attach(IObserver* ob) override {
        observersList.push_back(ob);
        cout<<"\n observer sucessfully registered "<<endl;
    }

    void detach(IObserver* ob) override {
        observersList.erase(remove(observersList.begin(), observersList.end(), ob), observersList.end());
        cout<<"\n observer unregistered"<<endl;
    }

    void notify(const string& msg) override {
        cout<<"\n new state = "<<msg<<" notifying observer"<<endl;
        for(IObserver* ob : observersList){
            ob->update(msg);
        }
    }
};

int main(){

    ConcreteSubject* subject = new ConcreteSubject();
    
    ConcreteObserver* ob1 = new ConcreteObserver("ob1");
    ConcreteObserver* ob2 = new ConcreteObserver("ob2");

    subject->attach(ob1);
    subject->attach(ob2);

    subject->notify("cold climate");

    ConcreteObserver* ob3 = new ConcreteObserver("ob3");
    ConcreteObserver* ob4 = new ConcreteObserver("ob4");

    subject->attach(ob3);
    subject->attach(ob4);

    subject->notify("hot climate");

    subject->detach(ob1);

    subject->notify("sunny");

}