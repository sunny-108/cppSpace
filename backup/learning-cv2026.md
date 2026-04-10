# **SUNNY SHIVAM**

Senior Software Engineer | C/C++ & Java Systems Programming | 15+ Years Experience
Email: shivam.edac@gmail.com | Mobile: 9930003853 | Bengaluru, IN

---

## **PROFESSIONAL SUMMARY**

Senior Software Engineer with 15+ years of experience in systems programming, specializing in multi-threaded C/C++ and Java platform development for enterprise-scale storage systems. Proven expertise in concurrent architecture design, cross-product security frameworks, memory optimization, and platform reliability. Contributor across 26+ repositories spanning C++, Java and Python, with a track record of end-to-end feature ownership — from architecture and implementation through testing, deployment, and customer escalation resolution. Experienced in leading engineering quality initiatives, mentoring teams, and setting technical direction for mission-critical storage infrastructure.

---

## **CORE COMPETENCIES**

| **Concurrency & Threading** | Multi-threaded architecture design, Thread synchronization (mutexes, semaphores, condition variables), Lock-free programming, Deadlock detection/prevention, Race condition analysis, Producer-Consumer patterns |
| **Networking** | TCP internals (congestion control, flow control, Nagle/NODELAY), Socket tuning (BDP, buffer sizing, keepalives), RDMA concepts (QP, zero-copy, one-sided ops), tcpdump packet analysis |
| **Security Engineering** | Dual Authorization (authN/authZ) frameworks, TOTP/2FA multi-factor authentication, TPM configuration, Log4j remediation, Secure-by-default engineering |
| **Memory & Performance** | Memory leak detection, Smart pointers, RAII patterns, Heap profiling, Resource lifecycle optimization, JaCoCo code coverage |
| **Languages** | C++14/17/20, C, Java (JDK 8+), Python |
| **Testing & Quality** | Python test automation frameworks, JaCoCo integration, Unit/Integration/System testing, AddressSanitizer, ThreadSanitizer |
| **Linux Debugging & Profiling** | strace (syscall tracing, latency analysis), Valgrind, GDB |
| **Build & Tools** | CMake, Gradle, Visual Studio, GDB, Git |
| **Platforms** | Linux (Rocky, CentOS, Ubuntu), Windows Server, AWS |
| **Databases & APIs** | MS SQL Server, Oracle 11g, REST API design|
| **AI-Assisted Development** | GitHub Copilot (code completion, chat, agent mode, PR reviews), Claude, Gemini; daily use for code generation, debugging, refactoring, and architectural exploration |

---

## **PROFESSIONAL EXPERIENCE**

### **Hewlett Packard Enterprise** | System Software Engineer

**December 2018 – Present** | StoreOnce Storage Platform
*Contributions across 26+ repositories, 8 GitHub organizations, 146+ pull requests, 37+ code reviews*

---

#### **Security & Authorization (2023 – 2026)**

- **Dual Authorization Framework — Cross-Platform Architecture:** Architected and implemented end-to-end Dual Authorization (DA) security framework across 6+ StoreOnce microservices (so-pml-core, pml-storeonce-cat, pml-storeonce-nas, pml-storeonce-rep, localization packs, test automation); designed REST API validation hooks, event logging infrastructure, and cross-component security workflow for NAS shares, Catalyst items, VTL, and Replication operations — delivering multi-repo coordinated releases with full localization and Python-based test automation coverage
- **Multi-Factor Authentication (TOTP/2FA) — Full-Stack Delivery:** Implemented TOTP-based Two-Factor Authentication for StoreOnce local users end-to-end; built React/Grommet UI frontend, integrated with backend security services; delivered localization support across all 2FA interfaces
- **TPM Configuration for Manufacturing:** Implemented TPM configuration framework in platform BIOS settings for StoreOnce manufacturing process, contributing to secure boot infrastructure
- **Log4j Security Remediation:** Addressed Log4j vulnerability across Catalyst Plugin installer, removing vulnerable dependencies for enterprise customers

---

#### **Platform Architecture & Cross-Product Development (2019 – 2026)**

