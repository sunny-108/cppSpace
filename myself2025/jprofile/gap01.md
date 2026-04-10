## Job Profile vs CV Comparison

### Strong Alignment

| Requirement                   | Your CV Evidence                                                                    |
| ----------------------------- | ----------------------------------------------------------------------------------- |
| 12-17 yrs systems programming | 14+ years, 10+ in C/C++ systems ✓                                                  |
| C/C++ proficiency             | C++14/17 expert, daily driver ✓                                                    |
| Concurrency & threading       | Thread pools, mutexes, condition variables, deadlock prevention — core strength ✓ |
| Linux development             | SAP-HANA, RMAN plugins on Linux ✓                                                  |
| Memory management & profiling | Valgrind, ASan, TSan, 7.8 GB leak resolution ✓                                     |
| Design patterns in production | Command, Object Pool, Singleton, Factory, Strategy ✓                               |
| Storage domain experience     | StoreOnce backup/restore platform ✓                                                |
| Some team leadership          | Led teams of 4 engineers (×2) ✓                                                   |

### Critical Gaps

| Requirement                                                                                          | Gap                                                                                                                                                                                                   |
| ---------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Distributed systems** (replication, consistency, fault tolerance, placement)                 | No distributed systems experience mentioned — this is the**biggest gap**. The role builds clustered, scale-out storage.                                                                        |
| **Networking** (RDMA, TCP stack, tcpdump)                                                      | No networking experience on CV                                                                                                                                                                        |
| **Storage platform internals** (S3 object store, NAS, erasure coding, filesystem internals)    | You built*plugins for* a storage system, not the storage system itself. The role requires building the core platform.                                                                               |
| **Senior Principal-level leadership**                                                          | Led 4 engineers ≠ "setting technical direction for critical AI data platform," "mentoring senior engineers," "cross-product architecture." The role expects architectural ownership across products. |
| **Linux kernel debugging** (strace/dtrace/eBPF)                                                | Not mentioned — the role expects deep kernel-level debugging                                                                                                                                         |
| **Performance engineering at scale** (latency/throughput profiling, NVMe/SSD/HDD optimization) | CV shows memory profiling but not system-wide performance engineering                                                                                                                                 |
| **CI/CD, automated testing, release engineering**                                              | Not mentioned                                                                                                                                                                                         |
| **Python**                                                                                     | Not on CV (good-to-have for the role)                                                                                                                                                                 |
| **Cloud platforms** (Azure, GCP, AWS)                                                          | Not mentioned                                                                                                                                                                                         |
| **Security/crypto** (FIPS, STIG/NIST, TLS, authN/Z)                                            | Coursera cert only — no production experience shown                                                                                                                                                  |
| **Design docs, RFCs, PRDs**                                                                    | Not mentioned                                                                                                                                                                                         |

### Assessment

**Your CV is a strong mid-to-senior systems engineer profile**, but this role is a **Senior Principal Engineer** — essentially a staff/architect-level position building **distributed storage infrastructure from the ground up**. The three hardest gaps to close:

1. **Distributed systems fundamentals** — you need hands-on experience or deep study of consensus protocols, replication, distributed consistency, and fault tolerance
2. **Storage platform internals** — the jump from "plugin developer on a storage product" to "building the storage product itself" is significant
3. **Principal-level technical leadership** — the role expects setting org-wide technical direction, not just leading a small team

### Recommendations to Bridge Gaps

- **Immediate study priorities:** Distributed systems (raft/paxos, CAP theorem, consistent hashing), Linux kernel debugging (strace, eBPF), networking fundamentals (TCP internals, RDMA concepts)
- **Project ideas:** Build a simple distributed key-value store in C++ to demonstrate distributed systems + C++ together
- **CV reframing:** Emphasize any cross-team architectural decisions you've made at HPE; highlight the StoreOnce *platform* context more than the plugin work; add CI/CD and testing practices you've used
