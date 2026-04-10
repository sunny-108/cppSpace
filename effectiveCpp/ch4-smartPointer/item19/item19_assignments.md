# Item 19 Programming Assignments: std::shared_ptr

## 📚 Assignment Overview
These assignments progress from medium to advanced difficulty, focusing on shared ownership patterns, `weak_ptr`, `enable_shared_from_this`, and real-world scenarios where `shared_ptr` is essential.

---

## Assignment 1: Multi-User Document Editor System (Medium)

### 🎯 Objective
Build a collaborative document editing system where multiple users can simultaneously edit the same document. The document should remain alive as long as at least one user is editing it.

### 📋 Requirements

1. Create a `Document` class that:
   - Has a unique ID and content (string)
   - Tracks edit history (vector of changes)
   - Has methods: `write()`, `read()`, `getHistory()`, `save()`
   - Prints creation/destruction messages
   - Uses `enable_shared_from_this` to register with undo manager

2. Create a `User` class that:
   - Has a name and ID
   - Can open documents (receives `shared_ptr<Document>`)
   - Can edit documents
   - Can close documents (releases ownership)
   - Tracks which documents are currently open

3. Create a `DocumentManager` class that:
   - Stores documents using `weak_ptr` in a map (ID → weak_ptr)
   - Creates new documents
   - Returns existing documents if still alive
   - Cleans up expired weak_ptrs periodically
   - Provides statistics (total docs created, active docs)

4. Create an `UndoManager` class that:
   - Stores weak references to documents
   - Can perform undo operations on all active documents
   - Automatically skips expired documents

5. Implement a `Session` class that:
   - Manages multiple users
   - Coordinates document access
   - Handles save-on-exit for active documents

### 💻 Key Design Points

```cpp
class Document : public std::enable_shared_from_this<Document> {
private:
    int id_;
    std::string content_;
    std::vector<std::string> history_;
    
public:
    explicit Document(int id);
    ~Document();
    
    void write(const std::string& text, const std::string& author);
    std::string read() const;
    void save();
    std::shared_ptr<Document> getSharedPtr();
};

class User {
private:
    std::string name_;
    std::map<int, std::shared_ptr<Document>> openDocuments_;
    
public:
    explicit User(std::string name);
    void openDocument(std::shared_ptr<Document> doc);
    void editDocument(int docId, const std::string& text);
    void closeDocument(int docId);
    size_t activeDocuments() const;
};

class DocumentManager {
private:
    std::map<int, std::weak_ptr<Document>> documents_;
    int nextId_;
    int totalCreated_;
    
public:
    DocumentManager();
    std::shared_ptr<Document> createDocument();
    std::shared_ptr<Document> getDocument(int id);
    void cleanupExpired();
    size_t activeCount() const;
};
```

### ✅ Test Scenarios

```cpp
int main() {
    DocumentManager manager;
    
    // Scenario 1: Create and share document
    auto doc1 = manager.createDocument();
    std::cout << "Reference count: " << doc1.use_count() << "\n";
    
    {
        User alice("Alice");
        User bob("Bob");
        
        alice.openDocument(doc1);
        bob.openDocument(doc1);
        
        std::cout << "Reference count with 2 users: " << doc1.use_count() << "\n";
        
        alice.editDocument(doc1->getId(), "Hello from Alice");
        bob.editDocument(doc1->getId(), "Hello from Bob");
        
        alice.closeDocument(doc1->getId());
        std::cout << "Reference count after Alice closes: " << doc1.use_count() << "\n";
    }
    
    std::cout << "Reference count after users destroyed: " << doc1.use_count() << "\n";
    
    // Scenario 2: Document cleanup
    {
        User charlie("Charlie");
        auto doc2 = manager.createDocument();
        charlie.openDocument(doc2);
        // doc2 goes out of scope, but still alive via charlie
    }
    
    manager.cleanupExpired();
    std::cout << "Active documents: " << manager.activeCount() << "\n";
    
    return 0;
}
```

### 🎓 Learning Outcomes
- Shared ownership across multiple owners
- Automatic lifetime management
- `weak_ptr` for non-owning references
- `enable_shared_from_this` usage
- Reference counting behavior

---

## Assignment 2: Observer Pattern with Automatic Cleanup (Medium-Advanced)

### 🎯 Objective
Implement a robust Observer pattern using `weak_ptr` that automatically handles observer lifetime and prevents memory leaks from circular references.

### 📋 Requirements

1. Create a `Subject` class template that:
   - Stores observers as `weak_ptr`
   - Automatically removes expired observers during notification
   - Supports multiple notification types (update, error, complete)
   - Thread-safe (use mutex for observer list)
   - Provides observer count (active vs registered)

