# Abstract Factory Design Pattern

## What Is It? (Simple Explanation)

Imagine you walk into an IKEA store. You want to furnish a living room. IKEA offers complete **furniture families**:

- **Modern style**: Modern sofa + Modern table + Modern lamp
- **Classic style**: Classic sofa + Classic table + Classic lamp
- **Minimalist style**: Minimalist sofa + Minimalist table + Minimalist lamp

You pick **one style** and everything matches — you never accidentally mix a Classic lamp with a Modern sofa.

**Abstract Factory is exactly this**: a factory that produces a *family* of related objects that are designed to work together, without you knowing the exact concrete class being created.

---

## Simple Definition

> **Abstract Factory** provides an interface for creating **families of related objects** without specifying their concrete classes.

---

## The Key Difference: Factory Method vs Abstract Factory

| Factory Method | Abstract Factory |
|---|---|
| Creates **one type** of object | Creates **families** of related objects |
| One factory method | Multiple factory methods (one per product type) |
| "Give me a button" | "Give me a button AND a checkbox AND a text field that all match" |

---

## Real-World Example 1: Cross-Platform UI (GUI)

You're building an app that runs on Windows, macOS, and Linux. Each platform has its own look for UI widgets, but your app code should not care which platform it's on.

```
Abstract Products:
  Button      → WindowsButton   / MacButton   / LinuxButton
  Checkbox    → WindowsCheckbox / MacCheckbox / LinuxCheckbox
  TextField   → WindowsTextField/ MacTextField/ LinuxTextField

Abstract Factory:
  GUIFactory  → WindowsFactory  / MacFactory  / LinuxFactory
```

### Code Walkthrough

```cpp
// ─── STEP 1: Define Abstract Products ───────────────────────────────────────

class Button {
public:
    virtual ~Button() = default;
    virtual void render() = 0;         // what it does, not how
    virtual void onClick() = 0;
};

class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual void render() = 0;
    virtual void onToggle() = 0;
};

// ─── STEP 2: Implement Concrete Products (Windows family) ───────────────────

class WindowsButton : public Button {
public:
    void render() override {
        std::cout << "[Windows] Rendering a flat, square button\n";
    }
    void onClick() override {
        std::cout << "[Windows] Click animation: highlight border\n";
    }
};

class WindowsCheckbox : public Checkbox {
public:
    void render() override {
        std::cout << "[Windows] Rendering a square checkbox\n";
    }
    void onToggle() override {
        std::cout << "[Windows] Toggle: fill square with checkmark\n";
    }
};

// ─── STEP 3: Mac family ──────────────────────────────────────────────────────

class MacButton : public Button {
public:
    void render() override {
        std::cout << "[Mac] Rendering a rounded, glossy button\n";
    }
    void onClick() override {
        std::cout << "[Mac] Click animation: subtle glow\n";
    }
};

class MacCheckbox : public Checkbox {
public:
    void render() override {
        std::cout << "[Mac] Rendering a rounded checkbox\n";
    }
    void onToggle() override {
        std::cout << "[Mac] Toggle: smooth tick animation\n";
    }
};

// ─── STEP 4: Define the Abstract Factory ────────────────────────────────────

class GUIFactory {
public:
    virtual ~GUIFactory() = default;
    virtual std::unique_ptr<Button>   createButton()   = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() = 0;
};

// ─── STEP 5: Implement Concrete Factories ───────────────────────────────────

class WindowsFactory : public GUIFactory {
public:
    std::unique_ptr<Button>   createButton()   override {
        return std::make_unique<WindowsButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() override {
        return std::make_unique<WindowsCheckbox>();
    }
};

class MacFactory : public GUIFactory {
public:
    std::unique_ptr<Button>   createButton()   override {
        return std::make_unique<MacButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() override {
        return std::make_unique<MacCheckbox>();
    }
};

// ─── STEP 6: Client Code — doesn't know/care about Windows or Mac ───────────

class Application {
    std::unique_ptr<Button>   btn_;
    std::unique_ptr<Checkbox> chk_;
public:
    explicit Application(std::unique_ptr<GUIFactory> factory) {
        btn_ = factory->createButton();
        chk_ = factory->createCheckbox();
    }
    void renderUI() {
        btn_->render();
        chk_->render();
    }
};

// ─── STEP 7: Composition Root — the only place that knows the platform ───────

int main() {
    std::unique_ptr<GUIFactory> factory;

#ifdef _WIN32
    factory = std::make_unique<WindowsFactory>();
#elif __APPLE__
    factory = std::make_unique<MacFactory>();
#endif

    Application app(std::move(factory));
    app.renderUI();
}
```

**Output on macOS:**
```
[Mac] Rendering a rounded, glossy button
[Mac] Rendering a rounded checkbox
```

Notice: `Application` has **zero** `#ifdef` or `if/else`. It just uses the factory it was given.

---

## Real-World Example 2: Database Connectors

