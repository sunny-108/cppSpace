# perf — Linux Performance Profiling

**Study Time: ~1.5 hours**  
**Prerequisites:** Basic Linux, understanding of CPU cycles, function call stacks

---

## 1. What Is perf?

`perf` is the Linux kernel's built-in performance analysis tool. It uses hardware performance counters (PMCs) in the CPU to measure events with near-zero overhead.

```
strace → "what syscalls is my program making?"
perf   → "where is my program spending CPU time, and why?"
```

**Key difference from strace:** perf samples the CPU state at intervals (statistical profiling), rather than tracing every event. This means very low overhead (~1-5%) and safe for production.

---

## 2. perf Subcommands Overview

| Command | Purpose |
|---------|---------|
| `perf stat` | Count hardware events (cache misses, instructions, cycles) |
| `perf record` | Sample call stacks for CPU profiling |
| `perf report` | Analyze recorded samples (interactive TUI) |
| `perf top` | Live top-like view of hot functions |
| `perf trace` | Low-overhead syscall tracing (like strace but faster) |
| `perf annotate` | Show assembly with event counts per instruction |
| `perf list` | List available events to measure |

---

## 3. perf stat — Hardware Event Counting

### Basic Usage
```bash
# Count default events for a command
perf stat ./my_program

# Output:
 Performance counter stats for './my_program':

          1,523.45 msec  task-clock                #    0.987 CPUs utilized
                45      context-switches           #   29.539 /sec
                 3      cpu-migrations             #    1.970 /sec
            12,456      page-faults                #    8.178 K/sec
     4,567,890,123      cycles                     #    2.998 GHz
     3,210,654,789      instructions               #    0.70  insn per cycle  ← IPC
       456,789,012      branches                   #  299.820 M/sec
        12,345,678      branch-misses              #    2.70% of all branches ← bad if >5%
```

### Key Metrics Explained

```
IPC (Instructions Per Cycle):
  instructions / cycles
  
  IPC > 1.0: CPU-efficient code (compute-bound, cache-friendly)
  IPC < 0.5: Likely memory-bound (waiting for cache/memory)
  IPC ~ 0.1: Severe memory stalls
  
  Modern CPUs can retire 4-6 instructions/cycle at peak.
  Typical well-optimized code: IPC 1.0-2.0

Cache Misses:
  L1 cache hit:  ~1 ns  (4 cycles)
  L2 cache hit:  ~5 ns  (15 cycles)
  L3 cache hit:  ~15 ns (50 cycles)
  Main memory:   ~100 ns (300 cycles)  ← 75x slower than L1!
  
  High cache-miss rate → memory-bound → IPC drops

Branch Misprediction:
  CPU predicts branch direction (taken/not-taken) speculatively
  Misprediction: ~15-20 cycles wasted (pipeline flush)
  > 5% misprediction rate → worth investigating
  Causes: unpredictable if/else, virtual function calls, switch on random data
```

### Targeted Event Counting
```bash
# Specific cache events
perf stat -e cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses ./my_program

# Memory bandwidth
perf stat -e dTLB-load-misses,dTLB-store-misses ./my_program

# All available events
perf list   # shows hundreds of hardware/software events

# Count for a running process (10 seconds)
perf stat -p 12345 sleep 10

# Per-thread breakdown
perf stat -p 12345 --per-thread sleep 10
```

---

## 4. perf record + perf report — CPU Profiling

### Recording Samples
```bash
# Record CPU samples (default: cycles event, 4000 Hz sampling)
perf record -g ./my_program
# Creates perf.data file

# Record with call graph (DWARF unwinding — most accurate)
perf record -g --call-graph dwarf ./my_program

# Record a running process for 30 seconds
perf record -g -p 12345 -- sleep 30

# Record all threads
perf record -g -p 12345 --per-thread -- sleep 30

# Higher sampling frequency (more detail, more overhead)
perf record -F 9999 -g ./my_program
```