2. Create an `Observer` base class template with:
   - Pure virtual methods for different event types
   - Support for filtering events by type
   - Priority levels for notification order

3. Implement concrete observers:
   - `LoggingObserver` - logs all events to console
   - `StatisticsObserver` - tracks event counts
   - `AlertObserver` - triggers alerts on specific conditions
   - `FileObserver` - writes events to file

4. Create a `WeatherStation` class (concrete subject) that:
   - Publishes temperature, humidity, pressure updates
   - Inherits from `Subject<WeatherData>`
   - Notifies observers when readings change significantly

5. Implement an `ObserverManager` that:
   - Creates and tracks observers
   - Can bulk-register/unregister observers
   - Provides observer lifecycle statistics

### 💻 Advanced Design

```cpp
template<typename T>
class Subject {
protected:
    struct ObserverInfo {
        std::weak_ptr<Observer<T>> observer;
        int priority;
        bool operator<(const ObserverInfo& other) const {
            return priority > other.priority;  // Higher priority first
        }
    };
    
    std::vector<ObserverInfo> observers_;
    mutable std::mutex mutex_;
    
public:
    void attach(std::shared_ptr<Observer<T>> observer, int priority = 0);
    void detach(std::shared_ptr<Observer<T>> observer);
    size_t observerCount() const;
    size_t activeObserverCount() const;
    
protected:
    void notify(const T& data);
    void notifyError(const std::string& error);
    void cleanupExpired();
};

template<typename T>
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const T& data) = 0;
    virtual void onError(const std::string& error) {}
    virtual void onComplete() {}
    virtual std::string getName() const = 0;
};

struct WeatherData {
    double temperature;
    double humidity;
    double pressure;
    std::chrono::system_clock::time_point timestamp;
};

class WeatherStation : public Subject<WeatherData> {
private:
    WeatherData currentData_;
    
public:
    void setMeasurements(double temp, double humidity, double pressure);
    WeatherData getCurrentData() const;
};
```

### 🎯 Challenge Features

1. **Priority-based notification**: Higher priority observers notified first
2. **Filtered observers**: Observers can subscribe to specific event types only
3. **Batch notifications**: Collect multiple changes, notify once
4. **Notification threading**: Notify observers in parallel (optional)
5. **Observer resurrection**: Handle case where observer is destroyed during notification

### ✅ Test Scenarios

```cpp
int main() {
    WeatherStation station;
    
    // Create observers with different priorities
    auto logger = std::make_shared<LoggingObserver>("Logger", 100);
    auto stats = std::make_shared<StatisticsObserver>("Stats", 50);
    auto alerts = std::make_shared<AlertObserver>("Alerts", 200);
    
    station.attach(logger, logger->getPriority());
    station.attach(stats, stats->getPriority());
    station.attach(alerts, alerts->getPriority());
    
    std::cout << "Active observers: " << station.activeObserverCount() << "\n";
    
    // Update weather
    station.setMeasurements(25.5, 65.0, 1013.25);
    
    // Destroy one observer
    {
        auto temp = std::make_shared<LoggingObserver>("Temp", 10);
        station.attach(temp, 10);
        std::cout << "With temp observer: " << station.activeObserverCount() << "\n";
    }
    
    // Next update automatically cleans up expired observer
    station.setMeasurements(26.0, 70.0, 1012.0);
    std::cout << "After cleanup: " << station.activeObserverCount() << "\n";
    
    return 0;
}
```

### 🎓 Learning Outcomes
- Observer pattern with `weak_ptr`
- Automatic cleanup of expired observers
- Breaking circular dependencies
- Thread-safe notification
- Priority-based ordering

---

## Assignment 3: Resource Cache with Weak References (Advanced)

### 🎯 Objective
Implement a sophisticated caching system that uses `weak_ptr` to cache expensive resources without preventing their destruction when no longer needed.

### 📋 Requirements

1. Create a `Resource` base class template that:
   - Has creation timestamp
   - Tracks access count
   - Has virtual `load()` and `unload()` methods
   - Measures load time

2. Implement concrete resource types:
   - `ImageResource` - simulates image loading (sleep)
   - `AudioResource` - simulates audio loading
   - `ModelResource` - simulates 3D model loading
   - Each with different load times and memory footprints

3. Create a `ResourceCache` class template that:
   - Stores resources as `weak_ptr` (key → weak_ptr)
   - Implements LRU (Least Recently Used) eviction policy
   - Has configurable max memory limit
   - Provides factory method: `getOrCreate(key, factory)`
   - Automatically removes expired entries
   - Tracks cache statistics (hits, misses, evictions)
   - Thread-safe

