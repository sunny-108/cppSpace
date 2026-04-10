# RDMA — Remote Direct Memory Access

**Study Time: ~1.5 hours**  
**Prerequisites:** TCP basics, understanding of kernel vs userspace, DMA concept

---

## 1. What is RDMA?

RDMA allows one computer to directly read/write the memory of another computer **without involving the remote CPU or operating system**. The network card (NIC) handles everything.

```
Traditional Network I/O:
  App1 → write() → kernel → TCP stack → copy to NIC buffer → NIC → wire →
  → remote NIC → copy to kernel buffer → kernel → TCP stack → copy to app buffer → App2
  
  Problems:
    - 2 context switches per direction (user↔kernel)
    - 2-3 memory copies (app→kernel→NIC, NIC→kernel→app)
    - CPU on BOTH sides does significant work
    - Each step adds latency

RDMA:
  App1 → post RDMA verb → NIC reads directly from app memory → wire →
  → remote NIC → writes directly to remote app memory
  
  Benefits:
    - Zero copy (NIC accesses app memory directly via DMA)
    - Kernel bypass (no syscalls on the data path)
    - Zero CPU on remote side (for one-sided operations)
    - Microsecond latency (1-5 μs vs 50-100 μs for TCP)
```

### Performance Comparison

| Metric | TCP/IP | RDMA |
|--------|--------|------|
| Latency | 50-100 μs | 1-5 μs |
| Throughput | ~10-40 Gbps | 100-400 Gbps |
| CPU usage | High (both sides) | Near zero (one-sided ops) |
| Memory copies | 2-3 per transfer | 0 (zero-copy) |
| Context switches | 2 per operation | 0 (kernel bypass) |

---

## 2. RDMA Hardware & Transport Types

### Three RDMA Technologies

```
1. InfiniBand (IB):
   - Dedicated RDMA network (separate from Ethernet)
   - Requires InfiniBand switches and HCAs (Host Channel Adapters)
   - Highest performance: 100-400 Gbps, sub-microsecond latency
   - Used in: HPC clusters, AI/ML training (NVIDIA DGX), storage arrays
   - Most mature RDMA implementation

2. RoCE (RDMA over Converged Ethernet):
   - RDMA running over standard Ethernet infrastructure
   - RoCE v1: L2 only (same subnet)
   - RoCE v2: L3 routable (UDP encapsulation — can cross subnets)
   - Requires: RDMA-capable NIC (Mellanox/NVIDIA ConnectX, Broadcom)
   - Most popular for data centers transitioning to RDMA
   - Used by: Azure, Google, Meta data centers

3. iWARP (Internet Wide Area RDMA Protocol):
   - RDMA over TCP/IP
   - Works with standard network infrastructure
   - Slightly higher latency than RoCE (TCP overhead)
   - Better for lossy networks (TCP handles retransmission)
   - Less popular, mostly Intel/Chelsio NICs
```

### What You Need to Know for Interviews

```
"RDMA is most commonly deployed using either InfiniBand (for HPC/AI) 
or RoCE v2 (for data center storage and networking). RoCE v2 runs over 
standard Ethernet with UDP encapsulation, making it deployable on 
existing network infrastructure with RDMA-capable NICs. The Dell/HPE 
storage systems likely use RoCE v2 for inter-node data replication, 
as it provides near-InfiniBand performance without dedicated switching."
```

---

## 3. RDMA Core Concepts

### Queue Pair (QP)

```
A Queue Pair is the fundamental communication endpoint in RDMA.
Think of it as the RDMA equivalent of a socket.

┌──────────────────┐
│    Queue Pair     │
│  ┌──────────────┐ │
│  │  Send Queue  │ │ ← post send Work Requests here
│  │  (SQ)        │ │
│  ├──────────────┤ │
│  │  Receive     │ │ ← post receive Work Requests here
│  │  Queue (RQ)  │ │
│  └──────────────┘ │
└──────────────────┘

Each QP is associated with:
  - A Completion Queue (CQ) — where completions are posted
  - A Protection Domain (PD) — for access control
  - Connection state — connected to a remote QP

Types of QPs:
  RC (Reliable Connected):  TCP-like, guaranteed delivery, most common
  UC (Unreliable Connected): No ack, no retry (rarely used)
  UD (Unreliable Datagram):  UDP-like, supports multicast
```

