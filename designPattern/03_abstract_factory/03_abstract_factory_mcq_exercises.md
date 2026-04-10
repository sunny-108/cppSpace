# Abstract Factory — MCQ & Practical Exercises

---

## Part 1: Multiple Choice Questions (MCQs)

**Instructions**: Choose the single best answer for each question.

---

**Q1.** What is the primary intent of the Abstract Factory pattern?

- A) Create a single object without exposing instantiation logic  
- B) Provide an interface for creating families of related objects without specifying concrete classes  
- C) Separate an object's construction from its representation  
- D) Define a skeleton of an algorithm in a base class  

<details><summary>Answer</summary>
<b>B</b> — Abstract Factory creates entire families of related/dependent objects while hiding concrete types from the client.
</details>

---

**Q2.** Which of the following best describes the relationship between **Abstract Factory** and **Factory Method**?

- A) Abstract Factory uses inheritance; Factory Method uses composition  
- B) Factory Method creates one product; Abstract Factory creates a family of products  
- C) They are the same pattern with different names  
- D) Abstract Factory is only for UI; Factory Method is for databases  

<details><summary>Answer</summary>
<b>B</b> — A Factory Method deals with one product type; Abstract Factory groups multiple factory methods to produce consistent families.
</details>

---

**Q3.** In the Abstract Factory pattern, which component is responsible for the actual object instantiation?

- A) Abstract Product  
- B) Client  
- C) Concrete Factory  
- D) Abstract Factory interface  

<details><summary>Answer</summary>
<b>C</b> — Concrete Factories implement the abstract factory interface and call `new` (or `make_unique`) on the concrete product types.
</details>

---

**Q4.** What is the main **disadvantage** of the Abstract Factory pattern?

- A) It makes client code dependent on concrete product types  
- B) Adding a new product type to the family requires changing all concrete factories  
- C) It cannot be used to swap implementations at runtime  
- D) It violates the Open/Closed Principle  

<details><summary>Answer</summary>
<b>B</b> — This is the classic drawback. Adding e.g. a <code>Slider</code> to a GUI factory means updating every concrete factory class.
</details>

---

**Q5.** Consider the following code:

```cpp
class Application {
    std::unique_ptr<Button>   btn_;
    std::unique_ptr<Checkbox> chk_;
public:
    explicit Application(std::unique_ptr<GUIFactory> factory) {
        btn_ = factory->createButton();
        chk_ = factory->createCheckbox();
    }
};
```

What design principle does this demonstrate?

- A) Liskov Substitution Principle only  
- B) Program to an implementation, not an interface  
- C) Dependency Injection + program to interfaces  
- D) Singleton pattern via factory  

<details><summary>Answer</summary>
<b>C</b> — The <code>Application</code> depends only on the abstract <code>GUIFactory</code> and abstract products. The concrete factory is injected from outside.
</details>

---

**Q6.** You have `WindowsFactory`, `MacFactory`, and `LinuxFactory`. After shipping, you need to add support for a `Slider` widget. What must you do?

- A) Modify only `GUIFactory` abstract class  
- B) Modify `GUIFactory`, add `Slider` abstract product, and add `createSlider()` to all three concrete factories  
- C) Add `createSlider()` only to `WindowsFactory` since Sliders are Windows-specific  
- D) Create a new factory `SliderFactory` that is independent  

<details><summary>Answer</summary>
<b>B</b> — You must update the abstract factory interface AND every concrete factory. This friction is by design — it ensures families stay consistent.
</details>

---

**Q7.** Which SOLID principle does Abstract Factory most directly support by isolating object creation from client code?

- A) Single Responsibility Principle  
- B) Liskov Substitution Principle  
- C) Dependency Inversion Principle  
- D) Interface Segregation Principle  

<details><summary>Answer</summary>
<b>C</b> — High-level modules (Application) depend on abstractions (GUIFactory, Button), not concretions (WindowsButton). That is DIP.
</details>

---

**Q8.** Where should the decision of **which concrete factory to use** be made in a well-designed system?

- A) Distributed throughout the client classes using `#ifdef`  
- B) Centralised at the **composition root** (e.g., `main()` or a config loader)  
- C) Inside the abstract factory's constructor  
- D) Inside each product's constructor  

<details><summary>Answer</summary>
<b>B</b> — The composition root is the single place that knows concrete types. All other code operates on abstractions.
</details>

---

**Q9.** Given:

```cpp
struct DatabaseFactory {
    virtual std::unique_ptr<Connection>  createConnection()  = 0;
    virtual std::unique_ptr<Command>     createCommand()     = 0;
    virtual std::unique_ptr<DataReader>  createDataReader()  = 0;
    virtual ~DatabaseFactory() = default;
};
```