4. Implement a `ResourceLoader` class that:
   - Loads resources asynchronously
   - Uses cache when available
   - Provides progress callbacks
   - Handles load failures

5. Create a `ResourcePool` that:
   - Pre-loads common resources
   - Manages resource priorities
   - Implements smart prefetching

### 💻 Advanced Cache Design

```cpp
template<typename Key, typename Value>
class ResourceCache {
private:
    struct CacheEntry {
        std::weak_ptr<Value> resource;
        std::chrono::system_clock::time_point lastAccess;
        size_t accessCount;
        size_t memorySize;
    };
    
    std::map<Key, CacheEntry> cache_;
    mutable std::mutex mutex_;
    
    // Statistics
    size_t hits_;
    size_t misses_;
    size_t evictions_;
    size_t totalMemory_;
    size_t maxMemory_;
    
    // LRU tracking
    std::list<Key> lruList_;
    std::map<Key, typename std::list<Key>::iterator> lruMap_;
    
public:
    explicit ResourceCache(size_t maxMemory);
    
    // Get or create resource
    template<typename Factory>
    std::shared_ptr<Value> getOrCreate(const Key& key, Factory factory);
    
    // Cache operations
    bool contains(const Key& key) const;
    void invalidate(const Key& key);
    void clear();
    void cleanupExpired();
    
    // LRU operations
    void touch(const Key& key);
    void evictLRU();
    
    // Statistics
    struct Stats {
        size_t hits;
        size_t misses;
        size_t evictions;
        size_t activeEntries;
        size_t expiredEntries;
        size_t totalMemory;
        double hitRate() const { 
            return (hits + misses > 0) ? 
                   static_cast<double>(hits) / (hits + misses) : 0.0;
        }
    };
    
    Stats getStats() const;
    void resetStats();
    
private:
    bool hasSpace(size_t requiredSize) const;
    void makeSpace(size_t requiredSize);
    size_t calculateMemoryUsage() const;
};

// Resource interface
class Resource {
protected:
    std::string id_;
    size_t memorySize_;
    std::chrono::system_clock::time_point createdAt_;
    size_t accessCount_;
    
public:
    explicit Resource(std::string id, size_t size);
    virtual ~Resource();
    
    virtual void load() = 0;
    virtual void unload() = 0;
    virtual std::string getType() const = 0;
    
    size_t getMemorySize() const { return memorySize_; }
    size_t getAccessCount() const { return accessCount_; }
    void incrementAccess() { ++accessCount_; }
};

class ImageResource : public Resource {
private:
    int width_, height_;
    std::vector<uint8_t> data_;
    
public:
    ImageResource(std::string id, int width, int height);
    void load() override;
    void unload() override;
    std::string getType() const override { return "Image"; }
};
```

### 🎯 Advanced Features

1. **Time-based expiration**: Resources expire after TTL
2. **Memory pressure handling**: Automatic eviction under memory pressure
3. **Prefetching**: Predictive loading based on access patterns
4. **Hierarchical caching**: L1 (hot) and L2 (warm) cache levels
5. **Cache warming**: Pre-load common resources at startup
6. **Sharding**: Multiple cache instances for scalability

### ✅ Test Scenarios

```cpp
int main() {
    // Create cache with 10MB limit
    ResourceCache<std::string, ImageResource> cache(10 * 1024 * 1024);
    
    // Load resources
    auto img1 = cache.getOrCreate("image1.png", []() {
        return std::make_shared<ImageResource>("image1.png", 1920, 1080);
    });
    
    auto img2 = cache.getOrCreate("image2.png", []() {
        return std::make_shared<ImageResource>("image2.png", 1280, 720);
    });
    
    // Cache hit
    auto img1_again = cache.getOrCreate("image1.png", []() {
        return std::make_shared<ImageResource>("image1.png", 1920, 1080);
    });
    
    assert(img1.get() == img1_again.get());  // Same instance!
    
    std::cout << "Cache stats:\n";
    auto stats = cache.getStats();
    std::cout << "  Hits: " << stats.hits << "\n";
    std::cout << "  Misses: " << stats.misses << "\n";
    std::cout << "  Hit rate: " << stats.hitRate() << "\n";
    
    // Release strong references
    img1.reset();
    img2.reset();
    
    // Resources still in cache (weak_ptr)
    std::cout << "Active entries: " << cache.getStats().activeEntries << "\n";
    
    // But can be reclaimed when needed
    cache.cleanupExpired();
    
    return 0;
}
```