### Analyzing Results
```bash
# Interactive TUI
perf report

# Output (simplified):
Overhead  Command      Shared Object       Symbol
  35.20%  my_program   my_program          [.] processData
  22.10%  my_program   libc.so.6           [.] memcpy
  15.30%  my_program   my_program          [.] hashFunction
   8.50%  my_program   libpthread.so.0     [.] pthread_mutex_lock
   5.20%  my_program   my_program          [.] parseInput

# Navigation in TUI:
#   Enter → expand call graph
#   + → expand, - → collapse
#   a → annotate (show assembly)
#   q → quit
```

### Reading the Call Graph
```bash
perf report --stdio   # text output (good for scripts/sharing)

# Call graph shows WHO called the hot functions:
  35.20%  my_program  [.] processData
          |
          --- processData
              |
              |--60.00%-- handleRequest
              |          |
              |          --100%-- main_loop
              |
              --40.00%-- batchProcess
                         |
                         --100%-- timer_callback

# Reading: 60% of processData's samples came from handleRequest path
#          40% came from batchProcess path
```

---

## 5. perf top — Live Profiling

```bash
# Live view of hot functions across the whole system
sudo perf top

# For a specific process
sudo perf top -p 12345

# With call graph
sudo perf top -g -p 12345

# Output (updates in real-time):
Overhead  Shared Object        Symbol
  12.34%  my_server            [.] hash_lookup
   8.56%  libc.so.6            [.] malloc
   6.78%  my_server            [.] serialize_response
   5.43%  [kernel]             [k] copy_user_enhanced_fast_string
   4.32%  libpthread.so.0      [.] __pthread_mutex_lock
```

**Pro tip:** If you see kernel functions (`[k]`) taking significant time, your app is syscall-heavy. If it's all userspace (`[.]`), it's CPU-bound.

---

## 6. perf annotate — Instruction-Level Analysis

```bash
# After perf record:
perf annotate processData

# Shows assembly with sample counts:
       │     processData():
       │       push   %rbp
       │       mov    %rsp,%rbp
  2.30 │       mov    (%rdi),%rax        ← 2.3% of samples here
 45.60 │       cmp    %rax,(%rsi)        ← 45.6%! Cache miss likely
  1.20 │       je     40123a
 12.80 │       mov    0x8(%rdi),%rcx     ← another memory access
       │       add    %rcx,%rax
       │       ret

# The cmp on line 4 taking 45.6% strongly suggests a cache miss
# (CPU stalls waiting for memory → accumulates samples at that IP)
```

---

## 7. perf trace — Low-Overhead strace

```bash
# Like strace but uses kernel tracing (much lower overhead)
perf trace ./my_program

# Filter to specific syscalls
perf trace -e read,write,openat ./my_program

# Summary mode (like strace -c)
perf trace --summary ./my_program

# Trace a running process
perf trace -p 12345

# Overhead comparison:
# strace:     ~100x slowdown (ptrace)
# perf trace: ~2-5x slowdown (kernel-based)
```

---

## 8. Practical Debugging Workflows

### Workflow 1: "My Program Is Slow — Where?"
```bash
# Step 1: Quick overview
perf stat ./my_program
# Check IPC: < 0.5 → memory-bound, > 1.0 → compute-bound

# Step 2: Find hot functions
perf record -g ./my_program
perf report
# Top function taking 40%? That's your bottleneck.

# Step 3: Drill into the hot function
perf annotate hot_function
# Find the exact instruction that's stalling (highest sample %)
```

### Workflow 2: "Is This Cache-Friendly?"
```bash
# Compare two implementations:
perf stat -e cache-misses,cache-references,instructions,cycles ./version_a
perf stat -e cache-misses,cache-references,instructions,cycles ./version_b

# version_a: cache-miss rate 15%, IPC 0.4
# version_b: cache-miss rate 2%,  IPC 1.8
# version_b is 4.5x more CPU-efficient due to cache locality
```

### Workflow 3: "Lock Contention?"
```bash
# Check if mutex/futex is dominating
perf record -g ./threaded_app
perf report

# If __pthread_mutex_lock or futex shows high overhead:
# → Lock contention is the bottleneck
# Solutions: reduce critical section size, use reader-writer lock,
#            use lock-free structures, partition data by thread
```