### Completion Queue (CQ)

```
When a Work Request finishes, the NIC posts a Completion Queue Entry (CQE):

Application posts WR → NIC processes → NIC posts CQE to CQ
                                        ↓
                          Application polls CQ for completions

Polling modes:
  1. Busy-polling: app continuously checks CQ (lowest latency, burns CPU)
     while (poll_cq(cq, &wc) == 0) { /* spin */ }
  
  2. Event-driven: block until CQ has entries (saves CPU, higher latency)
     req_notify_cq(cq);       // arm notification
     get_cq_event(cq_channel); // block until event
     poll_cq(cq, &wc);        // read completions

Storage systems typically use busy-polling on dedicated cores for 
lowest latency, or a hybrid approach.
```

### Memory Region (MR)

```
Before RDMA can access application memory, that memory must be 
REGISTERED with the NIC:

1. Application allocates memory (malloc, mmap, huge pages)

2. Register with NIC:
   mr = ibv_reg_mr(pd, buffer, size, access_flags);
   
   This call:
   a) Pins the pages in physical memory (can't be swapped)
   b) Programs the NIC's translation tables (virtual → physical)
   c) Returns a local key (lkey) and remote key (rkey)
   
3. The NIC can now DMA to/from this memory without kernel involvement

4. For remote access (RDMA Read/Write), share the rkey with the remote side

Memory registration is EXPENSIVE (takes milliseconds):
  - Must be done upfront, not on the hot path
  - Common pattern: register large buffers at startup, reuse them
  - Advanced: On-Demand Paging (ODP) — register lazily, kernel handles faults
```

### Protection Domain (PD)

```
Groups RDMA resources for access control:
  - QPs can only access MRs in the same PD
  - Prevents one application from accessing another's memory

pd = ibv_alloc_pd(context);
qp = ibv_create_qp(pd, &qp_init_attr);
mr = ibv_reg_mr(pd, buffer, size, flags);
// qp can access mr because they share pd
```

---

## 4. RDMA Operations

### Two-Sided Operations (Send/Receive)

```
Both sides must participate — like traditional messaging but kernel-bypass.

Sender:                          Receiver:
  Post Send WR                   Post Receive WR (BEFORE sender sends!)
       ↓                              ↓
  NIC reads data from             NIC waiting for incoming data
  sender's memory                      ↓
       ↓                         NIC writes data to receiver's
  Data on wire →→→→→→→           pre-posted receive buffer
       ↓                              ↓
  Send CQE posted                Receive CQE posted
  (send completed)               (data arrived, in buffer)

Key points:
  - Receiver must post receive buffer BEFORE sender sends
  - If no receive buffer posted → data is dropped (on RC: error)
  - Both sides get a completion notification
  - Used for: control messages, small metadata exchanges
```

### One-Sided Operations: RDMA Write

```
Sender writes directly into remote memory. Remote CPU is NOT involved.

Setup:
  1. Receiver registers memory region → gets (rkey, remote_addr)
  2. Receiver sends rkey + remote_addr to sender (via Send/Recv or out-of-band)

Operation:
  Sender:                        Receiver:
    post RDMA Write WR           (does nothing — not involved!)
    (includes: rkey,              
     remote_addr, length,        NIC receives data
     local buffer addr)          NIC writes to registered memory
         ↓                       using DMA (no CPU)
    NIC reads local buffer            ↓
    Data on wire →→→→→→→         Data appears in receiver's memory
         ↓                       (receiver doesn't even know!)
    Write CQE posted             
    (write completed)            No CQE on receiver side (by default)

Used for:
  - Bulk data transfer (replication, backup/restore)
  - Distributed shared memory
  - Very high throughput scenarios (NIC does everything)

The receiver can detect writes via:
  - Polling a known memory location (check a flag/counter)
  - RDMA Write with Immediate (generates CQE on receiver)
```