### 🎓 Learning Outcomes
- `weak_ptr` for caching without ownership
- LRU eviction policy implementation
- Memory management strategies
- Thread-safe cache implementation
- Factory pattern with smart pointers
- Performance optimization with caching

---

## Assignment 4: Shared Connection Pool (Advanced)

### 🎯 Objective
Build a thread-safe connection pool that manages database connections using `shared_ptr` with custom deleters to return connections to the pool automatically.

### 📋 Requirements

1. Create a `Connection` class that:
   - Simulates database connection (has ID, server, status)
   - Has methods: `execute()`, `beginTransaction()`, `commit()`, `rollback()`
   - Tracks usage statistics (queries executed, transaction count)
   - Times out after inactivity
   - Can be in states: Available, InUse, Broken

2. Implement a `ConnectionPool` class that:
   - Pre-creates configurable number of connections
   - Returns `shared_ptr<Connection>` with custom deleter
   - Custom deleter returns connection to pool
   - Validates connections before reuse
   - Recreates broken connections
   - Implements connection timeout
   - Thread-safe (supports concurrent access)
   - Has max pool size and waiting queue
   - Provides health checks

3. Create a `PooledConnection` wrapper that:
   - Wraps `shared_ptr<Connection>`
   - Provides RAII guarantee for transactions
   - Auto-rollback on exception
   - Tracks connection lifetime

4. Implement a `ConnectionPoolManager` that:
   - Manages multiple pools (one per database)
   - Load balances across pools
   - Monitors pool health
   - Provides statistics dashboard

5. Add `ConnectionGuard` RAII wrapper for:
   - Automatic transaction management
   - Query timeout enforcement
   - Automatic cleanup on exceptions

### 💻 Advanced Pool Design

```cpp
class Connection {
public:
    enum class State { Available, InUse, Broken };
    
private:
    int id_;
    std::string server_;
    State state_;
    std::chrono::system_clock::time_point lastUsed_;
    size_t queryCount_;
    bool inTransaction_;
    
public:
    Connection(int id, std::string server);
    ~Connection();
    
    void execute(const std::string& query);
    void beginTransaction();
    void commit();
    void rollback();
    
    bool isHealthy() const;
    void markBroken();
    State getState() const { return state_; }
    void setState(State state) { state_ = state; }
    
    std::chrono::seconds idleTime() const;
};

class ConnectionPool : public std::enable_shared_from_this<ConnectionPool> {
private:
    struct PoolConfig {
        size_t minSize;
        size_t maxSize;
        std::chrono::seconds timeout;
        std::chrono::seconds idleTimeout;
        bool validateOnAcquire;
    };
    
    std::string poolName_;
    PoolConfig config_;
    std::vector<std::unique_ptr<Connection>> connections_;
    std::vector<Connection*> available_;
    std::set<Connection*> inUse_;
    
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
    // Statistics
    size_t totalAcquired_;
    size_t totalReleased_;
    size_t totalCreated_;
    size_t totalBroken_;
    size_t waitCount_;
    
public:
    explicit ConnectionPool(std::string name, PoolConfig config);
    ~ConnectionPool();
    
    // Acquire returns shared_ptr with custom deleter
    std::shared_ptr<Connection> acquire(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    // Statistics
    struct PoolStats {
        size_t totalConnections;
        size_t availableConnections;
        size_t inUseConnections;
        size_t brokenConnections;
        size_t waitingThreads;
        double utilizationRate;
    };
    
    PoolStats getStats() const;
    void healthCheck();
    void clear();
    
private:
    void release(Connection* conn);  // Called by custom deleter
    Connection* createConnection();
    bool validateConnection(Connection* conn);
    void removeConnection(Connection* conn);
    
    // Custom deleter for shared_ptr
    struct ConnectionDeleter {
        std::weak_ptr<ConnectionPool> pool;
        
        void operator()(Connection* conn) {
            if (auto p = pool.lock()) {
                p->release(conn);
            } else {
                // Pool destroyed, just mark connection
                conn->setState(Connection::State::Broken);
            }
        }
    };
    
    friend struct ConnectionDeleter;
};

// RAII Transaction guard
class TransactionGuard {
private:
    std::shared_ptr<Connection> conn_;
    bool committed_;
    
public:
    explicit TransactionGuard(std::shared_ptr<Connection> conn);
    ~TransactionGuard();
    
    void commit();
    void rollback();
};
```

### 🎯 Advanced Features

1. **Connection validation**: Health checks before reuse
2. **Automatic reconnection**: Recreate broken connections
3. **Wait queue**: Queue threads waiting for connections
4. **Connection timeout**: Reclaim idle connections
5. **Pool resizing**: Dynamic grow/shrink based on load
6. **Monitoring**: Real-time statistics and alerts
7. **Load balancing**: Distribute across multiple pools

