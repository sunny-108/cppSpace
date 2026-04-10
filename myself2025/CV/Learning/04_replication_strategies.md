# Replication Strategies — Deep Dive

**Study Time: ~1.5 hours**  
**Prerequisites:** CAP theorem, basic distributed systems concepts

---

## 1. Why Replication?

| Goal | How Replication Helps |
|------|----------------------|
| **Fault tolerance** | Data survives node failures |
| **Read scalability** | Spread reads across replicas |
| **Latency** | Serve from geographically closer replica |
| **Durability** | Data on disk in multiple locations |

**The fundamental tradeoff:** More replicas = more durability & read throughput, but more write latency & complexity.

---

## 2. Single-Leader (Primary-Backup) Replication

### How It Works
```
All writes → Primary → replicates to Followers → ack

Client ──write──→ [Primary]
                     │
                     ├──replicate──→ [Follower 1]
                     ├──replicate──→ [Follower 2]
                     └──replicate──→ [Follower 3]

Reads can go to Primary (strong) or any Follower (eventual)
```

### Synchronous vs Asynchronous

**Synchronous Replication:**
```
Client → Primary: write x=5
Primary writes to local log
Primary → Follower1: replicate x=5
Primary → Follower2: replicate x=5
Primary waits for ALL followers to ack
Primary → Client: "OK"

Timeline:
  Client ──→ Primary ──→ F1 (ack) ──→ F2 (ack) ──→ Client (OK)
  Latency = max(F1_latency, F2_latency)

Pros: Strong consistency, zero data loss
Cons: Latency = slowest follower; one slow/dead follower blocks all writes
```

**Asynchronous Replication:**
```
Client → Primary: write x=5
Primary writes to local log
Primary → Client: "OK"  ← immediately returns
Primary → Follower1: replicate x=5  (background)
Primary → Follower2: replicate x=5  (background)

Timeline:
  Client ──→ Primary ──→ Client (OK)
                    └──→ F1, F2 (eventually)
  Latency = primary write only

Pros: Low latency, tolerates slow followers
Cons: Data loss if primary crashes before replication completes
```

**Semi-Synchronous (Practical compromise):**
```
Wait for at least 1 follower to ack before responding to client.
Other followers replicate asynchronously.

Client → Primary → F1 (ack) → Client (OK)
                 → F2 (async, background)

MySQL "semi-sync replication" does exactly this.
Guarantees: at least 2 copies before ack (primary + 1 follower)
```

### Replication Log Mechanics

```
Write-Ahead Log (WAL) shipping:
  Primary writes to WAL first → ships WAL entries to followers
  Followers replay WAL entries in order
  
  Pros: Simple, proven (PostgreSQL, MySQL)
  Cons: Tied to storage engine format; version coupling

Logical (Row-Based) Replication:
  Ship the logical change: "INSERT row (id=5, name='hello')"
  Not tied to storage engine internals
  
  Pros: Cross-version compatible, enables CDC (Change Data Capture)
  Cons: Larger messages for bulk operations

Statement-Based Replication:
  Ship the SQL statement: "UPDATE users SET age=30 WHERE id=5"
  
  Pros: Compact
  Cons: Non-deterministic functions (NOW(), RAND()), trigger issues
  Largely deprecated in favor of row-based.
```

### Handling Primary Failure (Failover)

```
1. Detect failure:
   Followers notice heartbeat timeout (typically 10-30 seconds)

2. Elect new primary:
   Follower with most up-to-date log wins
   (Raft handles this elegantly)

3. Reconfigure:
   Clients redirect writes to new primary
   Old primary (if it recovers) becomes follower

Dangers:
  - Split brain: two nodes think they're primary
    Solution: fencing (STONITH), lease-based leadership
  
  - Data loss: async follower promoted → missing recent writes
    Solution: semi-sync, or accept some data loss
  
  - Write conflicts: old primary comes back with writes not in new primary
    Solution: discard old primary's un-replicated writes (scary but correct)
```

---

## 3. Multi-Leader Replication

### When You Need It
- Multi-datacenter deployment (one leader per DC)
- Offline-capable clients (each client is a "leader")
- Collaborative editing (Google Docs)

```
DC 1                      DC 2
[Leader A] ←─async──→ [Leader B]
    ↑                      ↑
  writes                 writes
  (local)               (local)

Both accept writes independently
Replicate to each other asynchronously
```

### The Conflict Problem

