# Consistent Hashing — Deep Dive

**Study Time: ~1.5 hours**  
**Prerequisites:** Hash functions, basic data structures (maps, trees)

---

## 1. The Problem with Traditional Hashing

You have N servers and need to distribute keys across them:

```cpp
int server = hash(key) % N;
```

**Works fine until you add or remove a server:**

```
Before (N=3):
  hash("user:101") % 3 = 1  → Server 1
  hash("user:202") % 3 = 0  → Server 0
  hash("user:303") % 3 = 2  → Server 2

After adding a server (N=4):
  hash("user:101") % 4 = 2  → Server 2  ← MOVED
  hash("user:202") % 4 = 3  → Server 3  ← MOVED
  hash("user:303") % 4 = 1  → Server 1  ← MOVED
```

**Almost every key remaps.** For a cache with 1 billion keys, adding 1 server causes ~75% cache misses. Catastrophic.

---

## 2. The Hash Ring

**Core idea:** Place both servers and keys on a circular hash space (ring) from 0 to 2^32 - 1.

```
                    0
                    |
            Server C   Server A
                |         |
    2^32-1 ----+----+----+---- 2^32/4
                |         |
            Server B    (empty)
                |
                2^32/2

A key is assigned to the first server found going clockwise from the key's hash position.
```

### Example:
```
Ring: 0 ————————————————————————— 2^32
      |                              |
      ServerA(100)  ServerB(400)  ServerC(700)

key "user:1" → hash = 250  → walk clockwise → hits ServerB(400) ✓
key "user:2" → hash = 500  → walk clockwise → hits ServerC(700) ✓
key "user:3" → hash = 800  → walk clockwise → wraps → hits ServerA(100) ✓
```

### Adding a server — minimal disruption:
```
Add ServerD(550):

Before: key hash=500 → ServerC(700)
After:  key hash=500 → ServerD(550)  ← only keys between 400-550 remapped

Keys on other segments: UNTOUCHED
Fraction remapped: ~1/N (only keys in the new node's range)
```

### Removing a server:
```
Remove ServerB(400):

Before: keys 100-400 → ServerB
After:  keys 100-400 → ServerC(700)  ← next clockwise node inherits

Only ~1/N keys remapped.
```

---

## 3. Virtual Nodes (vnodes)

**Problem with basic consistent hashing:** Uneven distribution.

```
3 servers on a ring:
  ServerA at position 100
  ServerB at position 200
  ServerC at position 900

ServerC owns range 200→900 = 70% of keyspace!
ServerA owns range 900→100 = 20%
ServerB owns range 100→200 = 10%
```

**Solution: Virtual nodes.** Each physical server gets many positions on the ring.

```
ServerA → vnode_A1(100), vnode_A2(350), vnode_A3(600), vnode_A4(850)
ServerB → vnode_B1(200), vnode_B2(450), vnode_B3(700), vnode_B4(950)
ServerC → vnode_C1(150), vnode_C2(500), vnode_C3(750), vnode_C4(050)

Ring: 050(C) 100(A) 150(C) 200(B) 350(A) 450(B) 500(C) 600(A) 700(B) 750(C) 850(A) 950(B)

Now each server owns ~33% of the ring (well-balanced)
```

**Typical vnode count:** 100-256 per physical node (Cassandra default: 256)

**Benefits of vnodes:**
1. Even distribution regardless of node count
2. When a node goes down, its load spreads across ALL remaining nodes (not just one)
3. New nodes can steal small ranges from many nodes (faster rebalancing)
4. Heterogeneous hardware: give more vnodes to beefier servers

---

## 4. C++ Implementation