### ✅ Test Scenarios

```cpp
int main() {
    // Create pool
    ConnectionPool::PoolConfig config{
        .minSize = 5,
        .maxSize = 20,
        .timeout = std::chrono::seconds(30),
        .idleTimeout = std::chrono::seconds(300),
        .validateOnAcquire = true
    };
    
    auto pool = std::make_shared<ConnectionPool>("MainDB", config);
    
    // Single threaded usage
    {
        auto conn = pool->acquire();
        TransactionGuard guard(conn);
        
        conn->execute("INSERT INTO users VALUES (...)");
        conn->execute("UPDATE stats SET ...");
        
        guard.commit();
        // Connection automatically returned to pool when conn destroyed
    }
    
    // Multi-threaded usage
    std::vector<std::thread> threads;
    for (int i = 0; i < 50; ++i) {
        threads.emplace_back([pool, i]() {
            for (int j = 0; j < 10; ++j) {
                auto conn = pool->acquire();
                conn->execute("SELECT * FROM users WHERE id = " + 
                             std::to_string(i * 10 + j));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Print statistics
    auto stats = pool->getStats();
    std::cout << "Pool Statistics:\n";
    std::cout << "  Total: " << stats.totalConnections << "\n";
    std::cout << "  Available: " << stats.availableConnections << "\n";
    std::cout << "  In Use: " << stats.inUseConnections << "\n";
    std::cout << "  Utilization: " << stats.utilizationRate << "%\n";
    
    return 0;
}
```

### 🎓 Learning Outcomes
- Custom deleters for automatic resource return
- Thread-safe connection pooling
- RAII for transaction management
- Resource lifecycle management
- Condition variables for waiting
- `enable_shared_from_this` for callbacks
- Performance optimization through pooling

---

## Assignment 5: Graph with Shared Node Ownership (Advanced)

### 🎯 Objective
Implement a directed graph where nodes can be shared between multiple graphs and edges, using `shared_ptr` for node ownership and `weak_ptr` to prevent cycles.

### 📋 Requirements

1. Create a `GraphNode` class that:
   - Has unique ID and data (template)
   - Stores outgoing edges as `weak_ptr<GraphNode>`
   - Uses `enable_shared_from_this`
   - Has methods: `addEdge()`, `removeEdge()`, `getNeighbors()`
   - Tracks incoming edge count
   - Can detect cycles

2. Implement a `Graph` class template that:
   - Stores nodes as `shared_ptr<GraphNode<T>>`
   - Supports directed/undirected modes
   - Has methods:
     - `addNode()`, `removeNode()`
     - `addEdge()`, `removeEdge()`
     - `findNode()`, `hasEdge()`
     - `getNeighbors()`
   - Implements traversal algorithms (DFS, BFS)
   - Detects and prevents cycles (for DAGs)
   - Supports graph merging (combines two graphs)

3. Create `GraphAlgorithms` utility class with:
   - Shortest path (Dijkstra)
   - Topological sort
   - Strongly connected components
   - Cycle detection
   - Graph coloring

4. Implement `GraphView` that:
   - Provides filtered view of graph
   - Uses `weak_ptr` to reference original nodes
   - Doesn't own nodes
   - Can represent subgraphs

5. Create `GraphSerializer` that:
   - Serializes graph to JSON/XML
   - Handles shared node references
   - Deserializes back to graph

### 💻 Complex Graph Design