```
User 1 (DC 1): UPDATE title = "A"  at time T1
User 2 (DC 2): UPDATE title = "B"  at time T1

Both succeed locally → replicate to each other → CONFLICT

DC1 has: "A" then gets "B"
DC2 has: "B" then gets "A"
```

### Conflict Resolution Strategies

**1. Last-Writer-Wins (LWW)**
```
Attach a timestamp to each write. Highest timestamp wins.

Write 1: {value: "A", timestamp: 1001}
Write 2: {value: "B", timestamp: 1002}

Conflict resolution: B wins (higher timestamp)

Problem: Clock skew — clocks across DCs are never perfectly synced
  DC1 clock: 1001 (real time: 12:00:00.000)
  DC2 clock: 1002 (real time: 11:59:59.500)
  DC2 "wins" even though it happened 500ms EARLIER
  
Used by: Cassandra, DynamoDB
Risk: Silent data loss — "A" is discarded without notice
```

**2. Vector Clocks**
```
Each node maintains a vector of logical timestamps:
  [DC1_count, DC2_count, DC3_count]

Write 1 at DC1: version = [1, 0, 0]
Write 2 at DC2: version = [0, 1, 0]

Neither dominates the other → CONFLICT DETECTED
  [1,0,0] vs [0,1,0] — incomparable!

Application must resolve:
  - Show both versions to user (Amazon shopping cart)
  - Merge programmatically (union of sets)
  - Prompt user to pick

If Write 3 at DC1 happens after seeing Write 2:
  version = [2, 1, 0] — this dominates [0,1,0] and [1,0,0]
  → no conflict, Write 3 is the clear winner

Used by: Amazon Dynamo (original), Riak
Pros: No data loss, precise conflict detection
Cons: Application must handle conflicts, vector grows with nodes
```

**3. CRDTs (Conflict-Free Replicated Data Types)**
```
Data structures designed so concurrent updates always converge
without conflicts. Mathematically guaranteed.

G-Counter (Grow-only counter):
  Each node has its own counter
  Node A: {A: 5, B: 0, C: 0}
  Node B: {A: 0, B: 3, C: 0}
  
  Merge: take max of each element
  Result: {A: 5, B: 3, C: 0} → total = 8
  
  Works because max is commutative, associative, idempotent

PN-Counter (Positive-Negative counter):
  Two G-Counters: one for increments, one for decrements
  Value = sum(increments) - sum(decrements)

LWW-Register:
  Attach timestamp, highest wins (like LWW but as a formal CRDT)

OR-Set (Observed-Remove Set):
  Add and remove operations with unique tags
  Concurrent add + remove → add wins (add-wins semantics)

Used by: Redis (CRDT mode), Riak, Soundcloud, League of Legends
Pros: No conflict resolution logic needed, automatic convergence
Cons: Limited data types, space overhead for metadata
```

**4. Operational Transformation (OT)**
```
Used by: Google Docs, Apache Wave

Transforms concurrent operations against each other:
  User 1: Insert "A" at position 3
  User 2: Delete character at position 1

  User 1 sees User 2's delete → transforms own insert:
    position 3 → position 2 (shifted left by delete before it)

Complex to implement correctly — Google Docs is one of few successes.
```

---

## 4. Leaderless (Dynamo-Style) Replication

### How It Works
```
No leader — client writes to multiple replicas directly.

Client write:
  → Node A: write x=5 (ack)
  → Node B: write x=5 (ack)  
  → Node C: write x=5 (timeout — node C is slow/down)
  
  W=2 acks received → write succeeds

Client read:
  → Node A: read x → returns 5 (ack)
  → Node B: read x → returns 5 (ack)
  → Node C: read x → returns old_value (stale)
  
  R=2 responses received → pick most recent → return 5
```

### Quorum Math

```
N = total replicas
W = write quorum (minimum acks for write success)
R = read quorum (minimum responses for read)

Strong consistency guarantee: W + R > N

Example: N=3
  W=2, R=2: 2+2=4 > 3 ✓ (strong consistency)
  W=1, R=3: 1+3=4 > 3 ✓ (fast writes, slow reads)
  W=3, R=1: 3+1=4 > 3 ✓ (slow writes, fast reads)
  W=1, R=1: 1+1=2 < 3 ✗ (eventual consistency — might read stale)

Why it works:
  Write touches W nodes, read touches R nodes
  W + R > N → at least one node was in both sets
  That node has the latest value → read returns it
```

### Anti-Entropy & Read Repair

