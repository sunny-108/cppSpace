# Build Instructions

## Prerequisites

- C++20 compiler (GCC 13+, Clang 17+, MSVC 2022+)
- CMake 3.20+
- pthread (Linux/macOS — usually preinstalled)

## Quick build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## All CMake options

| Option | Default | Description |
|---|---|---|
| `CMAKE_BUILD_TYPE` | — | `Debug`, `Release`, `RelWithDebInfo` |
| `BUILD_TESTS` | `ON` | Fetch Google Test and build test targets |
| `BUILD_BENCHMARKS` | `OFF` | Build Google Benchmark targets (requires `benchmark` installed) |
| `ENABLE_ASAN` | `OFF` | Compile with `-fsanitize=address` |
| `ENABLE_TSAN` | `OFF` | Compile with `-fsanitize=thread` |
| `ENABLE_UBSAN` | `OFF` | Compile with `-fsanitize=undefined` |

## Running tests

```bash
cmake -B build -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

## Running with sanitizers

```bash
# ThreadSanitizer (recommended for this project)
cmake -B build-tsan -DENABLE_TSAN=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tsan -j$(nproc)
cd build-tsan && ctest --output-on-failure

# AddressSanitizer
cmake -B build-asan -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-asan -j$(nproc)
cd build-asan && ctest --output-on-failure
```

## Installing Google Benchmark (for benchmarks)

```bash
# macOS
brew install google-benchmark

# Ubuntu
sudo apt-get install libbenchmark-dev

# Or from source
git clone https://github.com/google/benchmark.git
cd benchmark && cmake -B build -DBENCHMARK_ENABLE_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc) && sudo cmake --install build
```