```cpp
#include <map>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <iostream>
#include <sstream>

class ConsistentHashRing {
public:
    explicit ConsistentHashRing(int vnodes_per_node = 150)
        : vnodes_per_node_(vnodes_per_node) {}

    // Add a physical node with its virtual nodes
    void addNode(const std::string& node) {
        for (int i = 0; i < vnodes_per_node_; ++i) {
            uint32_t hash = hashKey(node + "#" + std::to_string(i));
            ring_[hash] = node;
        }
    }

    // Remove a physical node and all its virtual nodes
    void removeNode(const std::string& node) {
        for (int i = 0; i < vnodes_per_node_; ++i) {
            uint32_t hash = hashKey(node + "#" + std::to_string(i));
            ring_.erase(hash);
        }
    }

    // Find the node responsible for a key
    std::string getNode(const std::string& key) const {
        if (ring_.empty()) return "";

        uint32_t hash = hashKey(key);

        // Find the first node at or after this hash position
        auto it = ring_.lower_bound(hash);

        // Wrap around if we've gone past the end
        if (it == ring_.end()) {
            it = ring_.begin();
        }

        return it->second;
    }

    // Get N nodes for replication (walk clockwise, skip same physical node)
    std::vector<std::string> getNodes(const std::string& key, int count) const {
        std::vector<std::string> result;
        if (ring_.empty()) return result;

        uint32_t hash = hashKey(key);
        auto it = ring_.lower_bound(hash);

        std::set<std::string> seen;
        int steps = 0;

        while (result.size() < static_cast<size_t>(count) 
               && steps < static_cast<int>(ring_.size())) {
            if (it == ring_.end()) it = ring_.begin();

            if (seen.find(it->second) == seen.end()) {
                result.push_back(it->second);
                seen.insert(it->second);
            }

            ++it;
            ++steps;
        }

        return result;
    }

    size_t size() const { return ring_.size(); }

private:
    uint32_t hashKey(const std::string& key) const {
        // FNV-1a hash (simple, good distribution)
        uint32_t hash = 2166136261u;
        for (char c : key) {
            hash ^= static_cast<uint32_t>(c);
            hash *= 16777619u;
        }
        return hash;
    }

    std::map<uint32_t, std::string> ring_;  // hash_position → physical_node
    int vnodes_per_node_;
};

// ---- Usage & Test ----
int main() {
    ConsistentHashRing ring(150);

    ring.addNode("server-A");
    ring.addNode("server-B");
    ring.addNode("server-C");

    // Distribute 10000 keys, check balance
    std::map<std::string, int> distribution;
    for (int i = 0; i < 10000; ++i) {
        std::string key = "key:" + std::to_string(i);
        distribution[ring.getNode(key)]++;
    }

    std::cout << "=== Distribution (3 nodes, 10000 keys) ===" << std::endl;
    for (auto& [node, count] : distribution) {
        std::cout << node << ": " << count << " keys ("
                  << (count * 100.0 / 10000) << "%)" << std::endl;
    }

    // Add a 4th node — how many keys move?
    std::map<std::string, std::string> before;
    for (int i = 0; i < 10000; ++i) {
        std::string key = "key:" + std::to_string(i);
        before[key] = ring.getNode(key);
    }

    ring.addNode("server-D");

    int moved = 0;
    for (int i = 0; i < 10000; ++i) {
        std::string key = "key:" + std::to_string(i);
        if (ring.getNode(key) != before[key]) moved++;
    }

    std::cout << "\n=== After adding server-D ===" << std::endl;
    std::cout << "Keys moved: " << moved << " / 10000 ("
              << (moved * 100.0 / 10000) << "%)" << std::endl;
    std::cout << "Ideal (1/N): " << (100.0 / 4) << "%" << std::endl;

    // Replication: get 3 nodes for a key
    auto replicas = ring.getNodes("important-key", 3);
    std::cout << "\n=== Replicas for 'important-key' ===" << std::endl;
    for (auto& r : replicas) std::cout << "  " << r << std::endl;

    return 0;
}
```

**Compile & run:**
```bash
g++ -std=c++17 -o consistent_hash consistent_hash.cpp && ./consistent_hash
```

Expected output:
```
=== Distribution (3 nodes, 10000 keys) ===
server-A: 3340 keys (33.4%)
server-B: 3280 keys (32.8%)
server-C: 3380 keys (33.8%)

=== After adding server-D ===
Keys moved: 2520 / 10000 (25.2%)
Ideal (1/N): 25%

=== Replicas for 'important-key' ===
  server-C
  server-A
  server-B
```

---

## 5. Key Design Decisions

### Hash Function Choice
| Function | Speed | Distribution | Use Case |
|----------|-------|-------------|----------|
| FNV-1a | Very fast | Good | General purpose, internal |
| MD5 | Moderate | Excellent | Cassandra uses this |
| MurmurHash3 | Fast | Excellent | Best for consistent hashing |
| SHA-1/256 | Slow | Excellent | When you need crypto-grade |