A developer writes `MySQLFactory` that overrides only `createConnection()` and `createCommand()` but forgets `createDataReader()`. What happens at compile time in C++?

- A) Compiles fine; `createDataReader()` returns null by default  
- B) Linker error only  
- C) `MySQLFactory` becomes an abstract class and cannot be instantiated  
- D) Runtime crash when `createDataReader()` is called  

<details><summary>Answer</summary>
<b>C</b> — Because <code>createDataReader()</code> is pure virtual and unoverridden, <code>MySQLFactory</code> is still abstract. Attempting <code>make_unique&lt;MySQLFactory&gt;()</code> causes a compile error.
</details>

---

**Q10.** Which scenario is the **best fit** for Abstract Factory?

- A) You need to create one type of object via a subclass decision  
- B) You need to ensure a set of related objects always come from the same family  
- C) You want to cache and reuse a single instance globally  
- D) You want to build a complex object step by step  

<details><summary>Answer</summary>
<b>B</b> — Abstract Factory shines when consistency across a product family is required (e.g., all widgets must be Mac-style, all DB objects must be MySQL-compatible).
</details>

---

**Q11.** Abstract Factory is often combined with which other creational pattern to ensure only one factory instance exists per family?

- A) Observer  
- B) Prototype  
- C) Singleton  
- D) Builder  

<details><summary>Answer</summary>
<b>C</b> — Concrete factories are frequently Singletons because you typically need exactly one factory per product family per application run.
</details>

---

**Q12.** How does Abstract Factory support the **Open/Closed Principle**?

- A) You can add new product types without any changes  
- B) You can add new product families (new concrete factories + products) without modifying existing client code  
- C) You can modify concrete products without recompiling  
- D) Open/Closed Principle does not apply to creational patterns  

<details><summary>Answer</summary>
<b>B</b> — Adding a new family (e.g., <code>WaylandFactory</code> for a new Linux compositor) requires no changes to <code>Application</code> or any existing factory.
</details>

---

**Q13.** Which factory pattern hierarchy is correct?

- A) Abstract Product → Concrete Factory → Abstract Factory → Concrete Product  
- B) Abstract Factory → Concrete Factory; Abstract Product → Concrete Product  
- C) Client → Abstract Factory → Abstract Product → Concrete Factory  
- D) Concrete Factory extends Concrete Product  

<details><summary>Answer</summary>
<b>B</b> — There are two independent hierarchies: (1) Abstract Factory → Concrete Factories, and (2) Abstract Products → Concrete Products. Concrete factories create concrete products.
</details>

---

**Q14.** What runtime cost does Abstract Factory introduce compared to direct `new` calls?

- A) None — the compiler always devirtualises factory calls  
- B) Virtual dispatch overhead per factory method call  
- C) Dynamic memory overhead proportional to number of factories  
- D) Mandatory heap allocation for all products  

<details><summary>Answer</summary>
<b>B</b> — Each factory method is a virtual call (vtable lookup). In hot paths this can matter; in most application code it is negligible.
</details>

---

**Q15.** A developer needs to unit test `Application` in isolation. Why does Abstract Factory make this easier?

- A) It removes the need for any testing since the pattern is provably correct  
- B) You can inject a `MockGUIFactory` that returns mock products, testing `Application` without real UI widgets  
- C) Abstract Factory auto-generates test doubles  
- D) Unit testing is not possible with virtual dispatch  

<details><summary>Answer</summary>
<b>B</b> — Because <code>Application</code> depends on an abstract factory, you can substitute a <code>MockFactory</code> in tests, making <code>Application</code> independently testable.
</details>

---

## Part 2: Practical Exercises

---

### Exercise 1 — Code Review

**Instructions**: Review the code below and identify **all problems**. For each issue: state the problem, explain *why* it is a problem, and provide the corrected code.

```cpp
// ── Buggy Abstract Factory Implementation ────────────────────────────────────

class Button {
public:
    virtual void paint() = 0;
    // (A) No virtual destructor
};

class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual void paint() = 0;
};

class WinButton : public Button {
public:
    void paint() override { std::cout << "WinButton\n"; }
    ~WinButton() { std::cout << "WinButton destroyed\n"; }
};

class WinCheckbox : public Checkbox {
public:
    void paint() override { std::cout << "WinCheckbox\n"; }
};

class GUIFactory {
public:
    virtual Button*   createButton()   = 0;   // (B) Raw pointer return
    virtual Checkbox* createCheckbox() = 0;   // (B) Raw pointer return
};

class WindowsFactory : public GUIFactory {
public:
    Button* createButton() override {
        return new WinButton();              // (C) Caller must delete
    }
    Checkbox* createCheckbox() override {
        return new WinCheckbox();
    }
};

class Application {
public:
    void run() {
        WindowsFactory factory;              // (D) Depends on concrete type
        Button*   btn = factory.createButton();
        Checkbox* chk = factory.createCheckbox();
        btn->paint();
        chk->paint();
        delete btn;
        // (E) forgot delete chk — memory leak
    }
};
```