### One-Sided Operations: RDMA Read

```
Sender READS from remote memory. Remote CPU not involved.

Sender:                          Receiver:
  post RDMA Read WR              (not involved!)
  (includes: rkey,
   remote_addr, length,          NIC receives read request
   local buffer addr)            NIC reads from registered memory
       ↓                         NIC sends data back
  NIC sends read request →→→     
       ↓                              ↓
  NIC receives data ←←←←←←      
  NIC writes to local buffer
       ↓
  Read CQE posted
  (data available locally)

Used for:
  - Reading remote metadata/indexes without bothering remote CPU
  - Distributed hash table lookups
  - Key-value store gets (client reads directly from server memory)
```

### Atomic Operations

```
RDMA supports remote atomic operations:

1. Compare-and-Swap (CAS):
   if (remote_memory[addr] == expected) {
       remote_memory[addr] = desired;
       return expected;    // success
   } else {
       return current;     // fail
   }

2. Fetch-and-Add:
   old = remote_memory[addr];
   remote_memory[addr] += value;
   return old;

Used for: distributed locks, counters, leader election
Performed atomically by the remote NIC — no CPU involvement.
```

---

## 5. RDMA Programming Model (Verbs API)

```c
// Simplified flow — NOT compilable, just to show the pattern

// 1. Open device and create resources
struct ibv_context *ctx = ibv_open_device(dev);
struct ibv_pd *pd = ibv_alloc_pd(ctx);
struct ibv_cq *cq = ibv_create_cq(ctx, 100, NULL, NULL, 0);

// 2. Create Queue Pair
struct ibv_qp_init_attr qp_attr = {
    .send_cq = cq,
    .recv_cq = cq,
    .cap = { .max_send_wr = 64, .max_recv_wr = 64 },
    .qp_type = IBV_QPT_RC,  // Reliable Connected
};
struct ibv_qp *qp = ibv_create_qp(pd, &qp_attr);

// 3. Register memory
char *buffer = malloc(1024 * 1024);  // 1 MB
struct ibv_mr *mr = ibv_reg_mr(pd, buffer, 1024*1024,
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
// mr->lkey = local access key
// mr->rkey = remote access key (share with remote side)

// 4. Connect QP to remote QP (exchange info via TCP or other side channel)
// ... modify QP state: RESET → INIT → RTR → RTS ...

// 5. Post an RDMA Write
struct ibv_sge sge = {
    .addr = (uint64_t)buffer,
    .length = 4096,
    .lkey = mr->lkey,
};
struct ibv_send_wr wr = {
    .opcode = IBV_WR_RDMA_WRITE,
    .sg_list = &sge,
    .num_sge = 1,
    .wr.rdma = {
        .remote_addr = remote_addr,  // from remote side
        .rkey = remote_rkey,         // from remote side
    },
    .send_flags = IBV_SEND_SIGNALED,
};
ibv_post_send(qp, &wr, &bad_wr);

// 6. Poll for completion
struct ibv_wc wc;
while (ibv_poll_cq(cq, 1, &wc) == 0) { /* spin */ }
if (wc.status == IBV_WC_SUCCESS) {
    // RDMA Write completed successfully
}
```

---

## 6. Why Storage Systems Use RDMA

