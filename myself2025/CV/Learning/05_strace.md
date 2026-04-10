# strace — Linux System Call Tracing

**Study Time: ~1.5 hours**  
**Prerequisites:** Basic Linux, familiarity with system calls (open, read, write, mmap)

---

## 1. What Is strace?

strace intercepts and logs **every system call** a process makes to the kernel. It's your X-ray into what a program is actually doing at the OS level.

```
Your C++ program:
  fopen("data.txt", "r")   ←── high-level C library call

What the kernel sees (strace shows this):
  openat(AT_FDCWD, "data.txt", O_RDONLY) = 3
  │       │              │         │        └── returned fd=3
  │       │              │         └── flag: read-only
  │       │              └── the file path
  │       └── relative to current working directory
  └── the actual system call
```

---

## 2. Essential Commands

### Trace a New Process
```bash
# Basic trace
strace ./my_program

# With timestamps (useful for latency debugging)
strace -tt ./my_program          # wall clock time per syscall
strace -T ./my_program           # time spent IN each syscall (most useful)
strace -ttt ./my_program         # Unix epoch timestamps

# Save to file (stderr is default output)
strace -o trace.log ./my_program

# Follow child processes/threads
strace -f ./my_multi_threaded_app

# Show string arguments fully (default truncates at 32 chars)
strace -s 256 ./my_program
```

### Attach to Running Process
```bash
# By PID
strace -p 12345

# By PID, follow all threads
strace -fp 12345

# Detach with Ctrl+C (doesn't kill the process)
```

### Filter by Syscall Category
```bash
# Only file operations
strace -e trace=file ./my_program
# Traces: open, openat, stat, fstat, lstat, access, chmod, chown, unlink, rename, ...

# Only network operations
strace -e trace=network ./my_program
# Traces: socket, connect, bind, listen, accept, send, recv, ...

# Only process management
strace -e trace=process ./my_program
# Traces: fork, clone, execve, wait4, exit_group, ...

# Only memory operations
strace -e trace=memory ./my_program
# Traces: mmap, munmap, mprotect, brk, ...

# Only signal operations
strace -e trace=signal ./my_program
# Traces: rt_sigaction, rt_sigprocmask, kill, ...

# Specific syscalls only
strace -e trace=open,read,write,close ./my_program

# Everything EXCEPT a set of calls
strace -e trace=!write ./my_program
```

### Statistical Summary
```bash
# Count syscalls, show time breakdown
strace -c ./my_program

# Output looks like:
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 45.23    0.012345          12      1000           read
 30.12    0.008234           8      1000           write
 15.45    0.004221         422        10           futex
  5.67    0.001548          77        20         5 openat
  3.53    0.000965          48        20           close
------ ----------- ----------- --------- --------- ----------------
100.00    0.027313                  2050         5 total
```

```bash
# Combine summary with full trace
strace -c -S time ./my_program    # sort by time (default)
strace -c -S calls ./my_program   # sort by call count
strace -c -S errors ./my_program  # sort by error count
```

---

## 3. Reading strace Output

### Anatomy of a Line
```
openat(AT_FDCWD, "/etc/passwd", O_RDONLY) = 3
│       │              │           │        │
│       │              │           │        └── return value (fd=3, or -1 on error)
│       │              │           └── flags
│       │              └── argument 2 (string)
│       └── argument 1 (special constant)
└── syscall name
```

### Common Return Values
```
= 0          success (for close, chmod, etc.)
= 3          success, returned file descriptor 3
= 1024       success, 1024 bytes read/written
= -1 ENOENT  error: file not found
= -1 EACCES  error: permission denied
= -1 EAGAIN  error: would block (non-blocking I/O)
= -1 EINTR   error: interrupted by signal
= ?          process exited
```

### Common Syscall Patterns

**Program startup:**
```
execve("./my_program", ["./my_program"], 0x7ffc...) = 0
brk(NULL)                               = 0x55a8f0    # get heap start
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f...
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY) = 3   # load libc
mmap(...)                               = 0x7f...     # map libc into memory
close(3)                                = 0
```