**Issues to find:**
- (A) Missing virtual destructor in `Button`
- (B) Raw pointer ownership — who owns the returned object?
- (C) Caller is responsible for `delete` — fragile, breaks with exceptions
- (D) `Application::run()` hardcodes `WindowsFactory` — defeats the entire pattern
- (E) Memory leak: `chk` is never deleted

**Your Task**: Rewrite a corrected version of this code using `std::unique_ptr`, proper virtual destructors, and dependency injection via constructor.

---

### Exercise 2 — Debugging

**Instructions**: The code below compiles but produces **incorrect behaviour**. Find and fix all bugs. There are **4 bugs** hidden in this code.

```cpp
#include <iostream>
#include <memory>

// Abstract products
struct Color { virtual std::string hex() const = 0; virtual ~Color() = default; };
struct Font  { virtual std::string name() const = 0; virtual ~Font() = default; };

// Dark theme products
struct DarkPrimary : Color {
    std::string hex() const override { return "#1A1A2E"; }
};
struct DarkFont : Font {
    std::string name() const override { return "Roboto Dark"; }
};

// Light theme products
struct LightPrimary : Color {
    std::string hex() const override { return "#FFFFFF"; }
};
struct LightFont : Font {
    std::string name() const override { return "Roboto Light"; }
};

// Abstract factory
struct ThemeFactory {
    virtual std::unique_ptr<Color> createColor() = 0;
    virtual std::unique_ptr<Font>  createFont()  = 0;
    virtual ~ThemeFactory() = default;
};

// Bug 1: DarkThemeFactory creates Light products
struct DarkThemeFactory : ThemeFactory {
    std::unique_ptr<Color> createColor() override {
        return std::make_unique<LightPrimary>();   // ← BUG
    }
    std::unique_ptr<Font> createFont() override {
        return std::make_unique<DarkFont>();
    }
};

// Bug 2: LightThemeFactory is missing createFont override
struct LightThemeFactory : ThemeFactory {
    std::unique_ptr<Color> createColor() override {
        return std::make_unique<LightPrimary>();
    }
    // Missing createFont() override — LightThemeFactory is abstract!
};

struct UIPanel {
    std::unique_ptr<Color> bg_;
    std::unique_ptr<Font>  font_;

    // Bug 3: Factory is taken by value — slicing
    UIPanel(ThemeFactory factory) {                // ← BUG
        bg_   = factory.createColor();
        font_ = factory.createFont();
    }

    void render() {
        std::cout << "Color: " << bg_->hex()    << "\n";
        std::cout << "Font:  " << font_->name() << "\n";
    }
};

int main() {
    // Bug 4: Trying to instantiate abstract LightThemeFactory
    auto factory = std::make_unique<LightThemeFactory>();   // ← BUG
    UIPanel panel(*factory);
    panel.render();
}
```

**Hints:**
1. Dark factory creates wrong product
2. `LightThemeFactory` never provides a `Font` — it remains abstract
3. You cannot pass a polymorphic type by value — object slicing occurs
4. Instantiating an abstract class fails to compile

**Your Task**: Fix all 4 bugs and write the expected output after fixing.

---

### Exercise 3 — Implementation from Scratch

**Scenario**: You are building a **notification system** for a mobile app. The app runs on iOS and Android. Each platform sends notifications differently.

**Products**:

| Abstract Product | iOS Concrete | Android Concrete |
|---|---|---|
| `PushNotification` | `APNSNotification` | `FCMNotification` |
| `LocalNotification` | `iOSLocalNotif` | `AndroidLocalNotif` |
| `NotificationChannel` | `iOSChannel` | `AndroidChannel` |

**Abstract Factory**: `NotificationFactory` with:
- `createPush() → unique_ptr<PushNotification>`
- `createLocal() → unique_ptr<LocalNotification>`
- `createChannel() → unique_ptr<NotificationChannel>`

