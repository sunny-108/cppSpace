# STO-1967: RMAN 3.5.0 Memory Exhaustion on AIX Servers

## Problem Summary

**Customer**: BASF SE (Germany)  
**Severity**: P2 (Customer Stable - Workaround Available)  
**Status**: Unresolved - Active Investigation  
**Platform**: IBM AIX (PowerPC_POWER10, 64-bit) - IBM,9105-22B  
**Plugin Version**: RMAN Catalyst Plugin 3.5.0  
**StoreOnce Version**: 4.3.9-2426.20  

### Symptom
Severe memory exhaustion on AIX servers causing system instability and process termination during RMAN backup operations using `StoreOnceCatalystCopy --copy` command.

### Critical Evidence

**Memory Consumption Pattern**:
- Process allocates 256 MB segments in heap memory
- 99 segments allocated (max) before system crash
- Final memory usage: **7998048 KB (~7.8 GB)** before termination
- Memory grows proportionally with:
  - Duration of backup window
  - Number of objects to copy

**Customer Testing Results**:
```
1-week timespan:  Memory grew to 9.17 GB (killed before completion)
2-day timespan:   Memory peaked at ~3 GB (completed successfully)
```

**Error from AIX System**:
```
Description: SOFTWARE PROGRAM ABNORMALLY TERMINATED
Probable Cause: SYSTEM RUNNING OUT OF PAGING SPACE
Program: StoreOnceCatalystCopy
Process ID: 56361422
Paging Space Use: 7998048 KB (1KB blocks)
```

## Root Cause Analysis

### Primary Finding (Lab Experiment - Aug 2025)

**Network Connectivity Issue Triggers Memory Leak**:

1. When network becomes unavailable for extended period (~40 seconds during copy operations)
2. Memory usage **continuously increases** during network downtime
3. **Critical**: Memory is **NOT released** after network recovers and stabilizes
4. Accumulated memory remains allocated until process termination

**Evidence from Customer Environment**:
- Network issue traces found in customer log files
- Intermittent connectivity problems likely causing progressive memory accumulation

### Secondary Observations

**Platform-Specific Issue**:
- Affects only AIX servers (IBM Power architecture)
- Linux hosts with same plugin version (3.5.0) work without issues
- Suggests platform-specific memory management or error handling defect

**Resource Leak Characteristics**:
- Heap memory segments (256 MB each) not deallocated
- Some segments allocated but not assigned/populated when system crashes
- Classic memory leak pattern in error recovery paths

## Technical Analysis

### Memory Leak Location
Likely in **network error handling/retry logic** within `StoreOnceCatalystCopy` binary:
- Connection failure handling
- Retry mechanism buffer allocation
- Session/context cleanup after network errors
- Object store communication layer

### Configuration Details
```
Customer Configuration:
- APPLICATION_MANAGED_COPIES: DISABLE (Catalyst-managed)
- CATALYST_PAYLOAD_CHECKSUM: DISABLE
- CATALYST_BODY_PAYLOAD_COMPRESSION: DISABLE
- Database: ARCHIVELOG mode (online backups)
```

## Impact Assessment

### Customer Impact
- **System Instability**: Frequent memory spikes causing AIX server crashes
- **Backup Failures**: Unable to run comprehensive backups (>2 days window)
- **Rollout Blocked**: Customer stopped deployment to remaining AIX servers
- **Operational Workaround**: Removed `StoreOnceCatalystCopy --copy` from scripts

### Business Impact
- Customer running without proper backup copy verification
- Reduced confidence in StoreOnce Catalyst Plugin on AIX platform
- Potential for similar issues at other AIX customer sites

## Investigation Status

### Reproduction Challenges
- **Lab Environment**: Unable to consistently reproduce issue
- **Network Simulation**: Successfully triggered memory growth with network interruption
- **Missing Element**: Cannot reliably simulate customer's specific network conditions
- **Long-term Testing**: Requires 3-4 week continuous runs (resource constraints)

### Proposed Next Steps
1. **Instrumented Patch**: Develop special build with enhanced logging for customer deployment
2. **Memory Profiling**: Add heap allocation tracking in network error paths
3. **Network Fault Injection**: Systematic testing with various network failure scenarios
4. **AIX-Specific Analysis**: Compare memory management between AIX and Linux implementations

## Current Workaround

**Customer Implementation**:
- Removed `StoreOnceCatalystCopy --copy` from backup scripts
- Systems now stable but missing backup verification functionality
- Workaround is **NOT a long-term solution**

## Related Issues

- **Bug 140581**: Original Bugzilla entry
- **STO-5415**: Similar issue reported by Sony (Japan) - different root cause
- **STO-5586**: Mitigation issue (linked)

## Engineering Assessment

### Problem Classification
- **Type**: Memory Leak (Resource Management Defect)
- **Trigger**: Network connectivity interruptions during copy operations
- **Scope**: AIX platform-specific
- **Complexity**: High - requires specific network conditions to reproduce

### Risk Level
- **Other Customers**: Medium-High (any AIX deployment with network instability)
- **Reproducibility**: Low (lab) / High (customer environment with network issues)
- **Fix Complexity**: Medium (once root cause isolated in error handling paths)

## Key Dates
- **Issue First Observed**: ~Nov 20, 2024 (45 days after plugin installation)
- **Escalation Created**: Mar 21, 2025
- **Lab Breakthrough**: Aug 25, 2025 (network connection correlation)
- **Current Status**: Dec 11, 2025 (Customer Stable, investigation ongoing)

## Recommendations

### Immediate Actions
1. Deploy instrumented build to customer environment
2. Collect detailed memory allocation traces during network interruptions
3. Review all network error handling code paths in AIX-specific layers

### Long-term Solutions
1. **Fix Memory Leaks**: Ensure all allocated buffers are freed in error paths
2. **Resource Cleanup**: Implement RAII patterns for network session management
3. **Connection Pooling**: Review and fix connection cleanup after network failures
4. **Memory Limits**: Add safeguards to prevent unbounded memory growth
5. **Enhanced Monitoring**: Add memory usage warnings before exhaustion

### Code Review Focus Areas
```cpp
- Network retry/reconnection logic
- Buffer allocation in error handling
- Session context cleanup on failures
- Object store client connection management (AIX-specific code paths)
```

## Notes
- Issue only manifests with large object counts and extended time windows
- Customer willing to work with instrumented builds for diagnosis
- Similar memory leak resolution experience from SAP-HANA Plugin (Bug 133233) may provide insights
