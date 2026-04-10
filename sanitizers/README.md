# Sanitizers Learning Guide

Learn AddressSanitizer (ASan) and ThreadSanitizer (TSan) through hands-on practice.

## Learning Path

### Phase 1: AddressSanitizer (Memory Bugs)
1. `asan_01_buffer_overflow.cpp` - Heap buffer overflow
2. `asan_02_stack_overflow.cpp` - Stack buffer overflow
3. `asan_03_use_after_free.cpp` - Use-after-free
4. `asan_04_double_free.cpp` - Double free
5. `asan_05_memory_leak.cpp` - Memory leak detection
6. `asan_06_use_after_return.cpp` - Use-after-return

### Phase 2: ThreadSanitizer (Data Races)
1. `tsan_01_basic_race.cpp` - Simple data race
2. `tsan_02_shared_counter.cpp` - Unprotected shared counter
3. `tsan_03_lazy_init.cpp` - Double-checked locking bug
4. `tsan_04_vector_race.cpp` - Container data race
5. `tsan_05_atomic_fix.cpp` - Fixing with atomics
6. `tsan_06_mutex_fix.cpp` - Fixing with mutex

## How to Use

### For AddressSanitizer:
```bash
# Compile with ASan
clang++ -fsanitize=address -g -O1 asan_01_buffer_overflow.cpp -o asan_01

# Run and observe the error
./asan_01

# Fix the bug in the code
# Recompile and verify it's fixed
```

### For ThreadSanitizer:
```bash
# Compile with TSan
clang++ -fsanitize=thread -g -O1 tsan_01_basic_race.cpp -o tsan_01

# Run and observe the race report
./tsan_01

# Fix the bug in the code
# Recompile and verify no races
```

## Quick Reference

### ASan Compile Flags
```bash
clang++ -fsanitize=address -g -O1 -fno-omit-frame-pointer file.cpp
```

### TSan Compile Flags
```bash
clang++ -fsanitize=thread -g -O1 file.cpp
```

### Common ASan Options
```bash
ASAN_OPTIONS="detect_leaks=1:halt_on_error=0:log_path=asan.log" ./program
```

### Common TSan Options
```bash
TSAN_OPTIONS="halt_on_error=1:history_size=7:log_path=tsan.log" ./program
```

## Learning Strategy

1. **Run the buggy version first** - See what the sanitizer reports
2. **Understand the error** - Read the sanitizer output carefully
3. **Fix the bug** - Apply the correct solution
4. **Verify the fix** - Rerun with sanitizer to confirm it's clean
5. **Experiment** - Try creating your own variations

## Tips

- Start with ASan examples (easier to understand)
- Then move to TSan examples (more complex)
- Read the sanitizer output carefully - it tells you exactly what's wrong
- Keep sanitizers enabled during development
- **Cannot use ASan and TSan together** - choose one per build

## Next Steps

After mastering these basics:
- Add sanitizers to your project's build system
- Run tests with sanitizers in CI/CD
- Learn UndefinedBehaviorSanitizer (UBSan)
- Learn MemorySanitizer (MSan)