**Requirements**:
1. Define all abstract products with at least 2 pure virtual methods each
2. Implement full iOS and Android product families (stub implementations are fine)
3. Implement `iOSNotificationFactory` and `AndroidNotificationFactory`
4. Implement `NotificationService` class that:
   - Receives a factory via constructor
   - Sends a push notification (`title`, `body` parameters)
   - Schedules a local notification (`delay_seconds` parameter)
   - Creates a default notification channel named `"general"`
5. In `main()`, select the factory based on a runtime string: `"ios"` or `"android"` (read from `std::cin` or a command-line argument)
6. `NotificationService` must contain **zero** platform-specific code

**Stretch Goal**: Add a `MockNotificationFactory` that records all calls and allows asserting that specific methods were called (useful for unit testing).

---

### Exercise 4 — Performance Optimization

**Scenario**: A game engine runs at 60 FPS. The rendering loop calls the factory to create short-lived render commands every frame. After profiling, virtual dispatch overhead is measurable.

**Baseline code** (the slow version):

```cpp
struct DrawCommand {
    virtual void execute() = 0;
    virtual ~DrawCommand() = default;
};

struct VulkanDrawCommand : DrawCommand {
    void execute() override {
        // simulate work
        volatile int x = 0;
        for (int i = 0; i < 100; ++i) x += i;
    }
};

struct RenderFactory {
    virtual std::unique_ptr<DrawCommand> createDrawCommand() = 0;
    virtual ~RenderFactory() = default;
};

struct VulkanFactory : RenderFactory {
    std::unique_ptr<DrawCommand> createDrawCommand() override {
        return std::make_unique<VulkanDrawCommand>();
    }
};

// Hot loop — called 60 times per second, creates 1000 commands per frame
void renderFrame(RenderFactory& factory) {
    for (int i = 0; i < 1000; ++i) {
        auto cmd = factory.createDrawCommand();
        cmd->execute();
    }
}
```

**Tasks**:

**Task 4a — Measure the baseline**:
Write a benchmark using `std::chrono::high_resolution_clock` that:
- Simulates 3600 frames (60 seconds × 60 FPS)
- Measures total time and average time per frame
- Prints results

**Task 4b — Eliminate per-frame heap allocation**:
Refactor `renderFrame` to use an **object pool**:
- Pre-allocate 1000 `VulkanDrawCommand` objects
- Reuse them each frame instead of allocating/deallocating
- Measure speedup vs baseline

**Task 4c — Remove virtual dispatch with `std::variant`**:
Replace the virtual `DrawCommand` hierarchy with:
```cpp
using DrawCommandVariant = std::variant<VulkanDrawCommand, DirectXDrawCommand, OpenGLDrawCommand>;
```
Use `std::visit` with a lambda to call `execute()`. Measure speedup vs Task 4b.

**Task 4d — Analyse**:
Fill in the table:

| Approach | Total Time (3600 frames) | Avg per Frame | Speedup vs Baseline |
|---|---|---|---|
| Baseline (virtual + heap alloc) | | | 1.0× |
| Object Pool (virtual, no alloc) | | | |
| std::variant + std::visit | | | |

Write 3-5 sentences explaining your findings. Which approach would you choose for a real game engine, and why?

---

### Exercise 5 — Extend the Pattern (Design Challenge)

**Scenario**: Your existing `GUIFactory` produces `Button`, `Checkbox`, and `TextField`. Product managers want to add `DropdownMenu` to all platforms.

**Current class count**: 3 abstract products + 3 concrete products per platform (9 concrete products) + 3 concrete factories + 1 abstract factory = **16 classes**

**Task**:
1. Add `DropdownMenu` to the system. Count the new total classes.
2. Propose an alternative design using **prototype-based factory** or **template factory** that reduces class explosion. Implement it.
3. Compare your design to the classic Abstract Factory on these axes: extensibility, type safety, testability, compile-time vs runtime flexibility.
4. Write a short design document (as comments in your `.cpp`) arguing which approach is better for:
   - A 3-platform desktop app that is very stable
   - An embedded system that needs to add sensor vendors frequently

---

## Grading Rubric

| Exercise | Key Assessment Criteria |
|---|---|
| **Code Review** | All 5 issues found? Corrected code compiles and is idiomatic modern C++? |
| **Debugging** | All 4 bugs found and explained? Fixed code runs and produces correct output? |
| **From Scratch** | Does `NotificationService` contain zero platform code? Are all products implemented? Does runtime factory selection work? |
| **Performance** | Is measurement methodology correct? Are results reproducible? Is analysis thoughtful? |
| **Design Challenge** | Does the alternative reduce class count? Is the trade-off analysis accurate and specific? |