```
NVMe over Fabrics (NVMe-oF):
  - Access remote NVMe SSDs as if they're local
  - RDMA transport: NVMe commands + data via RDMA Write/Read
  - Latency: ~10 μs (vs ~100 μs for iSCSI/TCP)
  - Linux kernel support: nvme-rdma driver

Distributed Storage Data Path:
  Client write:
    App → local node → RDMA Write to replica nodes → CQE → committed
    
    With TCP: 100 μs × N replicas (sequential or parallel)
    With RDMA: 5 μs × N replicas (and zero remote CPU usage!)

  This is critical at scale:
    - 1000 IOPS with TCP: 1000 × 100 μs = 100 ms of CPU time
    - 1000 IOPS with RDMA: nearly zero CPU (NIC handles everything)
    - CPU freed for: checksums, compression, dedup, metadata

Replication:
  Primary logs write → RDMA Write data to replica → 
  RDMA Write with Immediate (notifies replica) →
  Replica applies to its log → sends ACK via RDMA Send
  
  Total replication latency: ~10-15 μs (vs ~200 μs TCP)

Example systems using RDMA:
  - Azure Storage (RoCE v2)
  - Google Snap (custom RDMA)
  - Ceph (RDMA messenger)
  - DAOS (Intel's distributed storage — RDMA-native)
  - SPDK (uses RDMA for NVMe-oF)
  - Dell PowerStore / PowerScale (likely RDMA for inter-node)
```

---

## 7. Challenges & Tradeoffs

### Memory Pinning
```
Problem: RDMA requires registered memory to be pinned (not swappable)
  - Large registrations = large non-swappable memory footprint
  - Registration is slow (milliseconds) — can't do it per-request

Solutions:
  - Pre-register large buffers at startup, use as pool
  - On-Demand Paging (ODP): kernel handles page faults
    (available on Mellanox/NVIDIA NICs, adds ~1 μs on first access)
  - Memory registration cache: reuse registrations
```

### Connection Scalability
```
Problem: RC QPs are connected (like TCP connections)
  N nodes, each connected to all others = N × (N-1) QPs
  Each QP consumes NIC memory (~4 KB on-chip per QP)
  
  1000 nodes → ~1M QPs → NIC runs out of on-chip memory
  → QP state spills to host memory → performance degrades

Solutions:
  - Shared Receive Queues (SRQ): multiple QPs share receive buffers
  - Dynamic Connected (DC) transport: one QP connects to many (Mellanox)
  - Use UD (unreliable datagram) for scalability, add reliability in software
  - Connection pooling: fewer connections, multiplex traffic
```

### Lossless Network Requirement
```
RoCE v2 requires lossless Ethernet:
  - PFC (Priority Flow Control): pause frames prevent buffer overflow
  - ECN (Explicit Congestion Notification): mark packets instead of drop
  
Without lossless network:
  - Packet loss → go-back-N retransmission → severe performance drop
  - RoCE performance degrades to worse-than-TCP
  
iWARP doesn't need lossless (TCP handles retransmission)
InfiniBand has built-in flow control (lossless by design)

This is why RDMA deployment requires network configuration:
  DCB (Data Center Bridging): PFC + ETS + DCBX
  Careful traffic engineering to avoid PFC storms
```

### Programming Complexity
```
RDMA programming is significantly harder than sockets:
  - Must manage memory registration explicitly
  - Must manage QP state machine (RESET → INIT → RTR → RTS)
  - Must handle completions asynchronously
  - One-sided operations: remote side doesn't know data arrived
  - Debugging: can't tcpdump RDMA traffic easily
  - Error handling: NIC errors are cryptic

Libraries that simplify RDMA:
  - libfabric (OFI): abstraction layer over RDMA and TCP
  - UCX: Unified Communication X (used by MPI, Spark)
  - eRPC: efficient RPC over RDMA (Carnegie Mellon)
  - SPDK env: abstracts RDMA for storage applications
```

---

## 8. RDMA vs DPDK vs SPDK

```
RDMA:
  - NIC moves data between remote memories
  - Kernel bypass is automatic (NIC handles transport)
  - Requires RDMA-capable NICs
  - Best for: inter-node communication in clusters

DPDK (Data Plane Development Kit):
  - Kernel bypass for packet processing
  - Application sends/receives raw packets (you implement the protocol)
  - Works with any NIC (not just RDMA)
  - Best for: network function virtualization, firewalls, load balancers

SPDK (Storage Performance Development Kit):
  - Kernel bypass for storage (NVMe)
  - Application talks directly to NVMe SSD (no kernel block layer)
  - Uses polling (no interrupts) for lowest latency
  - Integrates with RDMA for NVMe over Fabrics
  - Best for: storage applications needing microsecond latency

Dell JD mentions all three: RDMA, SPDK, DPDK
```

