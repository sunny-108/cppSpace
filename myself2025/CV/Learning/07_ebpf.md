# eBPF — Extended Berkeley Packet Filter

**Study Time: ~2 hours**  
**Prerequisites:** Basic Linux kernel concepts, strace/perf familiarity

---

## 1. What Is eBPF?

eBPF lets you run **sandboxed programs inside the Linux kernel** without modifying kernel source or loading kernel modules. It's the modern, safe, programmable way to observe and extend kernel behavior.

```
Traditional approach:
  Want to trace all disk I/O? 
  → Modify kernel source → recompile → reboot → danger of crash
  
  Or: Load a kernel module → can crash entire system
  
  Or: Use strace → 100x overhead

eBPF approach:
  Write a small program → kernel verifier checks it's safe →
  attach to kernel event → runs in-kernel with near-zero overhead →
  sends results to userspace
```

### Why It Matters

```
Before eBPF:                        After eBPF:
  strace (slow, unsafe)       →      bpftrace (fast, safe)
  custom kernel modules       →      eBPF programs (verified)
  tcpdump (limited)           →      XDP (line-rate packet processing)
  iptables (slow in kernel)   →      Cilium (eBPF-based networking)
  custom profilers            →      eBPF-based continuous profiling
```

---

## 2. eBPF Architecture

```
┌─────────────────── USERSPACE ───────────────────┐
│                                                  │
│  ┌──────────────┐    ┌──────────────────────┐   │
│  │ bpftrace /   │    │  Your Application     │   │
│  │ bcc tools    │    │  (reads maps/events)  │   │
│  └──────┬───────┘    └──────────┬────────────┘   │
│         │ load program          │ read results    │
│         │                       │                 │
├─────────┼───────────────────────┼─────────────────┤
│         ↓                       ↑                 │
│  ┌──────────────┐    ┌──────────────────────┐    │
│  │  Verifier    │    │    eBPF Maps          │   │
│  │  (safety     │    │ (hash/array/ringbuf)  │   │
│  │   checks)    │    └──────────┬────────────┘   │
│  └──────┬───────┘               │                │
│         ↓                       │                │
│  ┌──────────────┐               │                │
│  │  JIT Compiler│               │                │
│  │  (to native) │               │                │
│  └──────┬───────┘               │                │
│         ↓                       │                │
│  ┌──────────────────────────────┴──────────┐     │
│  │         eBPF Program (running in kernel) │    │
│  │  Attached to: kprobe / tracepoint / XDP  │    │
│  └──────────────────────────────────────────┘    │
│                                                   │
│                    KERNEL                          │
└───────────────────────────────────────────────────┘
```

### Key Components

| Component | Role |
|-----------|------|
| **eBPF Program** | Small C-like program that runs in kernel VM |
| **Verifier** | Ensures program is safe: no infinite loops, no invalid memory access, bounded execution time |
| **JIT Compiler** | Compiles eBPF bytecode to native CPU instructions for speed |
| **Maps** | Shared data structures between eBPF programs and userspace (hash tables, arrays, ring buffers) |
| **Helper Functions** | Kernel-provided APIs: get current time, PID, access packet data, etc. |

### Attach Points (Where eBPF Programs Run)

```
┌─────────────────────────────────────────────────────────────┐
│  Userspace                                                   │
│    uprobe ──→ attach to any userspace function               │
│    USDT   ──→ user-level static tracepoints                  │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│  Syscall boundary                                            │
│    tracepoint:syscalls ──→ sys_enter_read, sys_exit_write   │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│  Kernel                                                      │
│    kprobe      ──→ attach to ANY kernel function             │
│    kretprobe   ──→ function return (capture return value)    │
│    tracepoint  ──→ stable kernel trace points                │
│    fentry/fexit ──→ faster kprobe alternative (5.5+)         │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│  Network                                                     │
│    XDP         ──→ earliest point in packet receive path     │
│    tc          ──→ traffic control (after XDP)               │
│    socket      ──→ socket-level filtering                    │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│  Other                                                       │
│    perf_event  ──→ hardware performance counters             │
│    cgroup      ──→ container resource control                │
│    LSM         ──→ security module hooks                     │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Verifier — Why eBPF Is Safe

The kernel verifier analyzes every eBPF program before allowing it to run:

```
Checks performed:
  1. No infinite loops (all loops must have bounded iteration count)
  2. No out-of-bounds memory access
  3. All code paths terminate
  4. No null pointer dereferences
  5. Stack size limited (512 bytes per program)
  6. Maximum instruction count (1 million verified instructions)
  7. Only approved helper functions can be called
  8. Memory access must be within registered regions

