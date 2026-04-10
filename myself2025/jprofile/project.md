## How to Position It

**Don't call it "education project"** — that undersells it. Use one of these:

| Option                                 | When to Use                             |
| -------------------------------------- | --------------------------------------- |
| **Personal Engineering Project** | Safest, honest, professional            |
| **Independent Systems Research** | If you write a design doc / RFC with it |
| **Open-Source Project**          | If you publish it on GitHub (public)    |

Add it as a **separate section** between Education and Technical Highlights:

```markdown
## INDEPENDENT ENGINEERING PROJECTS

- **Distributed Key-Value Store (C++17 / Linux):** Designed and implemented a 
  distributed KV store with Raft consensus for leader election and log replication, 
  consistent hashing for data partitioning, gRPC-based inter-node communication, 
  write-ahead logging for durability, and configurable replication factor with 
  quorum-based reads/writes; includes fault injection testing and Docker-based 
  multi-node deployment
```

## Skills It Directly Fills (Dell JD Gaps)

| Dell JD Requirement                                                       | How the Project Covers It                                |
| ------------------------------------------------------------------------- | -------------------------------------------------------- |
| **Distributed systems fundamentals** (consistency, fault tolerance) | Raft consensus, quorum reads/writes, partition tolerance |
| **Replication & durability**                                        | Log replication, WAL, configurable replication factor    |
| **Networking**                                                      | gRPC / TCP socket layer between nodes                    |
| **Data paths & metadata**                                           | KV storage engine, key routing, consistent hashing       |
| **Scheduling & placement**                                          | Data placement strategy, node membership                 |
| **Observability & telemetry**                                       | If you add metrics/logging (Prometheus, structured logs) |
| **Failure injection / chaos testing**                               | Node crash, network partition, split-brain scenarios     |
| **CI/CD & automated tests**                                         | CMake + CTest + GitHub Actions pipeline                  |
| **Design docs / RFCs**                                              | Write one — the JD explicitly asks for this skill       |
| **Linux development & debugging**                                   | strace, perf, eBPF for profiling the store               |

## Recommended Tech Stack (Maximize JD Coverage)

```
Core:       C++17/20, CMake
Consensus:  Raft (implement from scratch — don't use a library)
RPC:        gRPC or raw TCP sockets (shows networking)
Storage:    RocksDB or custom LSM-tree / B-tree
Serialization: Protocol Buffers
Testing:    Google Test + fault injection framework
Profiling:  perf, strace, AddressSanitizer, ThreadSanitizer
Deployment: Docker Compose (3-5 node cluster)
CI:         GitHub Actions
```

## What Makes It Principal-Level vs Junior-Level

| Junior Project        | Principal-Level Project                                     |
| --------------------- | ----------------------------------------------------------- |
| Just stores/gets keys | Has a written**design doc / RFC**                     |
| Single-node           | 3-5 node cluster with**leader election**              |
| No failure handling   | **Chaos tests** (kill nodes, partition network)       |
| No metrics            | **Observability** (latency histograms, throughput)    |
| No profiling          | **Benchmarks** with perf/strace analysis, latency p99 |
| README only           | **Architecture diagram** + tradeoff discussion (CAP)  |

## Bottom Line

This single project, done well, covers **6 of the 8 confirmed gaps** from our earlier analysis. Publish it on GitHub (public), write a design doc, and it becomes the strongest thing you can add to your CV for this role. It transforms "plugin developer" into "someone who understands distributed storage internals."