### Workflow 4: "Is This I/O-Bound or CPU-Bound?"
```bash
perf stat ./my_program

# I/O-bound indicators:
#   - task-clock << wall time (CPUs utilized < 1.0)
#   - High context-switch count
#   - Low instruction count relative to time
#
# CPU-bound indicators:
#   - task-clock ≈ wall time × num_cores
#   - CPUs utilized close to core count
#   - High instruction/cycle count
```

---

## 9. Flame Graphs (Visualization)

perf report's TUI is useful but flame graphs give a much better overview:

```bash
# Step 1: Record
perf record -g --call-graph dwarf ./my_program

# Step 2: Generate flame graph (using Brendan Gregg's scripts)
git clone https://github.com/brendangregg/FlameGraph.git

perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > flamegraph.svg

# Step 3: Open flamegraph.svg in browser
# Width of each box = % of CPU time in that function
# Taller stacks = deeper call chains
# Wide boxes at the top = where you should optimize
```

```
Reading a flame graph:
  
  ┌───────────────────────────────────────────────────┐
  │                    main()                          │  ← bottom = root
  ├────────────────────────┬──────────────────────────┤
  │    handleRequest()     │     batchProcess()       │
  ├──────────┬─────────────┤                          │
  │processData│  serialize  │                          │
  ├──────────┤             │                          │
  │hash_lookup│             │                          │  ← top = leaf (where time is spent)
  └──────────┴─────────────┴──────────────────────────┘
  
  Wide box = lots of CPU time
  hash_lookup is narrow but deep → optimize if it's a wide leaf
  handleRequest is wide → lots of time in this subtree
```

---

## 10. Hardware Performance Counters — Cheat Sheet

```bash
# Memory hierarchy analysis
perf stat -e \
  L1-dcache-loads,L1-dcache-load-misses,\
  L1-icache-load-misses,\
  LLC-loads,LLC-load-misses,\
  dTLB-load-misses,iTLB-load-misses \
  ./my_program

# Branch prediction analysis
perf stat -e \
  branches,branch-misses \
  ./my_program

# Instruction mix
perf stat -e \
  instructions,cycles,\
  fp_arith_inst_retired.scalar_single,\
  fp_arith_inst_retired.scalar_double \
  ./my_program
```

---

## 11. Interview Questions

**Q: "How would you profile a C++ storage service to find performance bottlenecks?"**  
A: "Start with `perf stat` to get IPC and cache-miss rates — this tells me if it's CPU-bound or memory-bound. Then `perf record -g` + `perf report` to find hot functions. If the top function is `memcpy` or `malloc`, I'd look at memory allocation patterns. If it's a mutex function, lock contention. For a visual overview, I'd generate a flame graph."

**Q: "What does a low IPC mean and how do you fix it?"**  
A: "Low IPC (< 0.5) usually means the CPU is stalled waiting for memory. I'd run `perf stat` with cache-miss events to confirm. Fixes include: improving data locality (struct of arrays vs array of structs), reducing working set size, using prefetch hints, aligning data to cache lines, avoiding pointer chasing."

**Q: "How is perf different from strace?"**  
A: "strace traces every syscall via ptrace (high overhead, full detail). perf uses hardware counters and sampling — statistical profiling with very low overhead. strace tells you WHAT syscalls are happening; perf tells you WHERE CPU time is spent. For production, always perf or eBPF. strace is for development/debugging only."

---

## Quick Reference Card

```
perf stat ./app           → hardware counters (IPC, cache misses, branches)
perf record -g ./app      → CPU profile (creates perf.data)
perf report               → analyze profile (interactive TUI)
perf top -p PID           → live profiling (like top but for functions)
perf annotate func        → instruction-level breakdown
perf trace ./app          → low-overhead syscall trace

Key metrics:
  IPC > 1.0: CPU-efficient       IPC < 0.5: memory-bound
  cache-miss > 10%: fix locality  branch-miss > 5%: unpredictable branches
  
Flame graphs: perf script | stackcollapse-perf.pl | flamegraph.pl > out.svg

Overhead: ~1-5% (safe for production)

Quick workflow: perf stat → perf record → perf report → perf annotate
```
