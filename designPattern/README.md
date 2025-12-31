# C++ Design Patterns Exercise Collection

This collection contains practical exercises for learning the most important design patterns in C++. Each exercise file includes:
- Pattern explanation and use cases
- Skeleton code to implement
- TODO markers for implementation
- Real-world examples

## Pattern Categories

### Creational Patterns
1. **Singleton** - Ensure a class has only one instance
2. **Factory Method** - Create objects without specifying exact classes
3. **Abstract Factory** - Create families of related objects
4. **Builder** - Construct complex objects step by step
5. **Prototype** - Clone objects instead of creating new ones

### Structural Patterns
6. **Adapter** - Make incompatible interfaces work together
7. **Decorator** - Add responsibilities to objects dynamically
8. **Facade** - Provide simplified interface to complex subsystem
9. **Proxy** - Control access to objects
10. **Composite** - Treat individual objects and compositions uniformly

### Behavioral Patterns
11. **Observer** - Define one-to-many dependency between objects
12. **Strategy** - Define family of algorithms, make them interchangeable
13. **Command** - Encapsulate requests as objects
14. **Template Method** - Define algorithm skeleton, let subclasses override steps
15. **State** - Alter object behavior when internal state changes

## How to Use

1. Start with any pattern that interests you
2. Read the explanation and use cases
3. Implement the TODO sections
4. Compare with solutions in `SOLUTIONS.md`
5. Compile and test: `g++ -std=c++17 <filename>.cpp -o <output>`

## Compilation

All exercises use C++17 features. Compile with:
```bash
g++ -std=c++17 -Wall -Wextra <exercise_file>.cpp -o program
./program
```

## Recommended Learning Order

**Beginners:**
1. Singleton → Factory Method → Observer → Strategy

**Intermediate:**
2. Builder → Decorator → Command → Template Method

**Advanced:**
3. Abstract Factory → Adapter → Proxy → State → Composite