```cpp
template<typename T>
class GraphNode : public std::enable_shared_from_this<GraphNode<T>> {
public:
    struct Edge {
        std::weak_ptr<GraphNode<T>> target;
        double weight;
        std::string label;
        
        std::shared_ptr<GraphNode<T>> lock() const {
            return target.lock();
        }
        
        bool expired() const {
            return target.expired();
        }
    };
    
private:
    int id_;
    T data_;
    std::vector<Edge> outgoingEdges_;
    std::atomic<size_t> incomingCount_;
    
public:
    GraphNode(int id, T data);
    ~GraphNode();
    
    void addEdge(std::shared_ptr<GraphNode<T>> target, 
                 double weight = 1.0,
                 std::string label = "");
    
    void removeEdge(std::shared_ptr<GraphNode<T>> target);
    
    std::vector<std::shared_ptr<GraphNode<T>>> getNeighbors() const;
    
    const std::vector<Edge>& getEdges() const { return outgoingEdges_; }
    
    void cleanupExpiredEdges();
    
    int getId() const { return id_; }
    const T& getData() const { return data_; }
    void setData(const T& data) { data_ = data; }
    
    size_t outDegree() const;
    size_t inDegree() const { return incomingCount_.load(); }
};

template<typename T>
class Graph {
public:
    enum class Type { Directed, Undirected };
    
private:
    Type type_;
    std::map<int, std::shared_ptr<GraphNode<T>>> nodes_;
    int nextId_;
    
public:
    explicit Graph(Type type = Type::Directed);
    
    // Node operations
    std::shared_ptr<GraphNode<T>> addNode(const T& data);
    bool removeNode(int nodeId);
    std::shared_ptr<GraphNode<T>> getNode(int nodeId) const;
    
    // Edge operations
    bool addEdge(int fromId, int toId, double weight = 1.0);
    bool addEdge(std::shared_ptr<GraphNode<T>> from,
                 std::shared_ptr<GraphNode<T>> to,
                 double weight = 1.0);
    bool removeEdge(int fromId, int toId);
    bool hasEdge(int fromId, int toId) const;
    
    // Traversal
    template<typename Visitor>
    void depthFirstSearch(int startId, Visitor visitor);
    
    template<typename Visitor>
    void breadthFirstSearch(int startId, Visitor visitor);
    
    // Algorithms
    std::vector<int> shortestPath(int fromId, int toId);
    std::vector<int> topologicalSort();  // For DAG
    bool hasCycle() const;
    
    // Graph operations
    void merge(const Graph<T>& other);
    Graph<T> subgraph(const std::vector<int>& nodeIds) const;
    void cleanup();  // Remove expired edges
    
    // Statistics
    size_t nodeCount() const { return nodes_.size(); }
    size_t edgeCount() const;
    
    std::map<int, std::shared_ptr<GraphNode<T>>>& getNodes() { 
        return nodes_; 
    }
};

// Graph view - doesn't own nodes
template<typename T>
class GraphView {
private:
    std::map<int, std::weak_ptr<GraphNode<T>>> nodes_;
    std::function<bool(const GraphNode<T>&)> filter_;
    
public:
    explicit GraphView(const Graph<T>& graph,
                      std::function<bool(const GraphNode<T>&)> filter = nullptr);
    
    std::vector<std::shared_ptr<GraphNode<T>>> getNodes() const;
    bool contains(int nodeId) const;
    void refresh(const Graph<T>& graph);
};
```

### 🎯 Challenge Features

1. **Cycle prevention**: Detect and prevent cycles when adding edges (for DAG mode)
2. **Graph merging**: Combine two graphs, sharing common nodes
3. **Incremental updates**: Efficiently update derived data (path cache) on changes
4. **Concurrent access**: Thread-safe graph operations
5. **Memory efficiency**: Use `weak_ptr` where ownership not needed
6. **Subgraph extraction**: Create views without copying nodes

### ✅ Test Scenarios

```cpp
int main() {
    // Create graph
    Graph<std::string> graph(Graph<std::string>::Type::Directed);
    
    // Add nodes
    auto node1 = graph.addNode("A");
    auto node2 = graph.addNode("B");
    auto node3 = graph.addNode("C");
    auto node4 = graph.addNode("D");
    
    // Add edges
    graph.addEdge(node1, node2, 1.0);
    graph.addEdge(node2, node3, 2.0);
    graph.addEdge(node1, node3, 4.0);
    graph.addEdge(node3, node4, 1.0);
    
    // Test shared ownership
    std::cout << "Node1 ref count: " << node1.use_count() << "\n";  // In graph + node1
    
    // Traverse
    std::cout << "DFS from A:\n";
    graph.depthFirstSearch(node1->getId(), [](const GraphNode<std::string>& node) {
        std::cout << "  Visiting: " << node.getData() << "\n";
    });
    
    // Find shortest path
    auto path = graph.shortestPath(node1->getId(), node4->getId());
    std::cout << "Shortest path: ";
    for (int id : path) {
        std::cout << graph.getNode(id)->getData() << " ";
    }
    std::cout << "\n";
    
    // Create subgraph
    auto subgraph = graph.subgraph({node1->getId(), node2->getId()});
    std::cout << "Subgraph nodes: " << subgraph.nodeCount() << "\n";
    
    // Test cleanup
    node2.reset();  // Release reference
    graph.cleanup();  // Clean up expired edges
    
    return 0;
}
```

### 🎓 Learning Outcomes
- Complex ownership patterns with `shared_ptr` and `weak_ptr`
- Cycle prevention in graph structures
- `enable_shared_from_this` for self-references
- Graph algorithms with smart pointers
- Resource sharing across data structures
- Memory-efficient graph views

---