**File I/O:**
```
openat(AT_FDCWD, "data.txt", O_RDONLY)  = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=4096, ...}) = 0
read(3, "Hello World\n...", 4096)       = 4096
read(3, "", 4096)                       = 0     # EOF
close(3)                                = 0
```

**Network (TCP client):**
```
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) = 3
connect(3, {sa_family=AF_INET, sin_port=htons(80), sin_addr=inet_addr("93.184.216.34")}, 16) = 0
write(3, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n", 37) = 37
read(3, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n...", 4096) = 1256
close(3)                                = 0
```

**Thread synchronization (mutexes):**
```
# std::mutex::lock() translates to:
futex(0x7f..., FUTEX_WAIT_PRIVATE, 2, NULL)    # blocked waiting for mutex
futex(0x7f..., FUTEX_WAKE_PRIVATE, 1)          # waking one waiter

# If you see futex with high -T times → lock contention!
```

---

## 4. Real-World Debugging Scenarios

### Scenario 1: "Why is my app slow?"
```bash
strace -c -f ./slow_app

# Look at the output:
% time     seconds  usecs/call    calls  errors  syscall
 78.5     2.345000       2345      1000          futex     ← LOCK CONTENTION!
 15.2     0.456000        456      1000          read
  6.3     0.189000        189      1000          write

# 78.5% time in futex = massive mutex contention
# Solution: reduce lock scope, use lock-free structures, or reader-writer lock
```

```bash
# Drill deeper — which futex addresses are hot?
strace -T -e trace=futex -f ./slow_app 2>&1 | grep "FUTEX_WAIT" | head -20

# See specific wait times:
futex(0x7f1234, FUTEX_WAIT_PRIVATE, 2, NULL) = 0 <0.045123>
#                                                   └── 45ms waiting for this lock!
```

### Scenario 2: "My app can't find a config file"
```bash
strace -e trace=openat ./my_app 2>&1 | grep -i config

# Output:
openat(AT_FDCWD, "/etc/myapp/config.yaml", O_RDONLY) = -1 ENOENT (No such file)
openat(AT_FDCWD, "/usr/local/etc/myapp/config.yaml", O_RDONLY) = -1 ENOENT
openat(AT_FDCWD, "./config.yaml", O_RDONLY) = 3

# Now you know: it tries 3 paths, finds it in current directory
```

### Scenario 3: "Network connection timeout"
```bash
strace -T -e trace=network ./my_client 2>&1

# Output:
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) = 3
connect(3, {sin_port=htons(5432), sin_addr=inet_addr("10.1.2.3")}, 16) = -1 ETIMEDOUT <30.001234>
#                                                                          └── 30 second timeout!

# Diagnosis: server at 10.1.2.3:5432 is unreachable
# Could be: firewall, wrong IP, server down
```

### Scenario 4: "Memory-mapped file issues"
```bash
strace -e trace=mmap,munmap,mprotect ./my_app

# See how the app maps shared memory:
mmap(NULL, 1073741824, PROT_READ|PROT_WRITE, MAP_SHARED, 4, 0) = 0x7f...
#          └── 1 GB shared mapping!

# If this returns -1 ENOMEM → not enough virtual memory
```

### Scenario 5: "Why does startup take 10 seconds?"
```bash
strace -tt -T -o startup_trace.log ./my_server
# Then search for large gaps:
grep -E '<[0-9]+\.' startup_trace.log | head -20

# Lines with <1.234567> mean that syscall took 1.2 seconds
# Common culprits: DNS resolution, loading shared libraries, opening many files
```

---

## 5. strace for Multi-Threaded Applications

```bash
# Always use -f to follow threads
strace -f -o mt_trace.log ./my_threaded_app

# Each line shows the thread PID:
[pid 12345] futex(0x7f..., FUTEX_WAIT, ...) = 0  <0.050>
[pid 12346] write(1, "output\n", 7)              = 7
[pid 12347] read(5, ..., 4096)                   = 4096

# Filter for a specific thread:
grep "\[pid 12345\]" mt_trace.log

# Count syscalls per thread:
strace -c -f ./my_threaded_app
# Shows separate stats per thread
```

---

## 6. Performance Overhead

