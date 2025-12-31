/**
 * ADAPTER PATTERN
 * 
 * Purpose: Convert the interface of a class into another interface clients expect.
 *          Adapter lets classes work together that couldn't otherwise because of 
 *          incompatible interfaces.
 * 
 * Use Cases:
 * - Integrating third-party libraries
 * - Legacy code integration
 * - Payment gateway integration
 * - Database drivers
 * - Media players
 * 
 * Key Concepts:
 * - Target interface
 * - Adaptee (existing incompatible class)
 * - Adapter (makes adaptee compatible)
 * - Object adapter vs Class adapter
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ===== EXERCISE 1: Media Player Adapter =====

// Target Interface (what client expects)
class MediaPlayer {
public:
    virtual ~MediaPlayer() = default;
    virtual void play(const std::string& filename) = 0;
};

// Adaptees (existing incompatible interfaces)
class MP3Player {
public:
    void playMP3(const std::string& filename) {
        std::cout << "Playing MP3 file: " << filename << std::endl;
    }
};

class MP4Player {
public:
    void playMP4File(const std::string& filename) {
        std::cout << "Playing MP4 file: " << filename << std::endl;
    }
};

class VLCPlayer {
public:
    void playVLC(const std::string& filename) {
        std::cout << "Playing VLC file: " << filename << std::endl;
    }
};

// TODO: Implement MP3Adapter
class MP3Adapter : public MediaPlayer {
private:
    std::unique_ptr<MP3Player> mp3Player_;
    
public:
    // TODO: Implement play() that calls mp3Player_->playMP3()
};

// TODO: Implement MP4Adapter
class MP4Adapter : public MediaPlayer {
private:
    std::unique_ptr<MP4Player> mp4Player_;
    
public:
    // TODO: Implement play() that calls mp4Player_->playMP4File()
};

// TODO: Implement VLCAdapter
class VLCAdapter : public MediaPlayer {
    // TODO: Adapt VLCPlayer to MediaPlayer interface
};

// Universal Audio Player (uses adapters)
class AudioPlayer {
private:
    std::unique_ptr<MediaPlayer> adapter_;
    
public:
    void setAdapter(std::unique_ptr<MediaPlayer> adapter) {
        adapter_ = std::move(adapter);
    }
    
    void playAudio(const std::string& filename) {
        if (adapter_) {
            adapter_->play(filename);
        } else {
            std::cout << "No adapter set!" << std::endl;
        }
    }
};

// ===== EXERCISE 2: Payment Gateway Adapter =====

// Target Interface
class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;
    virtual bool processPayment(double amount, const std::string& currency) = 0;
    virtual std::string getTransactionId() const = 0;
};

// Adaptee 1: PayPal SDK
class PayPalSDK {
public:
    void makePayment(double usdAmount) {
        std::cout << "PayPal processing $" << usdAmount << std::endl;
    }
    
    std::string getPayPalTransactionRef() const {
        return "PP-" + std::to_string(rand() % 10000);
    }
};

// Adaptee 2: Stripe API
class StripeAPI {
public:
    bool charge(int amountInCents, const std::string& curr) {
        std::cout << "Stripe charging " << amountInCents << " cents in " << curr << std::endl;
        return true;
    }
    
    std::string getChargeId() const {
        return "ch_" + std::to_string(rand() % 10000);
    }
};

// Adaptee 3: Square
class SquarePayment {
public:
    void processTransaction(double amount, const std::string& currency) {
        std::cout << "Square processing " << amount << " " << currency << std::endl;
    }
    
    std::string getReceiptNumber() const {
        return "SQ-" + std::to_string(rand() % 10000);
    }
};

// TODO: Implement PayPalAdapter
class PayPalAdapter : public PaymentProcessor {
private:
    std::unique_ptr<PayPalSDK> paypal_;
    std::string transactionId_;
    
public:
    // TODO: Convert currency to USD if needed, process payment
};

// TODO: Implement StripeAdapter
class StripeAdapter : public PaymentProcessor {
    // TODO: Convert amount to cents, process payment
};

// TODO: Implement SquareAdapter
class SquareAdapter : public PaymentProcessor {
    // TODO: Adapt Square to PaymentProcessor interface
};

// ===== EXERCISE 3: Database Adapter =====

// Target Interface
class Database {
public:
    virtual ~Database() = default;
    virtual void connect(const std::string& connectionString) = 0;
    virtual void executeQuery(const std::string& query) = 0;
    virtual std::vector<std::string> fetchResults() = 0;
    virtual void disconnect() = 0;
};

// Adaptee 1: MySQL Library
class MySQLLib {
public:
    void mysql_connect(const char* host, const char* user, const char* pass) {
        std::cout << "MySQL connected to " << host << std::endl;
    }
    
    void mysql_query(const char* sql) {
        std::cout << "MySQL executing: " << sql << std::endl;
    }
    
    void mysql_close() {
        std::cout << "MySQL connection closed" << std::endl;
    }
};

// Adaptee 2: PostgreSQL Library
class PostgreSQLLib {
public:
    void PQconnectdb(const std::string& connInfo) {
        std::cout << "PostgreSQL connected with: " << connInfo << std::endl;
    }
    
    void PQexec(const std::string& command) {
        std::cout << "PostgreSQL executing: " << command << std::endl;
    }
    
    void PQfinish() {
        std::cout << "PostgreSQL connection closed" << std::endl;
    }
};

// TODO: Implement MySQLAdapter
class MySQLAdapter : public Database {
    // TODO: Adapt MySQLLib to Database interface
};

// TODO: Implement PostgreSQLAdapter
class PostgreSQLAdapter : public Database {
    // TODO: Adapt PostgreSQLLib to Database interface
};

// ===== EXERCISE 4: Temperature Sensor Adapter =====

// Target Interface (expects Celsius)
class TemperatureSensor {
public:
    virtual ~TemperatureSensor() = default;
    virtual double getTemperatureCelsius() = 0;
};

// Adaptee: Legacy Fahrenheit Sensor
class FahrenheitSensor {
public:
    double getTemperatureFahrenheit() {
        return 98.6;  // Body temperature in Fahrenheit
    }
};

// Adaptee: Kelvin Sensor
class KelvinSensor {
public:
    double getTemperatureKelvin() {
        return 310.15;  // Body temperature in Kelvin
    }
};

// TODO: Implement FahrenheitAdapter
class FahrenheitAdapter : public TemperatureSensor {
private:
    std::unique_ptr<FahrenheitSensor> sensor_;
    
public:
    // TODO: Convert Fahrenheit to Celsius
    // Formula: C = (F - 32) * 5/9
};

// TODO: Implement KelvinAdapter
class KelvinAdapter : public TemperatureSensor {
    // TODO: Convert Kelvin to Celsius
    // Formula: C = K - 273.15
};

// ===== EXERCISE 5: JSON to XML Adapter =====

class JSONData {
private:
    std::string jsonString_;
    
public:
    JSONData(const std::string& json) : jsonString_(json) {}
    
    std::string getJSON() const {
        return jsonString_;
    }
};

class XMLProcessor {
public:
    virtual ~XMLProcessor() = default;
    virtual void processXML(const std::string& xml) = 0;
};

// TODO: Implement JSONToXMLAdapter
class JSONToXMLAdapter : public XMLProcessor {
private:
    std::unique_ptr<JSONData> jsonData_;
    
public:
    // TODO: Convert JSON to XML and process
    // Simple conversion: {"key": "value"} -> <key>value</key>
};

int main() {
    std::cout << "=== Adapter Pattern Exercise ===\n\n";
    
    std::cout << "--- Media Player Adapter Test ---" << std::endl;
    // TODO: Test media player adapters
    // AudioPlayer player;
    
    // player.setAdapter(std::make_unique<MP3Adapter>());
    // player.playAudio("song.mp3");
    
    // player.setAdapter(std::make_unique<MP4Adapter>());
    // player.playAudio("video.mp4");
    
    // player.setAdapter(std::make_unique<VLCAdapter>());
    // player.playAudio("movie.vlc");
    
    std::cout << "\n--- Payment Gateway Adapter Test ---" << std::endl;
    // TODO: Test payment adapters
    // std::unique_ptr<PaymentProcessor> processor;
    
    // processor = std::make_unique<PayPalAdapter>();
    // processor->processPayment(100.0, "USD");
    // std::cout << "Transaction ID: " << processor->getTransactionId() << std::endl;
    
    // processor = std::make_unique<StripeAdapter>();
    // processor->processPayment(50.0, "EUR");
    
    // processor = std::make_unique<SquareAdapter>();
    // processor->processPayment(75.0, "GBP");
    
    std::cout << "\n--- Database Adapter Test ---" << std::endl;
    // TODO: Test database adapters
    // std::unique_ptr<Database> db;
    
    // db = std::make_unique<MySQLAdapter>();
    // db->connect("localhost:3306/mydb");
    // db->executeQuery("SELECT * FROM users");
    // db->disconnect();
    
    // db = std::make_unique<PostgreSQLAdapter>();
    // db->connect("postgresql://localhost/mydb");
    // db->executeQuery("SELECT * FROM users");
    // db->disconnect();
    
    std::cout << "\n--- Temperature Sensor Adapter Test ---" << std::endl;
    // TODO: Test temperature adapters
    // std::unique_ptr<TemperatureSensor> sensor;
    
    // sensor = std::make_unique<FahrenheitAdapter>();
    // std::cout << "Temperature: " << sensor->getTemperatureCelsius() << "°C" << std::endl;
    
    // sensor = std::make_unique<KelvinAdapter>();
    // std::cout << "Temperature: " << sensor->getTemperatureCelsius() << "°C" << std::endl;
    
    std::cout << "\n--- JSON to XML Adapter Test ---" << std::endl;
    // TODO: Test JSON to XML adapter
    // auto jsonData = std::make_unique<JSONData>("{\"name\": \"John\", \"age\": 30}");
    // auto adapter = std::make_unique<JSONToXMLAdapter>(std::move(jsonData));
    // adapter->processXML();
    
    return 0;
}

/**
 * EXERCISES:
 * 
 * 1. Complete all media player adapters
 * 2. Implement payment gateway adapters with currency conversion
 * 3. Implement database adapters for different SQL libraries
 * 4. Implement temperature sensor adapters with conversion formulas
 * 5. Implement JSON to XML adapter
 * 6. Add error handling for conversion failures
 * 7. BONUS: Implement bidirectional adapters (two-way conversion)
 * 8. BONUS: Create a class adapter using multiple inheritance
 * 9. BONUS: Implement an adapter registry for dynamic adapter selection
 * 
 * DISCUSSION QUESTIONS:
 * - What's the difference between Object Adapter and Class Adapter?
 * - When should you use Adapter vs Bridge pattern?
 * - How does Adapter help with the Open/Closed Principle?
 * - What are the trade-offs of adding an adapter layer?
 */