## Assignment 6: Async Task Scheduler with Shared State (Advanced)

### 🎯 Objective
Build an asynchronous task scheduler where tasks share state through `shared_ptr` and use `weak_ptr` for cancellation and timeout management.

### 📋 Requirements

1. Create a `Task` class that:
   - Inherits from `enable_shared_from_this<Task>`
   - Has unique ID, priority, dependencies
   - Stores callable (function/lambda)
   - Tracks state (Pending, Running, Completed, Failed, Cancelled)
   - Can be cancelled
   - Supports timeout
   - Returns result via `std::shared_ptr<Result>`

2. Implement a `TaskScheduler` that:
   - Thread pool for execution (configurable size)
   - Task queue with priority ordering
   - Tracks tasks with `weak_ptr` (allows external cancellation)
   - Respects task dependencies
   - Handles task timeout
   - Provides progress tracking
   - Supports task groups

3. Create a `TaskHandle` wrapper that:
   - Wraps `shared_ptr<Task>`
   - Provides cancellation interface
   - Allows waiting for completion
   - Provides result access
   - Auto-cancels on destruction (optional)

4. Implement `TaskGroup` for:
   - Batch task submission
   - Wait for all/any completion
   - Collective cancellation
   - Progress aggregation

5. Create `SharedTaskState` for:
   - State shared between multiple tasks
   - Thread-safe access
   - Automatic cleanup when last task completes

### 💻 Scheduler Design

```cpp
class Task : public std::enable_shared_from_this<Task> {
public:
    enum class State { Pending, Running, Completed, Failed, Cancelled };
    
    using TaskFunction = std::function<std::any()>;
    using Callback = std::function<void(std::shared_ptr<Task>)>;
    
private:
    int id_;
    std::string name_;
    TaskFunction function_;
    int priority_;
    State state_;
    std::any result_;
    std::exception_ptr exception_;
    
    std::vector<std::weak_ptr<Task>> dependencies_;
    std::chrono::milliseconds timeout_;
    std::chrono::system_clock::time_point startTime_;
    
    Callback onComplete_;
    Callback onError_;
    Callback onCancel_;
    
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> cancelRequested_;
    
public:
    Task(int id, std::string name, TaskFunction func, int priority = 0);
    
    // Execution
    void execute();
    void cancel();
    bool isCancelled() const;
    bool isCompleted() const;
    bool canExecute() const;  // Check dependencies
    
    // Result access
    template<typename T>
    T getResult() const;
    
    State getState() const;
    
    // Dependencies
    void addDependency(std::shared_ptr<Task> dep);
    void removeDependency(std::shared_ptr<Task> dep);
    std::vector<std::shared_ptr<Task>> getDependencies() const;
    
    // Callbacks
    void setOnComplete(Callback cb) { onComplete_ = std::move(cb); }
    void setOnError(Callback cb) { onError_ = std::move(cb); }
    void setOnCancel(Callback cb) { onCancel_ = std::move(cb); }
    
    // Timeout
    void setTimeout(std::chrono::milliseconds timeout) { timeout_ = timeout; }
    bool hasTimedOut() const;
    
    int getId() const { return id_; }
    int getPriority() const { return priority_; }
};

class TaskScheduler {
private:
    struct TaskCompare {
        bool operator()(const std::weak_ptr<Task>& a,
                       const std::weak_ptr<Task>& b) const;
    };
    
    std::priority_queue<std::weak_ptr<Task>,
                       std::vector<std::weak_ptr<Task>>,
                       TaskCompare> taskQueue_;
    
    std::vector<std::thread> workers_;
    std::map<int, std::weak_ptr<Task>> activeTasks_;
    
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::atomic<bool> shutdown_;
    
    size_t threadCount_;
    int nextTaskId_;
    
    // Statistics
    std::atomic<size_t> completedTasks_;
    std::atomic<size_t> failedTasks_;
    std::atomic<size_t> cancelledTasks_;
    
public:
    explicit TaskScheduler(size_t threadCount = std::thread::hardware_concurrency());
    ~TaskScheduler();
    
    // Task submission
    std::shared_ptr<Task> submit(Task::TaskFunction func,
                                 std::string name = "",
                                 int priority = 0);
    
    std::shared_ptr<Task> submitWithDependencies(
        Task::TaskFunction func,
        std::vector<std::shared_ptr<Task>> dependencies,
        std::string name = "",
        int priority = 0);
    
    // Task control
    bool cancelTask(int taskId);
    void cancelAll();
    void waitForAll();
    
    // Status
    size_t queueSize() const;
    size_t activeTaskCount() const;
    
    struct Statistics {
        size_t completed;
        size_t failed;
        size_t cancelled;
        size_t queued;
        size_t active;
    };
    
    Statistics getStatistics() const;
    
private:
    void workerThread();
    std::shared_ptr<Task> getNextTask();
    void executeTask(std::shared_ptr<Task> task);
    void cleanupExpired();
};

// Task handle with RAII
class TaskHandle {
private:
    std::shared_ptr<Task> task_;
    bool autoCancelOnDestroy_;
    
public:
    TaskHandle(std::shared_ptr<Task> task, bool autoCancel = false);
    ~TaskHandle();
    
    void cancel();
    void wait();
    bool waitFor(std::chrono::milliseconds timeout);
    
    template<typename T>
    T get();  // Wait and get result
    
    Task::State getState() const;
    bool isCompleted() const;
};

// Task group for batch operations
class TaskGroup {
private:
    std::vector<std::shared_ptr<Task>> tasks_;
    std::string name_;
    
public:
    explicit TaskGroup(std::string name = "");
    
    void addTask(std::shared_ptr<Task> task);
    void cancelAll();
    void waitForAll();
    bool waitForAny();
    
    size_t completedCount() const;
    size_t totalCount() const { return tasks_.size(); }
    double progress() const;
    
    std::vector<std::shared_ptr<Task>> getCompletedTasks() const;
    std::vector<std::shared_ptr<Task>> getFailedTasks() const;
};
```