If verification fails → program is rejected → never runs
This is why eBPF can't crash the kernel.
```

---

## 4. eBPF Maps — Kernel ↔ Userspace Communication

```
Maps are key-value data structures shared between:
  - eBPF programs (in kernel)
  - Userspace applications (your tool)

Map types:
  BPF_MAP_TYPE_HASH          → general-purpose hash table
  BPF_MAP_TYPE_ARRAY         → fixed-size array
  BPF_MAP_TYPE_PERF_EVENT_ARRAY → send events to userspace (perf buffer)
  BPF_MAP_TYPE_RINGBUF       → efficient ring buffer (kernel 5.8+)
  BPF_MAP_TYPE_LRU_HASH      → hash with LRU eviction
  BPF_MAP_TYPE_STACK_TRACE   → collect stack traces
  BPF_MAP_TYPE_LPM_TRIE      → longest prefix match (routing tables)

Usage pattern:
  eBPF program (kernel):
    map_update(key, value)   // write data
    value = map_lookup(key)  // read data
  
  Userspace:
    bpf_map_lookup_elem()    // read what eBPF program wrote
    bpf_map_update_elem()    // configure the eBPF program
```

---

## 5. bpftrace — The Easy Way to Use eBPF

bpftrace is a high-level tracing language (like awk for eBPF).

### Installation
```bash
# Ubuntu/Debian
sudo apt install bpftrace

# RHEL/Rocky
sudo dnf install bpftrace

# macOS: not available (eBPF is Linux-only)
# Use a Linux VM or Docker container
```

### One-Liner Examples

```bash
# 1. Trace all open() syscalls with filename
sudo bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("%s %s\n", comm, str(args.filename)); }'
# Output: bash /etc/passwd
#         vim /home/user/.vimrc

# 2. Count syscalls by program
sudo bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @[comm] = count(); }'
# Ctrl+C to see:
# @[nginx]: 45023
# @[postgres]: 12456
# @[bash]: 234

# 3. Histogram of read() sizes
sudo bpftrace -e 'tracepoint:syscalls:sys_enter_read { @bytes = hist(args.count); }'
# Output: distribution of read request sizes

# 4. Trace disk I/O latency
sudo bpftrace -e '
kprobe:blk_account_io_start { @start[arg0] = nsecs; }
kprobe:blk_account_io_done /@start[arg0]/ {
    @usecs = hist((nsecs - @start[arg0]) / 1000);
    delete(@start[arg0]);
}'

# 5. Track malloc() calls larger than 1MB
sudo bpftrace -e 'uprobe:/lib/x86_64-linux-gnu/libc.so.6:malloc /arg0 > 1048576/ {
    printf("PID %d (%s) malloc %d bytes\n", pid, comm, arg0);
}'

# 6. Who's causing disk writes?
sudo bpftrace -e 'kprobe:vfs_write { @[comm] = count(); }'

# 7. TCP connection latency (connect time)
sudo bpftrace -e '
kprobe:tcp_v4_connect { @start[tid] = nsecs; }
kretprobe:tcp_v4_connect /@start[tid]/ {
    @connect_ms = hist((nsecs - @start[tid]) / 1000000);
    delete(@start[tid]);
}'

# 8. Function call frequency in your program
sudo bpftrace -e 'uprobe:/path/to/my_program:processData { @calls = count(); }'

# 9. Trace mutex contention (futex waits)
sudo bpftrace -e '
tracepoint:syscalls:sys_enter_futex /args.op == 0/ { @start[tid] = nsecs; }
tracepoint:syscalls:sys_exit_futex /@start[tid]/ {
    @futex_wait_ns = hist(nsecs - @start[tid]);
    delete(@start[tid]);
}'
```

### bpftrace Language Basics
```
Syntax:  probe /filter/ { actions }