---

## 9. Interview Questions

**Q: "What is RDMA and why is it important for storage systems?"**  
A: "RDMA lets one machine read or write another's memory without involving the remote CPU or kernel. For storage, this means data replication adds only ~5 μs latency instead of ~100 μs with TCP, and the remote node's CPU is free for compute tasks like dedup and compression. At scale with thousands of IOPS, this CPU saving is massive. It's deployed using InfiniBand in HPC or RoCE v2 in data center Ethernet."

**Q: "Explain one-sided vs two-sided RDMA operations."**  
A: "Two-sided (Send/Receive): both sides participate — receiver pre-posts a buffer, sender sends data to it. Similar to normal messaging but kernel-bypass. One-sided (RDMA Read/Write): only the initiator is involved — the remote NIC handles memory access without notifying the remote CPU. One-sided is used for bulk data transfer and remote key-value lookups because it has zero remote CPU cost."

**Q: "What challenges does RDMA introduce?"**  
A: "Three main challenges: (1) Memory must be registered and pinned — can't be swapped, registration is expensive; (2) RoCE v2 requires a lossless Ethernet fabric (PFC/ECN) — lossy networks destroy RDMA performance; (3) Programming complexity — must manage QP state machines, memory regions, and completions explicitly. Also, connected QPs don't scale well past thousands of nodes."

**Q: "You don't have RDMA experience. How would you get up to speed?"**  
A: "I'd approach it methodically: first understand the verbs API and QP state machine conceptually (which I've done), then set up a soft-RoCE (rxe) virtual RDMA device on Linux for local experimentation, then write a simple RDMA ping-pong with ibv_post_send/ibv_post_recv, then graduate to one-sided RDMA Write. My threading and memory management experience (14+ years with mutexes, shared memory, RAII) transfers directly — the patterns of buffer management, completion handling, and resource lifecycle are similar."

---

## 10. Hands-On: Soft-RoCE Setup (No Special Hardware)

```bash
# Linux provides software RDMA (rxe) for testing
# Works on any Linux machine with kernel 4.9+

# Load the module
sudo modprobe rdma_rxe

# Create soft-RoCE device on eth0
sudo rdma link add rxe0 type rxe netdev eth0

# Verify
rdma link
ibv_devices
ibv_devinfo rxe0

# Run RDMA perftest tools
sudo apt install perftest

# Terminal 1 (server):
ib_write_bw -d rxe0

# Terminal 2 (client):
ib_write_bw -d rxe0 localhost

# This gives you RDMA Write bandwidth numbers (over software emulation)
# Real hardware: 100x faster, but the API is identical
```

---

## Quick Reference Card

```
RDMA = NIC reads/writes remote memory directly (zero-copy, kernel-bypass)

Transports: InfiniBand (dedicated), RoCE v2 (Ethernet), iWARP (TCP)

Key abstractions:
  QP (Queue Pair):     send + receive queues (like a socket)
  CQ (Completion Queue): "operation done" notifications
  MR (Memory Region):  pinned, NIC-accessible memory buffer
  PD (Protection Domain): access control grouping

Operations:
  Send/Receive (two-sided):  both sides participate
  RDMA Write (one-sided):    write to remote memory, no remote CPU
  RDMA Read (one-sided):     read from remote memory, no remote CPU
  Atomic (CAS, Fetch-Add):   remote atomic operations

Performance: 1-5 μs latency, 100-400 Gbps, zero CPU on remote

Challenges: memory pinning, lossless network (PFC/ECN), QP scalability
Used by: Azure Storage, NVMe-oF, Ceph, DAOS, HPC/AI training
```
