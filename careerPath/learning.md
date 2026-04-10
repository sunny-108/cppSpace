# **SUNNY SHIVAM**

Senior Software Engineer | C++ Systems Programming | 14+ Years Experience | email: shivam.edac@gmail.com, Mobile: 9930003853, Bengaluru, IN

---

## **PROFESSIONAL SUMMARY**

Senior Software Engineer with 14+ years of experience, specializing in multi-threaded C/C++ systems programming for the past decade. Expert in concurrent architecture design, critical memory leak resolution, and resource optimization for enterprise-scale backup/restore systems. Proven track record delivering production-grade solutions with focus on thread synchronization, deadlock prevention, and performance optimization.

---

## **CORE COMPETENCIES**

| **Concurrency & Threading** | Multi-threaded architecture design, Thread synchronization (mutexes, semaphores, condition variables), Lock-free programming, Deadlock detection/prevention, Race condition analysis |
| **Memory Management** | Memory leak detection, Smart pointers, RAII patterns, Heap profiling, Resource lifecycle optimization |
| **Languages** | C++14/17, C, Java (JDK 8+) |
| **Debugging & Tools** | Valgrind, GDB, Visual Studio Debugger, AddressSanitizer, ThreadSanitizer, CMake |
| **Platforms** | Linux (Rocky, CentOS, Ubuntu), Windows Server |
| **Databases** | MS SQL Server, Oracle 11g |
| **AI tools and IDE** | Github copilot + VS Code|

---

## **PROFESSIONAL EXPERIENCE**

### **Hewlett Packard Enterprise** | System Software Engineer

**December 2018 – Present** | StoreOnce Backup/Restore Platform

**Key Achievements:**

- **Modern C++ Thread Pool Refactoring:** Modernized legacy Windows API thread pool to C++14/17 standards by replacing Windows Events with `std::condition_variable`, migrating mutexes to `std::mutex`, adopting `std::thread` for portability, and implementing smart pointers for automatic resource management; improved code maintainability while preserving performance characteristics
- **Design Patterns & Architecture:** Implemented Command pattern for job execution, Object Pool pattern for thread lifecycle management, and Singleton pattern for global resource coordination; redesigned entire business logic layer with thread-safe resource management and free-thread detection algorithm for efficient job dispatching
- **Memory Leak Resolution:** Identified and fixed memory leaks in RMAN and SAP-HANA plugins, eliminating crashes during long-running operations; resolved concurrent backup expiration failures by implementing process-level mutex, eliminating race conditions in credential file access
- **RAII & Smart Pointers:** Implemented comprehensive smart pointer adoption across legacy codebase, reducing memory-related defects

**Projects & Technologies:**

- **SQL Catalyst Plugin (C++ / Windows Server):** Achieved 25% memory reduction through optimized thread lifecycle and COM interface management
- **SAP-HANA Catalyst Plugin (C++ / Linux):** Multi-threaded backup orchestration with parallel stream processing; implemented IPC command pattern for backint interface
- **RMAN Catalyst Plugin (C++ / Linux & Windows):** Refactored single-threaded SBT 1.0 to multi-channel SBT 2.0 architecture with context manager for parallel tablespaces; resolved critical memory corruption (7.8 GB leak) in shared buffer access using state machine and ThreadSanitizer verification
- **Install-Update Component (C++ & Java / Linux):** Concurrent upgrade framework for StoreOnce
- **Modular Update System:** *Lead the team of 4 Engineers,* designed & developed component-level upgrade architecture for StoreOnce

---

### **Capgemini** (Client: Hewlett Packard Enterprise) | Consultant

**July 2014 – November 2018** | StoreOnce Plugin Development (C++ / Windows Server)

**Key Achievements:**

- **Custom Thread Pool Architecture:** Designed and implemented production-grade thread pool from scratch using Producer-Consumer pattern with configurable worker threads (1-4), event-based synchronization via Windows API (CreateEvent, WaitForMultipleObjects), mutex-protected job queues (std::list), and RAII-based lock management for exception-safe concurrency;
- **Hybrid C++/CLI Architecture:** Integrated native C++ core with .NET CLR for ADO.NET database access and managed code interop; implemented marshalling layer for seamless string/data conversion between native and managed contexts
- **Catalyst Plugin Installer Framework (Java / InstallAnywhere 2017):** Lead the team of 4 Engineer. Architected and developed enterprise-grade installer supporting 6 plugins (SQL, RMAN, SAP-HANA, NBU-OST, BE-OST, D2D-Copy) across 5 platforms (Linux, AIX, HP-UX, Solaris, Windows); implemented design patterns including Singleton for pre-flight check runners, Factory for platform-specific checkers, Strategy for plugin-specific validation, and Command for install/uninstall/upgrade operations; designed modular pre-flight validation framework with platform-agnostic interface, safe upgrade mechanism with file migration to temporary directory and rollback support, and multi-interface support (GUI/Console/Silent mode)

**Technologies:** C++14, C++/CLI, Visual Studio 2008/2010, MS SQL Server 2012/2014/2016, ADO.NET, Windows API (Threading, Events, Mutexes), Windows Task Scheduler COM API, RapidJSON, Valgrind

---

### **Halston Software** (Client: Kratos Network) | Sr. Software Engineer

**July 2012 – June 2014** | Device Communication Driver Development (C / Linux)

---

### **COVACSIS Tech. Pvt. Ltd.** | Software Programmer

**August 2010 – July 2012** | Industrial Automation Protocol Development (C/C++ / Linux)

---

## **EDUCATION & CERTIFICATIONS**

- **MTech in Data Science** – BITS Pilani (WILP - 2021)
- **Master of Computer Application** – IGNOU (2010)
- **Specialization Certificate in Programming with Google Go** – Coursera (Aug 2022)
- **Neural Network and Deep Learning** – deeplearning.ai / Coursera (Aug 2020)
- **Software Security** – University of Maryland, College Park / Coursera (2016)

---

## **TECHNICAL EXPERTISE HIGHLIGHTS**

✓ 10+ years designing multi-threaded systems with complex synchronization requirements
✓ Expert in memory leak detection, profiling, and optimization using Valgrind and AddressSanitizer
✓ Proficient in C++14/17 modern features (smart pointers, move semantics, lambda and concurrency)
✓ Strong OOP expertise with proven design pattern implementation in enterprise frameworks
✓ Skilled in deadlock prevention, lock ordering, and fine-grained concurrency control
✓ Production-level experience with enterprise backup/restore systems at scale