- **SAP-HANA Sliding-Window Thread Pool Modernization:** Modernized Linux pthread-based sliding-window thread pool to C++17 standards by replacing `pthread_create`/`pthread_join` with `std::thread`, migrating custom `thrLock_t` mutex wrappers to `std::mutex` with `std::lock_guard`, replacing `Sleep(1000)` busy-polling with `std::condition_variable` notification for near-zero dispatch latency, and adopting `std::unique_ptr` for thread lifecycle management; preserved 4-state thread execution model (NOT_STARTED → RUNNING → FINISHED → COMPLETED) for parallel backup/restore stream orchestration with server-capacity-based concurrency control
- **Design Patterns & Architecture:** Implemented Command pattern for job execution, Object Pool pattern for thread lifecycle management, and Singleton pattern for global resource coordination; redesigned business logic layer with thread-safe resource management and free-thread detection algorithm for efficient job dispatching
- **Delta Log Collection System:** Designed and implemented incremental (delta) log collection framework across 3 repositories (so-pml-logcollection, pml-storeonce-d2d-service, so-D2D-Stack); enabled time-based differential support ticket generation reducing data collection overhead
- **Parametric Data Collection — Cross-Component:** Implemented disk latency and utilization metrics across D2D-Stack and FME RPC server; extended parametric file collection to Comprehensive Support Bundles across 6 PML plugins (CAT, NAS, REP, VTL, SMM, ResMon), improving supportability and diagnostic capabilities
- **HANA Backup Expiration — CPU Saturation:** Resolved enterprise escalation where SAP HANA backup expiration scripts drove CPU to 100% on StoreOnce 5260; optimized script resource consumption for production stability
- **AIX Diagnostic Monitoring Suite:** Designed and built comprehensive Korn shell monitoring toolkit for AIX Power10/7.2 platforms to diagnose intermittent Oracle RMAN backup failures against StoreOnce; suite includes: master orchestrator spawning parallel monitors, 75+ hours continuous system monitoring (CPU/memory/vmstat at 30-sec intervals), Oracle RMAN session tracker with auto-detection of `ORACLE_HOME` via 4 discovery methods, StoreOnce network latency/throughput monitor, storage I/O subsystem profiler, real-time alert engine with configurable thresholds (CPU >85%, I/O wait >30%) and syslog integration, StoreOnce appliance auto-discovery, Power10 LPAR compatibility validation,; enabled field engineers to capture production evidence for root-cause analysis of intermittent failures

---

#### **Quality Engineering & Technical Leadership (2025 – 2026)**

- **Code Coverage Initiative — Led Across 9+ Repositories:** Drove JaCoCo code coverage integration across 9+ PML repositories (appliance, cat, cache, d2d-service, dashboard, deviceinterface, fc, pmle-mgmt); established baseline metrics, created onboarding documentation and standardized README templates; raised engineering quality bar across the StoreOnce platform
- **Python Test Automation Frameworks:** Built comprehensive test automation for Dual Authorization features using Python; designed reusable test frameworks for NAS, Catalyst, and Replication DA workflows in the StoreOnce-Test repository
- **Code Reviews & Mentorship:** Performed 37+ code reviews across multiple repositories and organizations; maintained active contributions across cross-functional teams

---

#### **Memory Optimization & Reliability (2019 – 2025)**

- **Critical Memory Leak Resolution:** Identified and resolved critical memory corruption in RMAN plugin shared buffer access using state machine redesign and ThreadSanitizer verification; eliminated crashes during long-running operations in SAP-HANA plugins
- **Concurrent Backup Expiration Fix:** Resolved concurrent backup expiration failures by implementing process-level mutex, eliminating race conditions in credential file access
- **Enterprise Escalation — Oracle RMAN Heap Corruption Fix:** Resolved critical customer escalation involving intermittent Oracle database crashes caused by heap corruption (`STATUS_HEAP_CORRUPTION`) in StoreOnce Catalyst RMAN plugin; performed root-cause analysis of Windows crash dumps, identified invalid `basic_string` destructor freeing address `0xc` due to object lifetime issue, and delivered fix eliminating a years-old intermittent defect affecting production backup operations
- **RAII & Smart Pointer Adoption:** Implemented comprehensive smart pointer migration across legacy codebase, reducing memory-related defects
- **RMAN Channel Hung — Long-Running Backup Failures:** Analyzed intermittent RMAN backup failures where channels hung during long-running operations on StoreOnce 5660; provided RCA covering contention analysis and recovery recommendations
- **Customer Escalation Resolution:** Resolved 10+ critical customer escalations directly impacting enterprise revenue — Sony, BASF, Lab Corp of America, Erste Bank, Ericsson, OTP Bank, Swissgrid, GIA Informatik, Gedeon Richter, Taiwan Intelligent Life — spanning heap corruption, memory exhaustion, CPU saturation, concurrent backup failures, and plugin installation issues across SQL, RMAN, and SAP-HANA plugins