```
strace overhead: 10-100x slowdown (high — uses ptrace)

Why so slow?
  Every syscall:
    1. Process stops (context switch to strace)
    2. strace reads registers/memory
    3. strace writes log
    4. Process resumes (context switch back)
    = 2 extra context switches per syscall

For low-overhead tracing → use eBPF/bpftrace instead (covered in 07_ebpf.md)

When strace is appropriate:
  - Development/debugging (not production)
  - Infrequent syscalls (startup analysis)
  - When you need exact arguments and return values
  - Quick one-off investigation

For production: use eBPF, perf, or ftrace
```

---

## 7. strace vs Other Tools

| Tool | Overhead | Kernel Needed | Output Detail | Production-Safe |
|------|----------|---------------|---------------|-----------------|
| strace | High (ptrace) | Any | Full args + return values | No |
| ltrace | High (ptrace) | Any | Library calls (not syscalls) | No |
| perf trace | Low (in-kernel) | 3.7+ | Syscalls with lower overhead | Yes |
| bpftrace | Very low (eBPF) | 4.9+ | Custom, programmable | Yes |
| ftrace | Low (in-kernel) | 2.6.27+ | Kernel functions | Yes |

---

## 8. Cheat Sheet — Copy & Paste

```bash
# Quick process overview
strace -c -f ./app                         # syscall stats with threads

# Debugging slow operations
strace -T -f -o trace.log ./app            # time per syscall, all threads
grep -E '<[0-9]{1,}\.' trace.log           # find slow syscalls (>1 sec)

# File issues
strace -e trace=openat -f ./app 2>&1 | grep ENOENT   # missing files
strace -e trace=file -f ./app                         # all file operations

# Network issues
strace -e trace=network -T ./app           # network calls with timing
strace -e connect -f ./app                 # just connection attempts

# Lock contention
strace -T -e futex -f ./app 2>&1 | sort -t'<' -k2 -rn | head  # slowest locks

# Memory issues
strace -e trace=memory ./app               # mmap, brk, mprotect
strace -e brk,mmap,munmap ./app            # heap + mmap activity

# Quick check: what's this process doing right now?
strace -p $(pgrep my_server) -c -f        # attach, Ctrl+C after 10 sec for stats

# Trace specific thread of multi-threaded process
strace -p <THREAD_TID>                     # TID from /proc/<pid>/task/
```

---

## 9. Interview Questions

**Q: "How would you debug a performance issue in a production storage service?"**  
A: "I'd start with `strace -c -f -p <PID>` to get a quick syscall profile — see where time is spent. If I see excessive `futex` calls → lock contention. Excessive `read`/`write` with high `-T` values → I/O bottleneck. For production-safe deep tracing, I'd switch to eBPF-based tools like `bpftrace` or `perf trace` to avoid the ptrace overhead."

**Q: "Your server takes 30 seconds to start. How do you investigate?"**  
A: "`strace -tt -T -f -o startup.log ./server`, then look for syscalls with duration > 1 second. Common causes: DNS resolution (`connect` to port 53), loading large config files (`read` with large counts), shared library loading (`openat` of .so files), database connection (`connect` to DB port with timeout)."

**Q: "How do you know if an application is CPU-bound or I/O-bound?"**  
A: "`strace -c` — if most time is in `read/write/pread/pwrite`, it's I/O-bound. If `strace -c` shows very few syscalls and total time is much less than wall-clock time, it's CPU-bound (time spent in userspace, not in syscalls). For CPU-bound analysis, switch to `perf record`."

---

## Quick Reference Card

```
strace = intercepts every syscall via ptrace

Essential flags:
  -f          follow threads/forks
  -T          show time spent IN each syscall
  -tt         wall clock timestamps
  -c          summary statistics
  -e trace=X  filter (file, network, process, memory, signal)
  -s 256      show full string arguments
  -o file     write to file
  -p PID      attach to running process

Key patterns:
  futex with high -T values → lock contention
  connect with ETIMEDOUT → network unreachable
  openat with ENOENT → missing file
  read returning 0 → EOF / pipe closed
  mmap with ENOMEM → out of memory

Overhead: 10-100x (not for production — use eBPF instead)
```