```
Abstract Products:
  Connection  → MySQLConnection   / PostgreSQLConnection / SQLiteConnection
  Command     → MySQLCommand      / PostgreSQLCommand    / SQLiteCommand
  DataReader  → MySQLDataReader   / PostgreSQLDataReader / SQLiteDataReader

Abstract Factory:
  DatabaseFactory → MySQLFactory / PostgreSQLFactory / SQLiteFactory
```

```cpp
// Abstract products
struct Connection {
    virtual void open(const std::string& dsn) = 0;
    virtual void close() = 0;
    virtual ~Connection() = default;
};

struct Command {
    virtual void execute(const std::string& sql) = 0;
    virtual ~Command() = default;
};

// Abstract factory
struct DatabaseFactory {
    virtual std::unique_ptr<Connection> createConnection() = 0;
    virtual std::unique_ptr<Command>    createCommand()    = 0;
    virtual ~DatabaseFactory() = default;
};

// Concrete: MySQL
struct MySQLConnection : Connection {
    void open(const std::string& dsn) override {
        std::cout << "[MySQL] Connected to: " << dsn << "\n";
    }
    void close() override { std::cout << "[MySQL] Connection closed\n"; }
};

struct MySQLCommand : Command {
    void execute(const std::string& sql) override {
        std::cout << "[MySQL] Executing: " << sql << "\n";
    }
};

struct MySQLFactory : DatabaseFactory {
    std::unique_ptr<Connection> createConnection() override {
        return std::make_unique<MySQLConnection>();
    }
    std::unique_ptr<Command> createCommand() override {
        return std::make_unique<MySQLCommand>();
    }
};
```

Switching from MySQL to PostgreSQL = change **one line** at the composition root.

---

## Structure Diagram

```
         ┌─────────────────────┐
         │   «interface»       │
         │   GUIFactory        │
         │ + createButton()    │
         │ + createCheckbox()  │
         └────────┬────────────┘
                  │ implements
       ┌──────────┼──────────┐
       ▼          ▼          ▼
 WindowsFactory  MacFactory  LinuxFactory
       │          │          │
       │ creates  │ creates  │ creates
       ▼          ▼          ▼
 WinButton    MacButton   LinuxButton    ← all implement «Button»
 WinCheckbox  MacCheckbox LinuxCheckbox  ← all implement «Checkbox»
```

---

## When To Use Abstract Factory

| Signal | Example |
|---|---|
| Your system must be independent of how its products are created | Don't want `new WindowsButton()` scattered everywhere |
| You need to enforce that products from the same family are used together | Can't mix Dark-theme button with Light-theme checkbox |
| You want to swap entire product families at runtime or config time | Switch DB from MySQL → PostgreSQL in config file |
| You're building a library/framework where users supply their own implementations | Plugin system |

---

## When NOT To Use It

- When you only have **one** product type (use Factory Method instead)
- When product families are unlikely to change or expand
- When the added indirection/complexity is not justified

---

## Pros and Cons

### Pros
- **Consistency**: Products in a family are always compatible with each other
- **Open/Closed Principle**: Add new families without modifying existing client code
- **Single Responsibility**: Creation logic is isolated in factories
- **Easy to swap**: Change one factory → entire product family changes

### Cons
- **Hard to add new product types**: Adding a new product (e.g., `Slider`) requires changing every concrete factory
- **Complexity**: More classes and interfaces → harder to understand at first
- **Over-engineering risk**: Not worth it for simple, single-product scenarios

---

## Programming Assignments

---

### MEDIUM — Assignment 1: Complete the GUI Factory (Real-World Platform UI)

**Scenario**: You are building a settings application that must run natively on Windows, macOS, and Linux. The UI components must look and behave according to each platform's guidelines.

**Task**:
1. Complete `WindowsButton`, `WindowsCheckbox`, `WindowsTextField` implementations
2. Complete `MacButton`, `MacCheckbox`, `MacTextField` implementations
3. Complete `LinuxButton`, `LinuxCheckbox`, `LinuxTextField` implementations
4. Implement `WindowsFactory`, `MacFactory`, `LinuxFactory`
5. Hook up the platform detection in `main()` using `#ifdef`
6. The `Application` class must **never** use `#ifdef` or know the platform

**Extension**: Add a `Menu` widget to all three families. Notice the impact — you must touch every factory. Reflect on why this is a known weakness of the pattern.

---

### MEDIUM — Assignment 2: Theme System

**Scenario**: A design system library needs Light and Dark themes. Each theme has a consistent `PrimaryColor`, `BackgroundColor`, `Font`, and `Icon`.

**Task**:
1. Define abstract products: `Color` (with `getHex()`), `Font` (with `getFamily()`, `getSize()`), `Icon` (with `display()`)
2. Implement `LightTheme` and `DarkTheme` product families
3. Implement `ThemeFactory` abstract factory with `createColor()`, `createFont()`, `createIcon()`
4. Implement `LightThemeFactory` and `DarkThemeFactory`
5. A `UIComponent` class receives a factory and renders itself using consistent theme objects
6. Allow switching theme via a config string `"dark"` or `"light"` — no `if/else` in `UIComponent`