For consistent hashing, **MurmurHash3** or **xxHash** are the best tradeoff of speed and distribution.

### Replication with Consistent Hashing
```
For a key K with replication factor RF=3:

1. Hash K → position P on ring
2. Walk clockwise, collect first RF *distinct physical nodes*
3. These are the replica set for K

      Server A(100)
          ↑
  K(90) → A is primary
           ↓ clockwise
      Server B(400) → first replica
           ↓ clockwise  
      Server C(700) → second replica
      
  Replica set: {A, B, C}
```

**Skipping vnodes of the same physical node is critical** — otherwise all 3 replicas could be on the same machine.

### Handling Node Failures
```
Node B dies:

Before: Key ranges → A, B, C
After:  B's ranges get absorbed by next clockwise nodes

With vnodes, B's load distributes across ALL remaining nodes
(not dumped onto one node like basic consistent hashing)

Cassandra calls this "hinted handoff":
  - Writes meant for B go to next node temporarily
  - When B comes back, hints are replayed to B
```

---

## 6. Where It's Used

| System | Details |
|--------|---------|
| **Amazon DynamoDB** | Consistent hashing + virtual nodes for partition placement |
| **Apache Cassandra** | Token ring with 256 vnodes per node (configurable) |
| **Riak** | Ring-based partitioning with configurable N/R/W |
| **Memcached** (client-side) | Clients use consistent hashing to pick server |
| **Nginx** | `consistent_hash` load balancing directive |
| **Ceph CRUSH** | Extended form — CRUSH algorithm uses hash + hierarchy |
| **Akamai CDN** | Original use case from the 1997 paper |

---

## 7. Interview Questions

**Q: "Why not just use `hash % N`?"**  
A: Adding/removing a server remaps ~(N-1)/N keys. For a cache with 1B keys, that's a catastrophic stampede. Consistent hashing remaps only ~1/N keys.

**Q: "How do you handle hot keys?"**  
A: Options:
1. More vnodes for the hot key's range (doesn't help if one key is hot)
2. Read replicas — serve reads from any replica in the key's replica set
3. Client-side caching with TTL
4. Split the hot key (`key:user:123:shard:0`, `key:user:123:shard:1`)

**Q: "How does this relate to your storage system work?"**  
A: "Consistent hashing is the foundation for data placement in distributed storage. In StoreOnce, our replication topology determines which nodes hold copies of backup data. When scaling the cluster, we need to rebalance minimally — consistent hashing ensures only ~1/N data migrates when adding a node."

**Q: "Implement `getNode()` — what's the time complexity?"**  
A: Using `std::map` + `lower_bound()`: O(log V) where V = total vnodes. This is a balanced BST lookup. Could also use a sorted array + binary search for better cache locality.

---

## 8. Advanced: Bounded-Load Consistent Hashing (Google, 2017)

Problem: Even with vnodes, some nodes can be overloaded due to request patterns.

Solution: Set a capacity cap per node (e.g., 1.25 × average). If a node hits its cap, the key falls through to the next node clockwise.

```
Average load per node = total_requests / N
Cap = average × (1 + epsilon)  // epsilon typically 0.25

getNode(key):
  Walk clockwise from hash(key)
  For each node encountered:
    if node.load < cap:
      return node
    else:
      continue clockwise  // skip overloaded node
```

This was published by Google and is used in their load balancers.

---

## Quick Reference Card

```
Consistent Hashing:
  - Nodes + keys on a hash ring (0 to 2^32)
  - Key → walk clockwise → first node = owner
  - Add/remove node → only ~1/N keys remapped

Virtual Nodes:
  - Each physical node gets 100-256 positions on ring
  - Ensures even distribution
  - Failed node's load spreads across ALL remaining nodes

Replication:
  - Walk clockwise, pick RF distinct physical nodes
  - Skip vnodes of same physical server

Implementation:
  - std::map<uint32_t, string> ring
  - getNode(): lower_bound(hash) → O(log V)
  - Hash: MurmurHash3 or FNV-1a

Used by: Cassandra, DynamoDB, Riak, Memcached, Akamai CDN
```