Probes:
  kprobe:function_name          → kernel function entry
  kretprobe:function_name       → kernel function return
  tracepoint:category:name      → kernel tracepoint
  uprobe:/path/to/binary:func   → userspace function entry
  uretprobe:/path/to/binary:func → userspace function return
  interval:s:1                  → run every 1 second
  BEGIN                         → run once at start
  END                           → run once at end

Built-in variables:
  pid       → process ID
  tid       → thread ID
  comm      → process name
  nsecs     → nanosecond timestamp
  arg0-argN → function arguments
  retval    → return value (kretprobe/uretprobe)
  @name     → map (persists across events)

Aggregation functions:
  count()   → count events
  sum(x)    → sum values
  avg(x)    → average
  min(x)    → minimum
  max(x)    → maximum
  hist(x)   → power-of-2 histogram
  lhist(x, min, max, step) → linear histogram
```

---

## 6. BCC Tools — Pre-Built eBPF Tools

BCC (BPF Compiler Collection) provides ready-to-use tools:

```bash
# Install
sudo apt install bpfcc-tools  # Ubuntu
# Tools are installed as *-bpfcc

# File I/O
sudo opensnoop-bpfcc           # trace file opens
sudo filelife-bpfcc            # trace short-lived files
sudo fileslower-bpfcc 10       # files taking >10ms to open

# Disk I/O
sudo biolatency-bpfcc          # histogram of disk I/O latency
sudo biosnoop-bpfcc            # trace each disk I/O
sudo biotop-bpfcc              # top-like for disk I/O

# Network
sudo tcpconnect-bpfcc          # trace outbound TCP connections
sudo tcpaccept-bpfcc           # trace inbound TCP connections
sudo tcpretrans-bpfcc          # trace TCP retransmits (network issues!)
sudo tcplife-bpfcc             # TCP connection lifespan

# Memory
sudo memleak-bpfcc -p PID      # detect memory leaks (!!!)
sudo cachestat-bpfcc           # page cache hit/miss stats
sudo slabtop                   # kernel slab allocator

# CPU / Scheduling
sudo runqlat-bpfcc             # CPU scheduler run queue latency
sudo cpudist-bpfcc             # on-CPU time distribution
sudo offcputime-bpfcc -p PID   # where time is spent OFF-CPU (blocking)

# Process
sudo execsnoop-bpfcc           # trace new processes (exec)
sudo exitsnoop-bpfcc           # trace process exits
```

### Most Useful BCC Tools for Storage Engineering

```bash
# 1. Disk I/O latency — is storage slow?
sudo biolatency-bpfcc
# Shows histogram: most I/O under 100us, but some outliers at 50ms → investigate

# 2. TCP retransmits — network issues?
sudo tcpretrans-bpfcc
# Shows every retransmission: source, dest, state
# High retransmits → packet loss → check switch, NIC, network

# 3. Memory leaks — automated detection
sudo memleak-bpfcc -p $(pgrep my_server) --top 10
# Shows top 10 allocation sites with leaked bytes
# Like Valgrind but MUCH lower overhead (can run in production)

# 4. Off-CPU analysis — where's my thread blocked?
sudo offcputime-bpfcc -p PID -f > offcpu.stacks
# Shows stack traces of where threads are sleeping/blocked
# Use with FlameGraph to create off-CPU flame graphs
```

---

## 7. eBPF Use Cases in Production

### Observability (What the Dell JD Cares About)
```
1. Continuous profiling:
   - Attach to CPU cycles event → always-on profiling (~1% overhead)
   - Tools: Parca, Pyroscope, Grafana Alloy
   
2. Distributed tracing correlation:
   - Trace requests across services without application changes
   - Attach to network syscalls → correlate by TCP connection
   
3. Custom metrics:
   - "Show me P99 latency of disk writes > 4KB to NVMe devices"
   - Write a bpftrace one-liner → get the answer in seconds
```

### Networking (Cilium)
```
Cilium uses eBPF to replace iptables for Kubernetes networking:
  - kube-proxy-free service mesh
  - Network policies enforced in-kernel
  - Load balancing at XDP level (before TCP stack — very fast)
  
Why faster than iptables:
  iptables: linear chain of rules, evaluated per packet
  eBPF: compiled program, O(1) hash lookups for rules