### ✅ Test Scenarios

```cpp
int main() {
    TaskScheduler scheduler(4);  // 4 worker threads
    
    // Simple task
    auto task1 = scheduler.submit([]() -> std::any {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 42;
    }, "SimpleTask");
    
    // Task with dependencies
    auto task2 = scheduler.submit([]() -> std::any {
        return std::string("Hello");
    }, "Task2");
    
    auto task3 = scheduler.submitWithDependencies(
        [task2]() -> std::any {
            auto result = task2->getResult<std::string>();
            return result + " World!";
        },
        {task2},
        "Task3"
    );
    
    // Task group
    TaskGroup group("BatchProcessing");
    for (int i = 0; i < 10; ++i) {
        auto task = scheduler.submit([i]() -> std::any {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            return i * i;
        }, "Task_" + std::to_string(i));
        
        group.addTask(task);
    }
    
    // Wait with progress
    while (group.progress() < 1.0) {
        std::cout << "Progress: " << (group.progress() * 100) << "%\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Get results
    std::cout << "Task1 result: " << task1->getResult<int>() << "\n";
    std::cout << "Task3 result: " << task3->getResult<std::string>() << "\n";
    
    // Statistics
    auto stats = scheduler.getStatistics();
    std::cout << "Completed: " << stats.completed << "\n";
    std::cout << "Failed: " << stats.failed << "\n";
    
    return 0;
}
```

### 🎓 Learning Outcomes
- Complex async patterns with `shared_ptr`
- `enable_shared_from_this` for callbacks
- `weak_ptr` for cancellation without ownership
- Thread-safe shared state management
- RAII for async resource management
- Dependency management with smart pointers

---

## 🎯 Grading Rubric

Each assignment is evaluated on:

| Criteria | Points | Description |
|----------|--------|-------------|
| **Correctness** | 35% | Code works correctly, handles edge cases |
| **Smart Pointer Usage** | 25% | Proper use of shared_ptr/weak_ptr/enable_shared_from_this |
| **Thread Safety** | 15% | Correct synchronization where needed |
| **Design Quality** | 10% | Clean architecture, good abstractions |
| **Performance** | 10% | Efficient implementation, minimal overhead |
| **Code Quality** | 5% | Readable, well-documented |

## 📚 Bonus Challenges

For each assignment, try these extra challenges:

1. **Add comprehensive metrics and logging**
2. **Implement serialization/deserialization**
3. **Add extensive unit tests with edge cases**
4. **Perform memory leak detection** (Valgrind, ASan)
5. **Benchmark performance** against alternative approaches
6. **Add visualization** of object relationships
7. **Implement thread-safe versions** where not already required
8. **Create Python/CLI interface** for testing

## 🚀 Advanced Bonus Assignment

**Multi-Level Cache with Shared Ownership**: Combine concepts from multiple assignments to create a multi-level caching system where:
- L1 cache: Hot data (strong ownership)
- L2 cache: Warm data (weak_ptr)
- L3 cache: Cold data (serialized to disk)
- Automatic promotion/demotion between levels
- Shared across multiple subsystems
- Thread-safe with minimal lock contention
- Support for cache coherency protocols

This tests your complete understanding of shared_ptr, weak_ptr, and complex ownership patterns!