---

#### **Key Projects**

| Project                             | Stack                 | Highlights                                                                                                     |
| ----------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------- |
| **SQL Catalyst Plugin**       | C++ / Windows Server  | memory reduction through optimized thread lifecycle and COM interface management                               |
| **SAP-HANA Catalyst Plugin**  | C++ / Linux           | Multi-threaded backup orchestration with parallel stream processing; IPC command pattern for backint interface |
| **RMAN Catalyst Plugin**      | C++ / Linux & Windows | Refactored single-threaded SBT 1.0 → multi-channel SBT 2.0 with context manager for parallel tablespaces      |
| **Install-Update Component**  | C++ & Java / Linux    | Concurrent upgrade framework; Rocky Linux migration; AWS upgrade support                                       |
| **Modular Update System**     | Java / Linux          | *Led team of 4 engineers;* designed component-level upgrade architecture                                     |
| **StoreOnce UI (MFA)**        | JavaScript/React      | Full-stack 2FA/TOTP feature with Grommet UI framework                                                          |
| **StoreOnce Test Automation** | Python                | DA test frameworks, integration/system test suites                                                             |

---

### **Capgemini** (Client: Hewlett Packard Enterprise) | Consultant

**July 2014 – November 2018** | StoreOnce Plugin Development (C++ / Windows Server)

**Key Achievements:**

- **Custom Thread Pool Architecture:** Designed and implemented production-grade thread pool from scratch using Producer-Consumer pattern with configurable worker threads (1-4), event-based synchronization via Windows API (CreateEvent, WaitForMultipleObjects), mutex-protected job queues (std::list), and RAII-based lock management for exception-safe concurrency
- **Hybrid C++/CLI Architecture:** Integrated native C++ core with .NET CLR for ADO.NET database access and managed code interop; implemented marshalling layer for seamless string/data conversion between native and managed contexts
- **Catalyst Plugin Installer Framework (Java / InstallAnywhere 2017):** *Led team of 4 engineers.* Architected enterprise-grade installer supporting 6 plugins (SQL, RMAN, SAP-HANA, NBU-OST, BE-OST, D2D-Copy) across 5 platforms (Linux, AIX, HP-UX, Solaris, Windows); implemented Singleton, Factory, Strategy, and Command patterns; designed modular pre-flight validation, safe upgrade with rollback support, and multi-interface support (GUI/Console/Silent)

**Technologies:** C++14, C++/CLI, Visual Studio 2008/2010, MS SQL Server 2012/2014/2016, ADO.NET, Windows API (Threading, Events, Mutexes), Windows Task Scheduler COM API, RapidJSON, Valgrind

---

### **Halston Software** (Client: Kratos Network) | Sr. Software Engineer

**July 2012 – June 2014** | Device Communication Driver Development (C / Linux)

---

### **COVACSIS Tech. Pvt. Ltd.** | Software Programmer

**August 2010 – July 2012** | Industrial Automation Protocol Development (C/C++ / Linux)

---

## **TECHNICAL EXPERTISE HIGHLIGHTS**

✓ 15+ years in systems programming; 10+ years designing multi-threaded systems with complex synchronization
✓ Architected cross-product security frameworks (Dual Auth, MFA/2FA, TPM) across 6+ microservices
✓ Expert in memory leak detection, profiling, and optimization
✓ Proficient in C++14/17, Java, Python — with production code in each
✓ Led code quality initiatives: JaCoCo coverage across 9+ repos, test automation frameworks
✓ Strong design pattern expertise (Command, Factory, Strategy, Singleton, Object Pool, Producer-Consumer)
✓ Resolved 10+ critical customer escalations across global enterprises (Sony, BASF, Lab Corp, Erste Bank, Ericsson, and others)
✓ Production-level experience with enterprise storage systems (backup/restore, NAS, Catalyst, Replication)
✓ Skilled in end-to-end feature delivery: architecture → implementation → testing → deployment → support

---

## **EDUCATION & CERTIFICATIONS**

- **MTech in Data Science** – BITS Pilani (WILP - 2021)
- **Master of Computer Application** – IGNOU (2010)
- **Specialization Certificate in Programming with Google Go** – Coursera (Aug 2022)
- **Neural Network and Deep Learning** – deeplearning.ai / Coursera (Aug 2020)
- **Software Security** – University of Maryland, College Park / Coursera (2016)

---
