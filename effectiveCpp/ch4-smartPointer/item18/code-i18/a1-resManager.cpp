#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cstdio>

// Base Resource class with virtual destructor
class Resource {
public:
    virtual ~Resource() = default;
    virtual std::string getDescription() const = 0;
};

// Custom deleter for FILE*
struct FileDeleter {
    std::string filename;
    void operator()(FILE* file) const {
        if (file) {
            std::cout << "Closing File: " << filename << std::endl;
            fclose(file);
        }
    }
};

// FileResource - manages FILE* handles
class FileResource : public Resource {
private:
    std::unique_ptr<FILE, FileDeleter> file_;
    std::string filename_;

public:
    explicit FileResource(const std::string& filename) : filename_(filename) {
        std::cout << "Creating File Resource: " << filename << std::endl;
        FILE* f = fopen(filename.c_str(), "w");
        if (!f) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        file_ = std::unique_ptr<FILE, FileDeleter>(f, FileDeleter{filename});
    }

    std::string getDescription() const override {
        return "File: " + filename_;
    }
};

// Custom deleter for socket
struct SocketDeleter {
    int port;
    void operator()(int* socket) const {
        if (socket && *socket >= 0) {
            std::cout << "Closing Socket: " << port << std::endl;
            // In real code, would call close(*socket)
            delete socket;
        }
    }
};

// SocketResource - manages socket descriptors
class SocketResource : public Resource {
private:
    std::unique_ptr<int, SocketDeleter> socket_;
    int port_;

public:
    explicit SocketResource(int port) : port_(port) {
        std::cout << "Creating Socket Resource on port: " << port << std::endl;
        // Simulate socket creation (in real code, would call socket())
        int* sock = new int(port);  // Just storing the port as the "socket fd"
        socket_ = std::unique_ptr<int, SocketDeleter>(sock, SocketDeleter{port});
    }

    std::string getDescription() const override {
        return "Socket: " + std::to_string(port_);
    }
};

// Custom deleter for database connection
struct DatabaseDeleter {
    std::string dbName;
    void operator()(void* connection) const {
        if (connection) {
            std::cout << "Closing Database: " << dbName << std::endl;
            // In real code, would close database connection
            delete static_cast<char*>(connection);
        }
    }
};

// DatabaseResource - manages DB connections
class DatabaseResource : public Resource {
private:
    std::unique_ptr<void, DatabaseDeleter> connection_;
    std::string dbName_;

public:
    explicit DatabaseResource(const std::string& dbName) : dbName_(dbName) {
        std::cout << "Creating Database Resource: " << dbName << std::endl;
        // Simulate database connection (mock object)
        void* conn = new char[1];  // Mock connection
        connection_ = std::unique_ptr<void, DatabaseDeleter>(conn, DatabaseDeleter{dbName});
    }

    std::string getDescription() const override {
        return "Database: " + dbName_;
    }
};

// ResourceManager - manages all resources with automatic cleanup
class ResourceManager {
private:
    std::vector<std::unique_ptr<Resource>> resources_;

public:
    // Add a resource to the manager
    void addResource(std::unique_ptr<Resource> resource) {
        resources_.push_back(std::move(resource));
    }

    // Remove a resource at specified index
    std::unique_ptr<Resource> removeResource(size_t index) {
        if (index >= resources_.size()) {
            throw std::out_of_range("Invalid resource index");
        }
        auto resource = std::move(resources_[index]);
        resources_.erase(resources_.begin() + index);
        return resource;
    }

    // Get resource count
    size_t getResourceCount() const {
        return resources_.size();
    }

    // List all active resources
    void listResources() const {
        std::cout << "Listing resources:" << std::endl;
        for (const auto& resource : resources_) {
            std::cout << "  - " << resource->getDescription() << std::endl;
        }
    }

    // Destructor automatically cleans up all resources
    ~ResourceManager() {
        std::cout << "Resources cleaned up" << std::endl;
    }
};

int main() {
    try {
        ResourceManager manager;

        // Add various resources
        manager.addResource(std::make_unique<FileResource>("data.txt"));
        manager.addResource(std::make_unique<SocketResource>(8080));
        manager.addResource(std::make_unique<DatabaseResource>("UserDB"));

        // Show resource count
        std::cout << "Resources active: " << manager.getResourceCount() << std::endl;

        // List all resources
        manager.listResources();

        // Demonstrate exception safety
        try {
            // This will fail if file cannot be created, but won't leak resources
            manager.addResource(std::make_unique<FileResource>("/invalid/path/file.txt"));
        } catch (const std::exception& e) {
            std::cout << "Exception caught (expected): " << e.what() << std::endl;
        }

        // Resources will be automatically cleaned up when manager goes out of scope
        // Custom deleters will be called for each resource

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}