# Raft Consensus Algorithm — Deep Dive

**Study Time: ~4 hours**  
**Prerequisites:** CAP theorem basics, client-server architecture, log-based systems

---

## 1. What Problem Does Raft Solve?

**Consensus:** Getting N machines to agree on a sequence of values (operations), even when some machines crash.

**Why it's hard:**
- Machines can crash at any time
- Network messages can be delayed, reordered, or lost
- No global clock — can't tell who was "first"
- Must guarantee: once a value is agreed upon (committed), it's permanent

**Real-world uses:**
- etcd (Kubernetes' brain) — stores cluster configuration
- CockroachDB — distributed SQL
- TiKV — distributed KV for TiDB
- Consul — service discovery
- Your distributed KV store project

---

## 2. Raft Overview

Raft decomposes consensus into 3 sub-problems:

| Sub-problem | Mechanism |
|-------------|-----------|
| **Leader Election** | One node is leader at a time; handles all client requests |
| **Log Replication** | Leader replicates log entries to followers |
| **Safety** | Rules that guarantee correctness |

### Three Node States

```
                            times out,
                           starts election
  ┌──────────┐            ┌───────────┐
  │ Follower │──────────→ │ Candidate │
  └──────────┘            └───────────┘
       ↑                     │    ↑
       │  discovers leader   │    │ new election
       │  or new term        │    │ (split vote)
       │                     ↓    │
       │                 ┌──────────┐
       └──────────────── │  Leader  │
                         └──────────┘
```

### Terms (Logical Clock)

```
Term 1          Term 2              Term 3
|── Leader A ──|── Election ──|── Leader C ──────→
               |   (failed)   |
               |   Leader B   |
               |   (crashed)  |

- Each term has at most one leader
- Terms are monotonically increasing integers
- A node rejects messages with older terms
- If a node sees a higher term → immediately becomes Follower
```

---

## 3. Leader Election — Step by Step

### The Happy Path

```
Time →

Node A (Follower): [---waiting---timeout!---]→ Candidate (term 2)
  │ Sends RequestVote(term=2, lastLog=...) to B and C
  │
Node B (Follower): Receives RequestVote → grants vote → responds YES
Node C (Follower): Receives RequestVote → grants vote → responds YES
  │
Node A: Got 2 votes (B+C) + own vote = 3/3 → becomes Leader (term 2)
  │ Immediately sends heartbeats (empty AppendEntries)
  │
Node B: Receives heartbeat → resets election timer, stays Follower
Node C: Receives heartbeat → resets election timer, stays Follower
```

### Election Timeout
```
Each Follower has a random election timeout: 150ms - 300ms

Why random? To avoid split votes:
  If all timeouts were identical:
    A, B, C all time out simultaneously
    All become Candidates, all vote for themselves
    Nobody gets majority → new election → repeat forever
  
  With random timeouts:
    Typically one node times out first
    It starts election, gets votes before others time out
```

### Split Vote Scenario
```
Node A timeout: 150ms → becomes Candidate (term 2), votes for self
Node B timeout: 155ms → becomes Candidate (term 2), votes for self
Node C timeout: 280ms → still Follower

Node A sends RequestVote to B, C
Node B sends RequestVote to A, C

Node C receives both → votes for A (first received), rejects B
Node A: votes = {A, C} = 2/3 → majority → Leader!
Node B: votes = {B} = 1/3 → loses, reverts to Follower

If no majority (true split):
  Election timeout elapses → new term → new election with new random timeouts
```

### RequestVote RPC

```
RequestVote(
  term:          candidate's term
  candidateId:   who's asking
  lastLogIndex:  index of candidate's last log entry
  lastLogTerm:   term of candidate's last log entry
)

Reply(
  term:        receiver's current term
  voteGranted: true/false
)

Voting rules:
  1. Only vote if candidate's term >= my term
  2. Only vote once per term (first-come-first-served)
  3. Only vote if candidate's log is at least as up-to-date as mine
     (prevents electing a node that's missing committed entries)
     
     "Up-to-date" comparison:
       if lastLogTerm differs → higher term wins
       if lastLogTerm same → longer log wins
```

---

## 4. Log Replication — Step by Step

### The Write Path

```
Client → Leader: SET x = 5

Step 1: Leader appends to its log (uncommitted)
  Leader log: [..., {index=7, term=3, cmd="SET x=5"}]

Step 2: Leader sends AppendEntries to all Followers
  AppendEntries(
    term:         3
    leaderId:     A
    prevLogIndex: 6
    prevLogTerm:  3
    entries:      [{index=7, term=3, cmd="SET x=5"}]
    leaderCommit: 6  (last committed index)
  )

Step 3: Followers append to their logs, respond OK
  Follower B: appends entry 7 → responds success
  Follower C: appends entry 7 → responds success

Step 4: Leader sees majority (2/3 including self) acknowledged
  Commits entry 7 → applies "SET x=5" to state machine
  Updates commitIndex = 7

Step 5: Next heartbeat tells Followers to commit
  leaderCommit=7 → Followers apply entry 7 to their state machines

Step 6: Leader responds to client: "OK"
```

### Log Structure

```
Index: | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
Term:  | 1 | 1 | 1 | 2 | 2 | 3 | 3 |
Cmd:   |x=1|y=2|z=3|x=4|y=5|z=6|x=5|
       |←── committed ──→|←uncommit→|
                     commitIndex=6
```

### Handling Log Inconsistencies

When a Follower is behind or has divergent entries:

```
Leader log:  | 1:1 | 2:1 | 3:2 | 4:3 | 5:3 | 6:3 |
Follower B:  | 1:1 | 2:1 | 3:2 | 4:3 | 5:3 |        ← just behind
Follower C:  | 1:1 | 2:1 | 3:2 | 4:2 | 5:2 |        ← divergent!

Leader sends AppendEntries to C with prevLogIndex=5, prevLogTerm=3
Follower C: "My entry 5 has term 2, not 3" → rejects

Leader decrements nextIndex for C, retries:
  prevLogIndex=4, prevLogTerm=3 → C rejects (term 2 ≠ 3)
  prevLogIndex=3, prevLogTerm=2 → C accepts!

Leader sends entries 4, 5, 6 to C
Follower C: deletes entries 4-5 (divergent), replaces with leader's 4-6

Result: C now matches Leader:
  | 1:1 | 2:1 | 3:2 | 4:3 | 5:3 | 6:3 |
```

**Key safety property:** The leader NEVER deletes or overwrites its own log. It only appends. Followers may have entries overwritten by the leader.

---

## 5. Safety Guarantees

### Election Restriction
A candidate can only win election if its log contains all committed entries:

```
Why? The voting rule: "vote only if candidate's log is at least as up-to-date"

If entry E is committed → majority of nodes have E
A candidate needs majority of votes
The intersection of these two majorities is non-empty
→ At least one voter has E
→ That voter won't vote for a candidate missing E
→ Winner must have E ✓
```

### Commit Rule
A leader only commits entries from its OWN term using majority count. Entries from previous terms are committed indirectly (when a current-term entry after them is committed).

```
Why? To prevent this scenario:

Term 1: S1 (leader) replicates entry A to S1, S2 (2/5 — not committed)
Term 2: S5 (leader) replicates entry B to S3, S4, S5 (committed)
Term 3: S1 (leader) — can it commit entry A now? 
  
  If S1 counts S1, S2, S3 as majority for entry A → commits it
  BUT entry B (committed in term 2) conflicts!
  
  Solution: S1 only commits using current-term entries
  It replicates a new entry C (term 3) → when C is committed,
  entry A (before C in log) is also committed implicitly
```

### Summary of Safety Properties

| Property | Guarantee |
|----------|-----------|
| Election Safety | At most one leader per term |
| Leader Append-Only | Leader never overwrites its log |
| Log Matching | If two logs have entry with same index+term, all preceding entries are identical |
| Leader Completeness | If entry is committed in term T, it's present in all leaders of terms > T |
| State Machine Safety | If a server applies a log entry at index i, no other server applies a different entry at i |

---

## 6. Heartbeats and Timeouts

```
Key timing parameters:

broadcastTime << electionTimeout << MTBF

broadcastTime: ~1-10ms (network round-trip)
electionTimeout: 150-300ms (random per node, per election)
MTBF: months (mean time between failures)

Heartbeat interval: typically broadcastTime × 5 = ~50ms
  Leader sends empty AppendEntries every 50ms
  Followers reset election timer on heartbeat receipt
  If no heartbeat for electionTimeout → start election
```

---

## 7. Cluster Membership Changes

Adding/removing nodes is tricky because you can't atomically switch all nodes:

### Single-Server Changes (Recommended — Simpler)
```
Add one node at a time:

1. New node joins as non-voting member, catches up on log
2. Leader proposes configuration change: C_old → C_new (includes new node)
3. Once committed, new node votes
4. Repeat for next node

Remove one node at a time:
1. Leader proposes C_old → C_new (excludes leaving node)
2. Once committed, leaving node shuts down
3. If leader is leaving → it steps down after committing
```

### Joint Consensus (Original Raft paper — Complex)
```
Two-phase approach:
  Phase 1: C_old,new → both old and new configurations must agree
  Phase 2: C_new → switch to new configuration only

This avoids the "two disjoint majorities" problem during transitions.
```

---

## 8. C++ Implementation Skeleton

```cpp
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

enum class NodeState { FOLLOWER, CANDIDATE, LEADER };

struct LogEntry {
    int term;
    int index;
    std::string command;  // e.g., "SET key value"
};

struct RequestVoteArgs {
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;
};

struct RequestVoteReply {
    int term;
    bool voteGranted;
};

struct AppendEntriesArgs {
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    std::vector<LogEntry> entries;  // empty for heartbeat
    int leaderCommit;
};

struct AppendEntriesReply {
    int term;
    bool success;
    int matchIndex;  // optimization: tells leader how far follower matched
};

class RaftNode {
public:
    RaftNode(int id, std::vector<int> peers)
        : id_(id), peers_(std::move(peers)), state_(NodeState::FOLLOWER),
          currentTerm_(0), votedFor_(-1), commitIndex_(0), lastApplied_(0) {
        resetElectionTimer();
    }

    // ---- RequestVote RPC Handler ----
    RequestVoteReply handleRequestVote(const RequestVoteArgs& args) {
        std::lock_guard<std::mutex> lock(mu_);
        RequestVoteReply reply{currentTerm_, false};

        // Rule 1: Reject if caller's term is stale
        if (args.term < currentTerm_) return reply;

        // If caller has higher term, step down
        if (args.term > currentTerm_) {
            stepDown(args.term);
        }

        // Rule 2: Only vote once per term
        if (votedFor_ != -1 && votedFor_ != args.candidateId) return reply;

        // Rule 3: Candidate's log must be at least as up-to-date
        int myLastTerm = log_.empty() ? 0 : log_.back().term;
        int myLastIndex = static_cast<int>(log_.size());

        bool logOk = (args.lastLogTerm > myLastTerm) ||
                     (args.lastLogTerm == myLastTerm && args.lastLogIndex >= myLastIndex);

        if (logOk) {
            votedFor_ = args.candidateId;
            reply.voteGranted = true;
            resetElectionTimer();
        }

        return reply;
    }

    // ---- AppendEntries RPC Handler ----
    AppendEntriesReply handleAppendEntries(const AppendEntriesArgs& args) {
        std::lock_guard<std::mutex> lock(mu_);
        AppendEntriesReply reply{currentTerm_, false, 0};

        // Reject if caller's term is stale
        if (args.term < currentTerm_) return reply;

        // Valid leader — step down and reset timer
        if (args.term > currentTerm_) {
            stepDown(args.term);
        }
        state_ = NodeState::FOLLOWER;
        resetElectionTimer();

        // Check log consistency at prevLogIndex
        if (args.prevLogIndex > 0) {
            if (static_cast<int>(log_.size()) < args.prevLogIndex) {
                return reply;  // log too short
            }
            if (log_[args.prevLogIndex - 1].term != args.prevLogTerm) {
                // Delete conflicting entry and all after it
                log_.resize(args.prevLogIndex - 1);
                return reply;
            }
        }

        // Append new entries (skip duplicates)
        for (size_t i = 0; i < args.entries.size(); ++i) {
            int idx = args.prevLogIndex + static_cast<int>(i);
            if (idx < static_cast<int>(log_.size())) {
                if (log_[idx].term != args.entries[i].term) {
                    log_.resize(idx);  // delete conflicting
                    log_.push_back(args.entries[i]);
                }
                // else: already have this entry, skip
            } else {
                log_.push_back(args.entries[i]);
            }
        }

        // Update commit index
        if (args.leaderCommit > commitIndex_) {
            commitIndex_ = std::min(args.leaderCommit, static_cast<int>(log_.size()));
            applyCommitted();
        }

        reply.success = true;
        reply.matchIndex = static_cast<int>(log_.size());
        return reply;
    }

    // ---- Leader: Start a new client command ----
    bool propose(const std::string& command) {
        std::lock_guard<std::mutex> lock(mu_);
        if (state_ != NodeState::LEADER) return false;

        LogEntry entry;
        entry.term = currentTerm_;
        entry.index = static_cast<int>(log_.size()) + 1;
        entry.command = command;

        log_.push_back(entry);
        // Trigger AppendEntries to all followers (in real impl: signal background thread)
        return true;
    }

private:
    void stepDown(int newTerm) {
        currentTerm_ = newTerm;
        state_ = NodeState::FOLLOWER;
        votedFor_ = -1;
    }

    void resetElectionTimer() {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(150, 300);
        electionTimeoutMs_ = dist(rng);
        lastHeartbeat_ = std::chrono::steady_clock::now();
    }

    void applyCommitted() {
        while (lastApplied_ < commitIndex_) {
            lastApplied_++;
            // Apply log_[lastApplied_ - 1].command to state machine
            // e.g., update key-value store
        }
    }

    // ---- Persistent State (must survive crashes) ----
    int currentTerm_;
    int votedFor_;
    std::vector<LogEntry> log_;

    // ---- Volatile State ----
    int commitIndex_;
    int lastApplied_;
    NodeState state_;
    int id_;
    std::vector<int> peers_;
    std::mutex mu_;

    // ---- Timer ----
    int electionTimeoutMs_;
    std::chrono::steady_clock::time_point lastHeartbeat_;
};
```

---

## 9. Raft vs Paxos

| Aspect | Raft | Paxos |
|--------|------|-------|
| Understandability | Designed to be simple | Notoriously difficult |
| Leader | Strong leader, all writes go through leader | Can have multiple proposers |
| Phases | 2 phases (election + replication) | 2 phases per value (prepare + accept) |
| Log | Built-in ordered log | Must be layered on top (Multi-Paxos) |
| Real systems | etcd, Consul, TiKV, CockroachDB | Google Chubby, Google Spanner |
| Membership changes | Built into protocol | Separate problem |
| Performance | Efficient (one round-trip for most writes) | Similar with Multi-Paxos optimization |
| Correctness proofs | Simpler, more accessible | Complex, multiple variants |

**For interviews:** "I'd implement Raft for our distributed KV store because it provides the same safety guarantees as Paxos with a more understandable and implementable design. The strong-leader approach simplifies log replication, and the election restriction guarantees that no committed entries are lost."

---

## 10. Interview Questions

**Q: "What happens if the leader crashes after replicating to only 1 follower?"**  
A: The entry is NOT committed (only 2/5 nodes). New election occurs. The new leader might or might not have the entry. If it doesn't → entry is lost (fine, it was never committed). If it does → entry will eventually be committed when the new leader replicates its log.

**Q: "Can two leaders exist at the same time?"**  
A: In different terms, briefly yes (old leader hasn't realized it's been replaced). But the old leader can't commit anything — it can't get majority acks because the majority has moved to the new term. When it contacts any node with a higher term, it immediately steps down.

**Q: "What's the minimum cluster size?"**  
A: 3 nodes (tolerates 1 failure). 5 nodes (tolerates 2) is common in production. Don't use even numbers — no benefit and risk of tied votes.

**Q: "How do you handle slow followers?"**  
A: Leader tracks `nextIndex` per follower. Slow followers get batched entries. You don't wait for all followers — only quorum. The slow follower catches up in the background.

**Q: "What about read consistency?"**  
A: By default, reads from leader are linearizable (leader confirms it's still leader first). For better read performance:
  - Read from any node with "lease-based reads" (leader holds a lease)
  - ReadIndex: leader confirms leadership then serves locally
  - Follower reads: forward to leader for confirmation, serve locally

---

## 11. Resources

| Resource | Time | Value |
|----------|------|-------|
| [The Raft Paper](https://raft.github.io/raft.pdf) — Sections 1-6 | 2 hours | Essential — read this first |
| [thesecretlivesofdata.com/raft](http://thesecretlivesofdata.com/raft/) | 30 min | Best visualization, play with all scenarios |
| [Raft Visualization (official)](https://raft.github.io/) | 30 min | Step-through simulation |
| Jon Gjengset's Raft implementation video (YouTube) | 2 hours | Practical implementation details |

---

## Quick Reference Card

```
Raft = Leader Election + Log Replication + Safety

States: Follower → Candidate → Leader
Terms: monotonically increasing logical clock, at most 1 leader per term

Election:
  Random timeout (150-300ms) → Candidate → RequestVote → majority → Leader
  Vote rules: higher term, at most once per term, log must be up-to-date

Replication:
  Client → Leader → append log → AppendEntries to followers →
  majority ack → committed → apply to state machine → respond to client

Safety:
  Election restriction: candidate log must be up-to-date to get votes
  Commit rule: only commit current-term entries by counting
  Leader never overwrites its own log

Quorum: ⌊N/2⌋ + 1  (3→2, 5→3, 7→4)
Fault tolerance: ⌊(N-1)/2⌋ failures (3→1, 5→2, 7→3)
```
