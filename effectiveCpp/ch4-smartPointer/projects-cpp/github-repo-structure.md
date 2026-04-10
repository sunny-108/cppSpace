# C++ GitHub Portfolio — 2026

>
> Target audience: Demonstrates Principal/Staff-level C++ expertise in concurrency, memory management, performance, and architecture.

---

## GitHub Repository Structure (apply to every project)

```
project-name/
│
├── README.md                  # Overview, motivation, build instructions, benchmarks summary
├── DESIGN.md                  # Architecture decisions, trade-offs, design diagrams
│
├── design/
│   ├── architecture.md        # High-level component diagram, data flow
│   ├── decisions/
│   │   ├── ADR-001-thread-model.md     # Architecture Decision Records
│   │   └── ADR-002-memory-layout.md
│   └── diagrams/
│       └── component.drawio   # or .svg exported diagram
│
├── src/
│   ├── core/                  # Core library / engine code
│   ├── utils/                 # Shared utilities
│   └── main.cpp
│
├── include/                   # Public headers
│
├── tests/
│   ├── unit/                  # Google Test / Catch2 unit tests
│   ├── integration/           # Multi-component integration tests
│   └── stress/                # Concurrency stress tests (run with ThreadSanitizer)
│
├── benchmarks/
│   ├── bench_main.cpp         # Google Benchmark harness
│   ├── results/
│   │   ├── baseline.json      # Captured benchmark results
│   │   └── optimized.json
│   └── README.md              # What was measured, hardware spec, methodology
│
├── profiling/
│   ├── flamegraphs/           # SVG flame graphs (perf + flamegraph.pl)
│   ├── vtune/                 # Intel VTune snapshots / reports
│   ├── heaptrack/             # Memory profiling reports (heaptrack / massif)
│   ├── tsan/                  # ThreadSanitizer reports (before fixes)
│   ├── asan/                  # AddressSanitizer reports (before fixes)
│   └── README.md              # Methodology: what profiler, what was found, what was fixed
│
├── docs/
│   ├── build.md               # Detailed build instructions (CMake options, dependencies)
│   ├── usage.md               # API usage examples
│   └── lessons-learned.md     # Key takeaways — this makes you stand out
│
├── CMakeLists.txt             # Modern CMake (target-based, no global includes)
├── vcpkg.json                 # Or conanfile.txt — dependency manifest
├── .clang-tidy                # Static analysis config
├── .clang-format              # Code style
└── .github/
    └── workflows/
        └── ci.yml             # GitHub Actions: build, test, sanitizers, benchmarks
```

---
