/**
 * OBSERVER PATTERN
 * 
 * Purpose: Define a one-to-many dependency between objects so that when one 
 *          object changes state, all its dependents are notified automatically.
 * 
 * Use Cases:
 * - Event handling systems
 * - Model-View-Controller (MVC)
 * - Real-time data feeds (stock prices, weather)
 * - Notification systems
 * - Publish-subscribe systems
 * 
 * Key Concepts:
 * - Subject (Observable)
 * - Observer interface
 * - Concrete observers
 * - Push vs Pull model
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

// ===== Observer Interface =====
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

// ===== Subject (Observable) =====
class Subject {
private:
    std::vector<Observer*> observers_;
    
public:
    virtual ~Subject() = default;
    
    void attach(Observer* observer) {
        observers_.push_back(observer);
    }
    
    void detach(Observer* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }
    
    void notify(const std::string& message) {
        for (auto observer : observers_) {
            observer->update(message);
        }
    }
};

// ===== EXERCISE 1: News Agency System =====

class NewsAgency : public Subject {
private:
    std::string latestNews_;
    
public:
    void setNews(const std::string& news) {
        latestNews_ = news;
        std::cout << "[NewsAgency] Breaking news: " << news << std::endl;
        notify(news);
    }
    
    std::string getLatestNews() const {
        return latestNews_;
    }
};

// TODO: Implement concrete observers
class NewsChannel : public Observer {
private:
    std::string channelName_;
    
public:
    NewsChannel(const std::string& name) : channelName_(name) {}
    
    // TODO: Implement update() to receive and display news
};

class MobileApp : public Observer {
private:
    std::string appName_;
    
public:
    // TODO: Implement update() to send push notification
};

class EmailSubscriber : public Observer {
private:
    std::string email_;
    
public:
    // TODO: Implement update() to send email
};

// ===== EXERCISE 2: Stock Market System =====

class Stock {
private:
    std::string symbol_;
    double price_;
    
public:
    Stock(const std::string& symbol, double price) 
        : symbol_(symbol), price_(price) {}
    
    std::string getSymbol() const { return symbol_; }
    double getPrice() const { return price_; }
};

class StockMarket : public Subject {
private:
    std::vector<Stock> stocks_;
    
public:
    // TODO: Implement addStock()
    // TODO: Implement updateStockPrice() that notifies observers
    
    Stock getStock(const std::string& symbol) const {
        // TODO: Find and return stock
        return Stock("", 0.0);
    }
};

// TODO: Implement StockInvestor observer
class StockInvestor : public Observer {
private:
    std::string name_;
    std::vector<std::string> watchlist_;
    
public:
    // TODO: Implement update() to react to stock price changes
    // TODO: Implement addToWatchlist()
};

// TODO: Implement StockDisplay observer (shows stock ticker)

// ===== EXERCISE 3: Weather Station =====

struct WeatherData {
    float temperature;
    float humidity;
    float pressure;
};

class WeatherStation : public Subject {
private:
    WeatherData currentWeather_;
    
public:
    // TODO: Implement setWeatherData()
    // TODO: Implement getWeatherData()
    
    void measurementsChanged() {
        // TODO: Notify all observers with updated weather data
    }
};

// TODO: Implement CurrentConditionsDisplay
class CurrentConditionsDisplay : public Observer {
public:
    // TODO: Display current temperature, humidity
};

// TODO: Implement StatisticsDisplay (shows avg, min, max)
class StatisticsDisplay : public Observer {
private:
    std::vector<float> temperatureHistory_;
    
public:
    // TODO: Track and display statistics
};

// TODO: Implement ForecastDisplay
class ForecastDisplay : public Observer {
public:
    // TODO: Predict weather based on pressure trends
};

// ===== EXERCISE 4: Event System with Typed Messages =====

template<typename T>
class TypedObserver {
public:
    virtual ~TypedObserver() = default;
    virtual void onEvent(const T& data) = 0;
};

template<typename T>
class EventDispatcher {
private:
    std::vector<TypedObserver<T>*> observers_;
    
public:
    // TODO: Implement subscribe(), unsubscribe(), dispatch()
};

struct MouseEvent {
    int x, y;
    std::string button;
};

struct KeyboardEvent {
    char key;
    bool isPressed;
};

// TODO: Implement concrete observers for mouse and keyboard events

// ===== EXERCISE 5: Thread-Safe Observer (BONUS) =====

#include <mutex>

class ThreadSafeSubject {
private:
    std::vector<Observer*> observers_;
    mutable std::mutex mutex_;
    
public:
    // TODO: Implement thread-safe attach(), detach(), notify()
};

int main() {
    std::cout << "=== Observer Pattern Exercise ===\n\n";
    
    std::cout << "--- News Agency Test ---" << std::endl;
    // TODO: Create news agency and observers
    // NewsAgency agency;
    // NewsChannel cnn("CNN");
    // MobileApp newsApp("NewsApp");
    // EmailSubscriber subscriber("user@example.com");
    
    // agency.attach(&cnn);
    // agency.attach(&newsApp);
    // agency.attach(&subscriber);
    
    // agency.setNews("Major tech company announces breakthrough!");
    // agency.setNews("Markets reach all-time high!");
    
    // agency.detach(&newsApp);
    // agency.setNews("This news won't go to the mobile app!");
    
    std::cout << "\n--- Stock Market Test ---" << std::endl;
    // TODO: Test stock market observers
    // StockMarket market;
    // market.addStock(Stock("AAPL", 150.0));
    // market.addStock(Stock("GOOGL", 2800.0));
    
    // StockInvestor investor1("Alice");
    // investor1.addToWatchlist("AAPL");
    // market.attach(&investor1);
    
    // market.updateStockPrice("AAPL", 155.0);
    
    std::cout << "\n--- Weather Station Test ---" << std::endl;
    // TODO: Test weather station observers
    // WeatherStation station;
    // CurrentConditionsDisplay current;
    // StatisticsDisplay stats;
    // ForecastDisplay forecast;
    
    // station.attach(&current);
    // station.attach(&stats);
    // station.attach(&forecast);
    
    // station.setWeatherData({72.5f, 65.0f, 30.4f});
    // station.setWeatherData({75.0f, 70.0f, 29.2f});
    
    std::cout << "\n--- Event System Test ---" << std::endl;
    // TODO: Test typed event system
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete the News Agency system with all observer types
 * 2. Implement the Stock Market system with investor observers
 * 3. Implement the Weather Station with multiple displays
 * 4. Implement the typed event dispatcher system
 * 5. Add error handling for invalid observer operations
 * 6. BONUS: Implement thread-safe observer pattern
 * 7. BONUS: Add priority levels to observers (high-priority observers notified first)
 * 8. BONUS: Implement lazy/deferred notifications (batch updates)
 * 
 * DISCUSSION QUESTIONS:
 * - What's the difference between push and pull models?
 * - How do you handle observers that might be destroyed?
 * - What are the performance implications of many observers?
 * - How does Observer relate to the Mediator pattern?
 */
