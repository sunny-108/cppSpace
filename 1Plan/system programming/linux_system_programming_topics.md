# Linux System Programming Topics for Senior C++ Developers (12+ Years Experience)

## 1. Process Management

### 1.1 Process Fundamentals
- Process creation: `fork()`, `vfork()`, `clone()`
- Process termination: `exit()`, `_exit()`, `atexit()`
- Process attributes: PID, PPID, UID, GID, EUID, EGID
- Process states and lifecycle
- Zombie and orphan processes
- Process groups and sessions

### 1.2 Process Execution
- `exec()` family: `execl()`, `execle()`, `execlp()`, `execv()`, `execve()`, `execvp()`
- Command-line arguments and environment variables
- Process memory layout (text, data, BSS, heap, stack)

### 1.3 Process Control
- `wait()`, `waitpid()`, `wait3()`, `wait4()`
- Process priority and nice values
- Resource limits: `getrlimit()`, `setrlimit()`
- CPU affinity: `sched_setaffinity()`, `sched_getaffinity()`

## 2. Inter-Process Communication (IPC)

### 2.1 Pipes and FIFOs
- Anonymous pipes: `pipe()`, `pipe2()`
- Named pipes (FIFOs): `mkfifo()`, `mknod()`
- Pipe capacity and atomicity
- Bidirectional communication patterns

### 2.2 System V IPC
- Message queues: `msgget()`, `msgsnd()`, `msgrcv()`, `msgctl()`
- Semaphores: `semget()`, `semop()`, `semctl()`
- Shared memory: `shmget()`, `shmat()`, `shmdt()`, `shmctl()`
- IPC permissions and keys

### 2.3 POSIX IPC
- POSIX message queues: `mq_open()`, `mq_send()`, `mq_receive()`
- POSIX semaphores: named and unnamed semaphores
- POSIX shared memory: `shm_open()`, `shm_unlink()`
- Comparison with System V IPC

### 2.4 Advanced IPC
- Unix domain sockets (AF_UNIX)
- Memory-mapped files: `mmap()`, `munmap()`, `msync()`
- File locking: `flock()`, `fcntl()` (F_SETLK, F_GETLK)
- Record locking and advisory vs. mandatory locking

## 3. Signals

### 3.1 Signal Fundamentals
- Signal types and numbers (SIGTERM, SIGKILL, SIGINT, SIGSEGV, etc.)
- Signal delivery and handling
- Signal disposition: default, ignore, custom handler
- Reliable vs. unreliable signals

### 3.2 Signal Handling
- `signal()` vs. `sigaction()`
- Signal masks: `sigprocmask()`, `pthread_sigmask()`
- Signal sets: `sigemptyset()`, `sigfillset()`, `sigaddset()`, `sigdelset()`
- Blocking and unblocking signals

### 3.3 Advanced Signal Concepts
- Real-time signals (SIGRTMIN to SIGRTMAX)
- `sigwait()`, `sigwaitinfo()`, `sigtimedwait()`
- Signal safety and async-signal-safe functions
- `signalfd()` for signal handling via file descriptors
- Signal handling in multi-threaded programs

## 4. Threads and Synchronization

### 4.1 POSIX Threads (pthreads)
- Thread creation: `pthread_create()`
- Thread termination: `pthread_exit()`, `pthread_cancel()`
- Thread joining and detaching: `pthread_join()`, `pthread_detach()`
- Thread attributes: `pthread_attr_t`

### 4.2 Thread Synchronization Primitives
- Mutexes: `pthread_mutex_t`, recursive mutexes, error-checking mutexes
- Condition variables: `pthread_cond_t`, spurious wakeups
- Read-write locks: `pthread_rwlock_t`
- Barriers: `pthread_barrier_t`
- Spin locks: `pthread_spinlock_t`

### 4.3 Thread-Specific Data
- Thread-local storage (TLS): `__thread`, `thread_local`
- `pthread_key_create()`, `pthread_setspecific()`, `pthread_getspecific()`

### 4.4 Advanced Threading
- Thread pools and worker patterns
- Thread cancellation: types and cleanup handlers
- CPU affinity for threads
- Thread priority and scheduling policies (SCHED_FIFO, SCHED_RR, SCHED_OTHER)

