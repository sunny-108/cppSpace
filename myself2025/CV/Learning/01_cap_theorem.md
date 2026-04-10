# CAP Theorem — Deep Dive

**Study Time: ~1 hour**  
**Prerequisites:** Basic understanding of distributed systems, client-server architecture

---

## 1. What Is the CAP Theorem?

Proposed by Eric Brewer in 2000, formally proven by Gilbert and Lynch in 2002.

**In any distributed data store, you can only guarantee 2 of the following 3 properties simultaneously:**

| Property | Definition |
|----------|-----------|
| **Consistency (C)** | Every read receives the most recent write or an error. All nodes see the same data at the same time. |
| **Availability (A)** | Every request receives a non-error response — without guarantee that it contains the most recent write. |
| **Partition Tolerance (P)** | The system continues to operate despite arbitrary message loss or failure of part of the network. |

---

## 2. Why You Can't Have All Three

Consider a simple 2-node system replicating data:

```
   Node A ←——replication——→ Node B
     ↑                        ↑
   Client 1                Client 2
```

**Normal operation:** Client 1 writes `x=5` to Node A. Node A replicates to Node B. Client 2 reads from Node B, gets `x=5`. All three properties hold.

**During a network partition** (Node A and Node B can't communicate):

```
   Node A    ✕✕✕✕✕✕✕✕    Node B
     ↑       (partition)      ↑
   Client 1               Client 2
```

Client 1 writes `x=5` to Node A. Now you have two choices:

### Choice 1: Consistency + Partition Tolerance (CP)
- Node A refuses to accept the write (or blocks until partition heals)
- Reasoning: "I can't guarantee Node B will get this, so I'd rather error than serve stale data"
- Client 1 gets an error → **Availability is sacrificed**
- Examples: ZooKeeper, etcd, HBase, MongoDB (with majority write concern)

### Choice 2: Availability + Partition Tolerance (AP)
- Both nodes keep accepting reads and writes independently
- Node A has `x=5`, Node B still has `x=old_value`
- Client 2 reads stale data → **Consistency is sacrificed**
- When partition heals, you need conflict resolution
- Examples: Cassandra, DynamoDB, CouchDB, Riak

### Why Not CA?
- Partition Tolerance isn't optional — networks **always** partition eventually  
- A "CA" system is just a single-node system or one that goes completely offline during partitions
- In practice, you're always choosing between CP and AP

---

## 3. The Spectrum — Not Binary

Real systems aren't purely CP or AP. They tune the tradeoff:

### Tunable Consistency (Cassandra/DynamoDB style)
```
N = number of replicas = 3
W = write quorum (writes ack'd by)
R = read quorum (reads queried from)

Strong consistency:    W + R > N  (e.g., W=2, R=2)
Eventual consistency:  W=1, R=1  (fastest, weakest)

Write-heavy optimized: W=1, R=3  (fast writes, slow reads)
Read-heavy optimized:  W=3, R=1  (slow writes, fast reads)
```

### Linearizability vs Sequential Consistency vs Eventual Consistency

```
Linearizability (strongest):
  - Operations appear to execute atomically at some point between 
    invocation and response
  - "It looks like there's only one copy of the data"
  - Example: etcd, ZooKeeper

Sequential Consistency:
  - Operations from each client appear in order
  - But there's no real-time ordering guarantee between clients
  - Weaker than linearizability

Causal Consistency:
  - If operation A causally precedes B, everyone sees A before B
  - Concurrent operations can be seen in different orders
  - Example: COPS

Eventual Consistency (weakest):
  - If no new writes, all replicas eventually converge
  - No ordering guarantees during convergence
  - Example: DNS, Cassandra with W=1/R=1
```

---

## 4. PACELC — The Extended CAP

Daniel Abadi's extension (2012): CAP only describes behavior **during partitions**. What about normal operation?

```
PACELC: 
  IF Partition → choose Availability or Consistency
  ELSE (normal) → choose Latency or Consistency
```

| System | Partition (PA/PC) | Else (EL/EC) |
|--------|-------------------|--------------|
| Cassandra | PA | EL |
| DynamoDB | PA | EL |
| MongoDB | PC | EC |
| ZooKeeper | PC | EC |
| PNUTS (Yahoo) | PA | EC |

**Why this matters for interviews:** Saying "PACELC" shows depth beyond the basic CAP discussion.

---

## 5. Real-World Examples

### etcd / Raft (CP system)
```
- Uses Raft consensus for all writes
- Write must be committed by majority of nodes
- During partition: minority partition CANNOT accept writes
- Leader in majority partition continues serving
- Strong consistency guaranteed
- Latency cost: every write needs network round-trips to quorum
```

### Cassandra (AP system)
```
- Uses consistent hashing for data placement
- Configurable W and R for tunable consistency
- During partition: all nodes keep accepting writes
- Conflict resolution: Last-Writer-Wins using wall-clock timestamps
- Risk: clock skew can cause data loss
- Benefit: always available, very low latency
```

### Amazon DynamoDB
```
- AP system with tunable consistency
- Default: eventually consistent reads (faster, cheaper)
- Optional: strongly consistent reads (costs 2x, higher latency)
- Uses vector clocks for conflict detection
- Application resolves conflicts on read (shopping cart use case)
```

---

## 6. How StoreOnce Relates (Your Interview Bridge)

Frame your existing experience in CAP terms:

```
"StoreOnce's backup/restore architecture makes explicit CAP tradeoffs:

- Backup writes prioritize Availability — a backup job should never 
  fail because a metadata node is temporarily unreachable

- Catalog metadata operations prioritize Consistency — we need 
  accurate dedup references to avoid data corruption

- Replication between StoreOnce appliances is eventually consistent 
  with conflict detection — the Dual Authorization framework I built 
  adds a consistency layer on top for destructive operations

- The parametric data collection I implemented is AP — we collect 
  metrics from all nodes independently and aggregate centrally, 
  accepting temporary inconsistency for availability"
```

---

## 7. Common Interview Questions

**Q: "How would you design a distributed KV store? What CAP tradeoffs would you make?"**  
A: Start with the use case. Read-heavy? Write-heavy? Can we tolerate stale reads?
- For a metadata store → CP with Raft (etcd-like)
- For a cache layer → AP with TTL-based expiry
- For a user-facing store → Tunable, default eventual, optional strong reads

**Q: "Your storage cluster has a network partition. Node A has the latest data. Client connects to Node B. What happens?"**  
A: Depends on our consistency model:
- If CP: Node B returns error or blocks
- If AP: Node B returns stale data
- If tunable: Depends on R — if R > N/2, the read will fail (can't reach quorum); if R=1, returns stale data

**Q: "Is CAP theorem still relevant?"**  
A: Yes, but it's an oversimplification. PACELC is more useful because most systems operate without partitions most of the time. The real engineering is in the EL/EC tradeoff — how much latency are you willing to pay for consistency during normal operation?

---

## 8. Key Papers & Resources

| Resource | What You Get | Time |
|----------|-------------|------|
| Brewer's original keynote (2000) | 12-page overview, the original argument | 20 min |
| Gilbert & Lynch proof (2002) | Formal proof, rigorous definitions | 30 min (skim) |
| Kleppmann "Please stop calling databases CP or AP" | Why CAP is oversimplified, real nuances | 15 min |
| Abadi "Consistency Tradeoffs in Modern Distributed Database System Design" | PACELC theorem | 20 min |

---

## Quick Reference Card

```
CAP = Consistency + Availability + Partition Tolerance (pick 2)
Partition Tolerance is mandatory → real choice is CP vs AP

CP: Block/error during partition. Strong consistency. (etcd, ZooKeeper)
AP: Always respond. Risk stale data. (Cassandra, DynamoDB)

Tunable: W + R > N = strong consistency
PACELC: During Partition → A or C; Else → Latency or Consistency

Linearizability > Sequential > Causal > Eventual (consistency spectrum)
```