```

### Security (Falco, Tetragon)
```
Runtime security monitoring:
  - Detect: unexpected file access, network connections, process execution
  - Enforce: kill processes violating policy
  - All in-kernel, near-zero overhead
```

---

## 8. Writing Custom eBPF Programs (libbpf / C)

For interview understanding — you don't need to write this, but should understand the structure:

```c
// my_tracer.bpf.c — runs in kernel
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

// Map: store per-PID byte counts
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, __u32);    // PID
    __type(value, __u64);  // byte count
} bytes_read SEC(".maps");

// Attach to read() syscall exit
SEC("tracepoint/syscalls/sys_exit_read")
int trace_read_exit(struct trace_event_raw_sys_exit *ctx) {
    __u32 pid = bpf_get_current_pid_tgid() >> 32;
    __s64 ret = ctx->ret;
    
    if (ret <= 0) return 0;  // error or EOF
    
    __u64 *val = bpf_map_lookup_elem(&bytes_read, &pid);
    if (val) {
        *val += ret;
    } else {
        __u64 initial = ret;
        bpf_map_update_elem(&bytes_read, &pid, &initial, BPF_ANY);
    }
    
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
```

```c
// my_tracer.c — runs in userspace (loads and reads maps)
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

int main() {
    struct bpf_object *obj = bpf_object__open_file("my_tracer.bpf.o", NULL);
    bpf_object__load(obj);
    // ... attach programs, poll maps, display results ...
}
```

---

## 9. Limitations

```
1. Kernel version requirements:
   - Basic eBPF: Linux 3.15+
   - Tracepoints: 4.7+
   - bpftrace: 4.9+
   - BTF (better type info): 5.2+
   - Ring buffer: 5.8+
   - For best experience: Linux 5.8+

2. Verifier constraints:
   - Max 1M verified instructions
   - 512-byte stack limit
   - No unbounded loops (before 5.3), bounded loops (5.3+)
   - Can't sleep or allocate memory

3. macOS: NOT supported (use Linux VM/Docker for learning)

4. Security: requires root or CAP_BPF capability

5. Debugging eBPF programs is harder than userspace code
```

---

## 10. Interview Questions

**Q: "What is eBPF and how would you use it in a storage system?"**  
A: "eBPF lets you run safe, verified programs inside the Linux kernel without modifying kernel source. For a storage system, I'd use it for: disk I/O latency histograms (biolatency), TCP retransmit monitoring (tcpretrans), memory leak detection in production (memleak), and custom metrics like per-client throughput. The key advantage over strace is near-zero overhead — safe for production profiling."

**Q: "How is eBPF different from a kernel module?"**  
A: "Kernel modules can access anything and crash the kernel. eBPF programs are verified before loading — the verifier proves they can't loop forever, access invalid memory, or crash. eBPF is sandboxed and safe; kernel modules are not. The tradeoff is eBPF programs are more limited in what they can do."

**Q: "How would you detect a memory leak in production without restarting?"**  
A: "Use eBPF-based memleak tool — it attaches uprobes to malloc/free, tracks allocations, and reports sites where allocations are never freed. Unlike Valgrind (which requires restarting with instrumentation and adds 20x overhead), memleak attaches to a running process with ~5% overhead."

---

## Quick Reference Card

```
eBPF = safe, verified programs running inside the Linux kernel

Architecture:
  Program → Verifier → JIT → attach to kernel event → results via Maps

Attach points:
  kprobe/kretprobe  → any kernel function
  tracepoint         → stable kernel trace points
  uprobe/uretprobe  → any userspace function
  XDP               → network packets (fastest)

Tools:
  bpftrace          → one-liner scripts (like awk for tracing)
  BCC tools         → pre-built tools (biolatency, tcpretrans, memleak)
  libbpf            → C library for custom eBPF programs

Key BCC tools for storage:
  biolatency  → disk I/O latency
  tcpretrans  → network retransmits
  memleak     → memory leak detection (production-safe!)
  offcputime  → where threads are blocked
  runqlat     → CPU scheduling latency

Requirements: Linux 4.9+ (5.8+ preferred), root access
Overhead: ~1-5% (safe for production)
```