```
Read Repair:
  Client reads from R nodes, gets different versions
  After returning the latest to client, pushes the latest to stale nodes
  
  Pros: Passive, happens naturally during reads
  Cons: Rarely-read data stays stale indefinitely

Anti-Entropy (Background repair):
  Background process compares data across replicas
  Uses Merkle trees to efficiently find differences
  
  Merkle Tree:
    Hash tree of data ranges — can identify differing ranges in O(log N)
    Compare root hash → if different, compare children → drill down
    
    Node A tree:    [hash: abc123]
                    /            \
            [hash: def]     [hash: 456]
            /      \         /      \
        [data1] [data2]  [data3] [data4]
    
    Compare with Node B: root hash differs → drill down
    Right subtree hash differs → data4 is different → sync it

Hinted Handoff:
  When a node is down, write goes to another node with a "hint"
  Hint: "this data belongs to Node C, deliver when it's back"
  When Node C recovers, hints are forwarded to it
  
  Sloppy quorum: hints count toward quorum (faster but weaker)
  Strict quorum: hints don't count (slower but stronger)
```

---

## 5. Comparison Matrix

| Aspect | Single-Leader | Multi-Leader | Leaderless |
|--------|--------------|-------------|------------|
| Write topology | All writes → 1 node | Writes → any leader | Writes → any node |
| Read scaling | Followers serve reads | Both leaders serve | All nodes serve |
| Write latency | Network + disk | Local disk (async repl) | W nodes * network |
| Consistency | Strong (sync) / Eventual (async) | Eventual (conflicts) | Tunable (W+R>N) |
| Failover | Need election/promotion | Other leaders continue | No failover needed |
| Conflict handling | None (single writer) | Must resolve conflicts | Version vectors |
| Complexity | Low | High (conflicts) | Medium (quorums) |
| Use case | Most databases | Multi-DC, offline | KV stores, caches |
| Examples | PostgreSQL, MySQL, MongoDB | CouchDB, Tungsten | Cassandra, DynamoDB, Riak |

---

## 6. Your Interview Bridge (StoreOnce Context)

```
"In StoreOnce, we use different replication strategies for different components:

- Backup data replication between appliances uses asynchronous single-leader 
  replication — the source appliance is the 'primary' and replication targets 
  receive data asynchronously. This is an AP design — we prioritize backup 
  availability over replication consistency.

- The Dual Authorization framework I designed adds a consistency layer on top: 
  destructive operations (delete NAS share, delete Catalyst item) require 
  authorization from a second administrator, which means the DA state must be 
  consistent across the cluster before the operation proceeds — this is a CP 
  pattern layered on an AP system.

- Parametric data (disk latency, utilization metrics) uses leaderless-style 
  collection — every node independently collects and reports metrics. The 
  aggregator on the central node does read-repair-style reconciliation."
```

---

## 7. Interview Questions

**Q: "You're designing a distributed storage system. How do you choose a replication strategy?"**  
A: Depends on the use case:
- User-facing metadata (accounts, permissions) → Single-leader, synchronous — can't lose this
- Bulk data (backups, objects) → Single-leader, async — throughput matters more
- Multi-DC active-active → Multi-leader with CRDTs for conflict-free types
- Read-heavy KV cache → Leaderless with tunable quorum

**Q: "How do you detect that a replica is stale?"**  
A: Multiple mechanisms:
1. Version numbers / vector clocks on each key
2. Read repair during client reads
3. Merkle tree comparison in background anti-entropy
4. Log position tracking (follower's applied log position vs leader's)

**Q: "What happens to in-flight writes when a primary fails?"**  
A: In sync replication — clients get an error, retry against new primary. In async replication — those writes may be lost. This is the fundamental tradeoff: async gives you lower latency at the cost of potential data loss during failover (RPO > 0).

---

## Quick Reference Card

```
Single-Leader: All writes → primary → replicate to followers
  Sync: wait for all acks (strong, slow)
  Async: return immediately (fast, risk data loss)
  Semi-sync: wait for 1 follower ack (practical compromise)

Multi-Leader: Multiple writers, async cross-replication
  Conflicts: LWW (lossy), Vector Clocks (detect), CRDTs (auto-resolve), OT (transforms)

Leaderless: Write to W nodes, read from R nodes
  W + R > N → strong consistency
  Anti-entropy: Merkle trees for background sync
  Hinted handoff: temporary storage during node outages

Failover: Detect → Elect new leader → Reconfigure clients
  Dangers: split brain, data loss, write conflicts
```