---

### MEDIUM — Assignment 3: Cloud Storage Abstraction

**Scenario**: Your application uploads files to cloud storage. Today it supports AWS S3 and Azure Blob Storage. Tomorrow it may support Google Cloud Storage.

**Products**:
- `StorageClient` — connects and authenticates
- `BucketManager` — creates/lists buckets
- `FileUploader` — uploads a file with progress callback

**Task**:
1. Define abstract products and a `CloudStorageFactory`
2. Implement `AWSFactory` (stub implementations printing `[AWS] ...`)
3. Implement `AzureFactory` (stub implementations printing `[Azure] ...`)
4. Write a `BackupService` class that takes a factory and runs a full backup workflow (connect → ensure bucket → upload)
5. Swap factories in `main()` and verify `BackupService` code is unchanged

---

### ADVANCED — Assignment 4: Game Rendering Engine

**Scenario**: A game engine must support three graphics backends: DirectX (Windows), OpenGL (cross-platform), and Vulkan (high-performance).

**Products**:
- `Renderer` — `beginFrame()`, `endFrame()`, `drawMesh()`
- `Texture` — `load(path)`, `bind(slot)`
- `Shader` — `compile(src)`, `setUniform(name, value)`
- `RenderTarget` — `attach()`, `detach()`, `resolve()`

**Task**:
1. Define all abstract products and `RenderingFactory`
2. Implement stub concrete products for DirectX, OpenGL, and Vulkan (print which backend is used for each call)
3. Implement a `RenderPipeline` class that:
   - Takes a `RenderingFactory`
   - Builds a complete frame: load texture → compile shader → begin frame → draw mesh → end frame
4. The backend selection must be driven by a config file (`render.cfg`) read at startup, **not** hardcoded `#ifdef`
5. Benchmark: measure time for 1000 `drawMesh()` calls via `std::chrono`. Compare virtual dispatch overhead vs direct call. Discuss results.

---

### ADVANCED — Assignment 5: Multi-Tenant Database Layer

**Scenario**: A SaaS platform serves multiple tenants, each potentially using a different database. Tenant config is loaded from JSON at runtime.

**Products**:
- `Connection` — `open(dsn)`, `close()`, `isAlive()`
- `Transaction` — `begin()`, `commit()`, `rollback()`
- `PreparedStatement` — `prepare(sql)`, `bind(index, value)`, `execute()`
- `ResultSet` — `next()`, `getString(col)`, `getInt(col)`

**Task**:
1. Implement all abstract products and `DatabaseFactory`
2. Implement stubs for `MySQLFactory`, `PostgreSQLFactory`, `SQLiteFactory`
3. Implement a `TenantRegistry` that maps tenant IDs to their factory (loaded from config)
4. Implement a `QueryService` that:
   - Accepts a tenant ID
   - Fetches the appropriate factory from `TenantRegistry`
   - Executes a parameterised query and returns results
5. Add **connection pooling**: each factory creates pooled connections (use `std::queue<std::unique_ptr<Connection>>`)
6. Thread-safety: protect the pool with `std::mutex`. Prove correctness using TSan (Thread Sanitizer).

---

### ADVANCED — Assignment 6: IoT Sensor Platform (Embedded-Inspired)

**Scenario**: An IoT gateway collects data from sensors. Different hardware vendors provide different implementations of the same sensor types.

**Products**:
- `TemperatureSensor` — `readCelsius() -> float`
- `HumiditySensor` — `readPercent() -> float`
- `PressureSensor` — `readHPa() -> float`
- `SensorCalibrator` — `calibrate(sensor)`

**Task**:
1. Define abstract products and `SensorFactory`
2. Implement `BoschSensorFactory` and `HoneywellSensorFactory` (simulate readings with random noise using `<random>`)
3. Implement a `DataCollector` that:
   - Uses a factory to create all sensors
   - Collects 100 samples at 10ms intervals using `std::this_thread::sleep_for`
   - Stores results in `std::vector<SensorReading>` (timestamp + all three values)
4. Implement `DataExporter` with two strategies: CSV export and JSON export (use Strategy pattern alongside Abstract Factory)
5. **Performance task**: Profile the sensor read loop. Replace virtual dispatch with `std::variant` + `std::visit` for the concrete sensor types. Measure the speedup using `std::chrono::high_resolution_clock`.

---

## Key Takeaways

```
Abstract Factory = "Give me a complete, consistent family of objects"

The client:
  - Knows the abstract types (Button, Checkbox)
  - Knows the abstract factory (GUIFactory)
  - Does NOT know the concrete types (WindowsButton, MacCheckbox)
  - Does NOT know the concrete factory (WindowsFactory)

The composition root (main / config loader):
  - Is the ONLY place that decides which concrete factory to use
  - Injects it into the client via constructor
```