## 5. File I/O and File Systems

### 5.1 File Descriptors and I/O
- `open()`, `close()`, `read()`, `write()`, `lseek()`
- File descriptor flags: O_NONBLOCK, O_CLOEXEC, O_DIRECT, O_SYNC
- `dup()`, `dup2()`, `fcntl()`
- File status flags and file descriptor flags

### 5.2 Advanced File I/O
- Scatter-gather I/O: `readv()`, `writev()`
- Positioned I/O: `pread()`, `pwrite()`
- Vectored and positioned I/O: `preadv()`, `pwritev()`
- `sendfile()` for zero-copy transfers
- Asynchronous I/O (AIO): `aio_read()`, `aio_write()`

### 5.3 File System Operations
- Directory operations: `opendir()`, `readdir()`, `closedir()`
- File metadata: `stat()`, `fstat()`, `lstat()`
- File permissions and ownership: `chmod()`, `chown()`
- Hard links and symbolic links: `link()`, `symlink()`, `readlink()`
- `inotify` for file system event monitoring

### 5.4 Memory-Mapped I/O
- `mmap()` for file mapping
- Memory protection: `mprotect()`
- Anonymous mappings (MAP_ANONYMOUS)
- Shared vs. private mappings
- `madvise()` for memory usage hints

## 6. Network Programming

### 6.1 Socket Programming Basics
- Socket types: SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
- Socket creation: `socket()`
- Binding: `bind()`
- TCP: `listen()`, `accept()`, `connect()`
- UDP: `sendto()`, `recvfrom()`

### 6.2 Advanced Socket Programming
- Socket options: `setsockopt()`, `getsockopt()`
- Non-blocking sockets and O_NONBLOCK
- Socket multiplexing: `select()`, `poll()`, `epoll()`
- `epoll()` edge-triggered vs. level-triggered modes
- Zero-copy techniques: `sendfile()`, splice(), tee()

### 6.3 Network Protocols
- IPv4 and IPv6 programming
- Raw sockets and packet capture
- Multicast and broadcast
- Unix domain sockets for IPC

### 6.4 High-Performance Networking
- TCP_NODELAY and Nagle's algorithm
- SO_REUSEADDR and SO_REUSEPORT
- Connection pooling
- Load balancing techniques

## 7. I/O Multiplexing and Event-Driven Programming

### 7.1 select()
- Syntax and usage
- File descriptor sets
- Limitations (FD_SETSIZE)

### 7.2 poll()
- Advantages over `select()`
- `pollfd` structure
- Event types: POLLIN, POLLOUT, POLLERR

### 7.3 epoll() (Linux-specific)
- `epoll_create()`, `epoll_ctl()`, `epoll_wait()`
- Edge-triggered vs. level-triggered
- Performance characteristics
- EPOLLONESHOT and EPOLLET flags

### 7.4 Alternative Event Mechanisms
- `kqueue` (BSD/macOS)
- `io_uring` (modern Linux)
- Event libraries: libevent, libev, libuv

## 8. Memory Management

### 8.1 Dynamic Memory Allocation
- `malloc()`, `calloc()`, `realloc()`, `free()`
- `posix_memalign()` for aligned allocations
- Memory allocation strategies
- Memory leaks and tools (Valgrind, ASan)

### 8.2 Virtual Memory
- Address spaces and memory layout
- Page faults and demand paging
- Copy-on-write (COW)
- Huge pages (THP, explicit huge pages)

### 8.3 Memory-Mapped Files
- `mmap()` and `munmap()`
- Memory protection and permissions
- Shared memory through mmap
- `mlock()`, `mlockall()` for preventing swapping

### 8.4 Advanced Memory Concepts
- NUMA awareness
- Memory barriers and cache coherence
- `malloc_trim()`, `malloc_stats()`
- Custom allocators and memory pools

## 9. Debugging and Profiling

### 9.1 Debugging Tools
- GDB: breakpoints, watchpoints, core dumps
- `strace` for system call tracing
- `ltrace` for library call tracing
- Core dump analysis: `ulimit`, `core_pattern`

### 9.2 Memory Debugging
- Valgrind (Memcheck, Helgrind, DRD)
- AddressSanitizer (ASan)
- LeakSanitizer (LSan)
- MemorySanitizer (MSan)
- ThreadSanitizer (TSan)

### 9.3 Performance Profiling
- `perf` tool for CPU profiling
- `gprof` for profiling
- Flame graphs
- `oprofile` and system-wide profiling

### 9.4 Tracing and Observability
- `ftrace` and kernel tracing
- Dynamic tracing: DTrace, SystemTap
- eBPF (extended Berkeley Packet Filter)
- Performance counters and PMU events

## 10. System Calls and Kernel Interface

### 10.1 System Call Fundamentals
- System call mechanism (int 0x80, syscall instruction)
- User space vs. kernel space
- Context switching overhead
- System call wrapper functions

### 10.2 Important System Calls
- Process management: `fork()`, `exec()`, `wait()`
- File operations: `open()`, `read()`, `write()`, `close()`
- Memory management: `brk()`, `sbrk()`, `mmap()`
- Network: `socket()`, `bind()`, `connect()`

### 10.3 Error Handling
- `errno` and error codes
- `perror()`, `strerror()`
- Thread-safe error handling
- Custom error handling strategies

## 11. Timers and Time Management

### 11.1 Time Representation
- `time_t`, `struct timespec`, `struct timeval`
- CLOCK_REALTIME vs. CLOCK_MONOTONIC
- Clock resolution and precision

### 11.2 Timer APIs
- `alarm()` and `setitimer()`
- POSIX timers: `timer_create()`, `timer_settime()`
- `timerfd_create()` for timer file descriptors
- High-resolution timers

### 11.3 Sleep and Delays
- `sleep()`, `usleep()`, `nanosleep()`
- `clock_nanosleep()` with absolute/relative time
- Busy-waiting vs. blocking

## 12. Security and Permissions

### 12.1 User and Group Management
- Real, effective, and saved user/group IDs
- `setuid()`, `setgid()`, `seteuid()`, `setegid()`
- Set-UID and set-GID programs
- File capabilities

### 12.2 Access Control
- File permissions (read, write, execute)
- Access Control Lists (ACLs)
- `access()`, `faccessat()`
- umask and permission inheritance

### 12.3 Security Features
- Privilege separation
- Sandboxing techniques
- seccomp (secure computing mode)
- Linux Security Modules (LSM): SELinux, AppArmor
- Namespace isolation

### 12.4 Secure Programming
- Buffer overflow prevention
- Input validation
- Secure random number generation: `/dev/urandom`, `getrandom()`
- Avoiding race conditions (TOCTOU)

## 13. Advanced Linux Features

### 13.1 Namespaces
- PID namespace
- Network namespace
- Mount namespace
- UTS namespace
- IPC namespace
- User namespace
- Cgroup namespace

### 13.2 Control Groups (cgroups)
- Resource limiting (CPU, memory, I/O)
- cgroups v1 vs. v2
- Integration with systemd

### 13.3 Container Technologies
- Understanding Docker internals
- Container isolation mechanisms
- Image layers and union file systems

### 13.4 Modern Kernel Features
- `io_uring` for high-performance I/O
- BPF and eBPF programming
- Kernel bypass techniques (DPDK, XDP)
- futex (fast userspace mutex)

## 14. Real-Time Programming

### 14.1 Real-Time Concepts
- Hard vs. soft real-time
- Latency and jitter
- Determinism and predictability

### 14.2 Real-Time Scheduling
- SCHED_FIFO and SCHED_RR
- Priority inheritance and priority ceiling
- CPU isolation and IRQ affinity

### 14.3 Real-Time Extensions
- PREEMPT_RT patch
- Real-time process priorities
- Memory locking for real-time processes
- Avoiding priority inversion

## 15. System Design and Architecture

### 15.1 Design Patterns
- Reactor and Proactor patterns
- Producer-consumer pattern
- Pipeline architecture
- Event-driven architecture

### 15.2 High-Performance Systems
- Lock-free data structures
- Wait-free algorithms
- Cache-aware programming
- False sharing and cache line optimization

### 15.3 Scalability
- Horizontal vs. vertical scaling
- Load balancing strategies
- Connection pooling
- Resource management

### 15.4 Reliability and Fault Tolerance
- Error handling and recovery
- Graceful degradation
- Health checks and monitoring
- Logging and observability

## 16. Build Systems and Development Tools

### 16.1 Build Tools
- Make and Makefiles
- CMake for cross-platform builds
- Autotools (autoconf, automake)
- Ninja build system

### 16.2 Version Control
- Git workflows for large projects
- Code review practices
- Continuous Integration/Continuous Deployment (CI/CD)

### 16.3 Static Analysis
- Clang Static Analyzer
- Cppcheck
- Coverity
- Compiler warnings and sanitizers

### 16.4 Documentation
- Doxygen for code documentation
- Man pages
- Technical design documents

## 17. Performance Optimization

### 17.1 CPU Optimization
- Branch prediction and speculation
- Instruction pipelining
- SIMD instructions (SSE, AVX)
- Cache optimization (L1, L2, L3)

### 17.2 Memory Optimization
- Memory alignment
- Data structure layout
- Memory prefetching
- Reducing memory allocations

### 17.3 I/O Optimization
- Buffering strategies
- Zero-copy techniques
- Async I/O and io_uring
- Direct I/O (O_DIRECT)

### 17.4 Profiling-Guided Optimization
- Identifying hotspots
- Benchmarking methodologies
- A/B testing for performance changes

## 18. System Administration Knowledge

### 18.1 Process Monitoring
- `ps`, `top`, `htop`
- `/proc` filesystem
- Process resource usage

### 18.2 System Resources
- CPU usage and load average
- Memory usage (free, available, cached)
- Disk I/O monitoring
- Network monitoring

### 18.3 Logging
- syslog and rsyslog
- journalctl and systemd logging
- Log rotation
- Centralized logging solutions

### 18.4 Service Management
- systemd service units
- Service dependencies
- Socket activation
- Resource limits in service files

## 19. Testing Strategies

### 19.1 Unit Testing
- Google Test framework
- Mock objects and dependency injection
- Code coverage analysis

### 19.2 Integration Testing
- Testing IPC mechanisms
- Network testing
- Database integration testing

### 19.3 System Testing
- Load testing and stress testing
- Performance regression testing
- Chaos engineering principles

### 19.4 Debugging Race Conditions
- Helgrind and ThreadSanitizer
- Reproducing race conditions
- Defensive programming techniques

## 20. Best Practices and Design Principles

### 20.1 RAII and Resource Management
- Smart pointers in modern C++
- Automatic resource cleanup
- Exception safety

### 20.2 Error Handling
- Error codes vs. exceptions
- Error propagation strategies
- Logging and diagnostics

### 20.3 Code Quality
- SOLID principles
- Code reviews
- Refactoring techniques
- Technical debt management

### 20.4 Documentation and Maintenance
- Self-documenting code
- API documentation
- Architecture decision records (ADRs)
- Knowledge transfer

---

## Recommended Books

1. **"The Linux Programming Interface" by Michael Kerrisk** - Comprehensive system programming reference
2. **"Advanced Programming in the UNIX Environment" (APUE) by W. Richard Stevens** - Classic UNIX/Linux programming
3. **"Linux System Programming" by Robert Love** - Modern Linux-specific techniques
4. **"UNIX Network Programming" by W. Richard Stevens** - Network programming bible
5. **"The Art of Multiprocessor Programming" by Maurice Herlihy** - Concurrent programming theory and practice

## Essential Man Pages to Master

- `man 2 fork`, `man 2 exec`, `man 2 wait`
- `man 2 open`, `man 2 read`, `man 2 write`, `man 2 mmap`
- `man 2 socket`, `man 2 epoll`
- `man 3 pthread_create`, `man 3 pthread_mutex_lock`
- `man 7 signal`, `man 7 pthreads`, `man 7 epoll`

## Practical Projects for Skill Development

1. Multi-threaded web server with epoll
2. Custom memory allocator
3. Producer-consumer queue with lock-free implementation
4. Process pool for task distribution
5. Network proxy with connection pooling
6. Real-time data processing pipeline
7. Container runtime (mini Docker)
8. Custom shell with job control
9. High-performance logging system
10. Distributed cache system

---

**Note**: This comprehensive list covers topics essential for senior C++ developers working on Linux systems. Mastery of these topics enables building high-performance, scalable, and reliable system software.
