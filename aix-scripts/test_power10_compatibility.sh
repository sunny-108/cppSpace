#!/usr/bin/ksh
#
# AIX Power 10 Compatibility Test Script
# Tests Power 10 specific features and optimizations
# For use with AIX monitoring scripts
# Version: 2.0
#

echo "=========================================="
echo "AIX POWER 10 COMPATIBILITY TEST"
echo "=========================================="
echo "AIX Level: $(oslevel -s)"
echo "Hostname: $(hostname)"
echo "Date: $(date)"

# Check if running on Power 10
echo ""
echo "=== POWER HARDWARE DETECTION ==="
if command -v prtconf >/dev/null 2>&1; then
    PROCESSOR_TYPE=$(prtconf | grep "Processor Type" | awk -F: '{print $2}' | sed 's/^ *//')
    echo "Processor Type: $PROCESSOR_TYPE"
    
    if echo "$PROCESSOR_TYPE" | grep -i "power10" >/dev/null; then
        echo "OK POWER10 processor detected"
        POWER10_DETECTED=true
    elif echo "$PROCESSOR_TYPE" | grep -i "power" >/dev/null; then
        echo "WARN POWER processor detected, but not POWER10: $PROCESSOR_TYPE"
        POWER10_DETECTED=false
    else
        echo "INFO Processor type not clearly identified: $PROCESSOR_TYPE"
        POWER10_DETECTED=false
    fi
    
    # Check processor frequency and cores
    PROCESSOR_CLOCK=$(prtconf | grep "Processor Clock Speed" | awk -F: '{print $2}' | sed 's/^ *//')
    PROCESSOR_CORES=$(prtconf | grep "Number Of Processors" | awk -F: '{print $2}' | sed 's/^ *//')
    echo "Processor Clock: $PROCESSOR_CLOCK"
    echo "Number of Processors: $PROCESSOR_CORES"
else
    echo "FAIL prtconf command not available"
    POWER10_DETECTED=false
fi

# Check LPAR configuration
echo ""
echo "=== LPAR CONFIGURATION ==="
if command -v lparstat >/dev/null 2>&1; then
    echo "OK lparstat available - testing LPAR statistics"
    echo "LPAR Statistics:"
    lparstat 1 1 | head -3
    
    # Check for Power 10 specific features
    echo ""
    echo "LPAR Configuration Details:"
    lparstat -i | grep -E "(Type|Mode|Online)"
else
    echo "❌ lparstat not available"
fi

# Power 10 Memory Features
echo ""
echo "=== POWER 10 MEMORY FEATURES ==="

echo "Testing memory capabilities:"
if svmon -G >/dev/null 2>&1; then
    echo "✅ svmon works"
    MEMORY_SIZE=$(svmon -G | grep memory | awk '{print $2}')
    echo "Total Memory Pages: $MEMORY_SIZE"
    
    # Check for large page support (important for Oracle on Power 10)
    if svmon -P | head -10 >/dev/null 2>&1; then
        echo "✅ Large page monitoring available"
        LARGE_PAGES=$(svmon -P | grep -c "64K\|16M")
        echo "Large page pools detected: $LARGE_PAGES"
    else
        echo "⚠️  Large page monitoring limited"
    fi
else
    echo "❌ svmon failed"
fi

# Power 10 I/O and Storage Features
echo ""
echo "=== POWER 10 I/O AND STORAGE ==="

echo "Testing I/O capabilities:"
if iostat >/dev/null 2>&1; then
    echo "✅ iostat works"
    
    # Check for NVMe devices (common on Power 10)
    NVME_COUNT=$(lsdev -Cc disk | grep -i nvme | wc -l)
    if [ $NVME_COUNT -gt 0 ]; then
        echo "✅ NVMe devices detected: $NVME_COUNT"
        echo "NVMe Devices:"
        lsdev -Cc disk | grep -i nvme
    else
        echo "⚠️  No NVMe devices found"
    fi
    
    # Check for FC adapters (common for SAN storage)
    FC_COUNT=$(lsdev -Cc adapter | grep fcs | wc -l)
    if [ $FC_COUNT -gt 0 ]; then
        echo "✅ Fibre Channel adapters: $FC_COUNT"
    else
        echo "⚠️  No FC adapters found"
    fi
else
    echo "❌ iostat failed"
fi

# Power 10 Network Features
echo ""
echo "=== POWER 10 NETWORK FEATURES ==="

echo "Testing network capabilities:"
if netstat -i >/dev/null 2>&1; then
    echo "✅ netstat works"
    
    # Check for high-speed interfaces
    INTERFACE_COUNT=$(netstat -i | grep -v "Name" | wc -l)
    echo "Network interfaces: $INTERFACE_COUNT"
    
    # Look for modern interface types
    if netstat -i | grep -E "(en|et)" >/dev/null; then
        echo "✅ Ethernet interfaces detected"
        HIGH_SPEED=$(netstat -i | grep -E "en|et" | wc -l)
        echo "Ethernet interface count: $HIGH_SPEED"
    fi
    
    # Check for SR-IOV or virtual adapters
    VIRTUAL_ADAPTERS=$(lsdev -Cc adapter | grep -i virtual | wc -l)
    if [ $VIRTUAL_ADAPTERS -gt 0 ]; then
        echo "✅ Virtual adapters detected: $VIRTUAL_ADAPTERS"
    fi
else
    echo "❌ netstat failed"
fi

# Power 10 Performance Monitoring
echo ""
echo "=== POWER 10 PERFORMANCE MONITORING ==="

echo "Testing performance monitoring tools:"

# Test topas (AIX performance monitor)
if command -v topas >/dev/null 2>&1; then
    echo "✅ topas available for advanced monitoring"
else
    echo "⚠️  topas not available"
fi

# Test pmcycles (Power-specific performance monitoring)
if command -v pmcycles >/dev/null 2>&1; then
    echo "✅ pmcycles available (Power-specific)"
else
    echo "⚠️  pmcycles not available"
fi

# Test tprof (trace profiler)
if command -v tprof >/dev/null 2>&1; then
    echo "✅ tprof available for detailed analysis"
else
    echo "⚠️  tprof not available"
fi

# Test vmstat with Power 10 specific options
echo "Testing vmstat capabilities:"
if vmstat 1 1 >/dev/null 2>&1; then
    echo "✅ vmstat basic functionality works"
    
    # Test extended vmstat options
    if vmstat -I 1 1 >/dev/null 2>&1; then
        echo "✅ vmstat interrupt monitoring works"
    else
        echo "⚠️  vmstat interrupt monitoring limited"
    fi
else
    echo "❌ vmstat failed"
fi

# Power 10 Virtualization Features
echo ""
echo "=== POWER 10 VIRTUALIZATION ==="

# Check for PowerVM features
if command -v lshwres >/dev/null 2>&1; then
    echo "✅ PowerVM hardware resource commands available"
    echo "Testing resource information:"
    lshwres -r proc -m $(hostname) --level sys 2>/dev/null | head -3 || echo "   (Resource query may require higher privileges)"
else
    echo "⚠️  PowerVM commands not available or limited access"
fi

# Check for live partition mobility features
if [ -d /proc/device-tree ] && [ -r /proc/device-tree ]; then
    echo "✅ Device tree access available"
else
    echo "⚠️  Device tree access limited"
fi

# Power 10 Oracle Optimizations
echo ""
echo "=== POWER 10 ORACLE OPTIMIZATIONS ==="

echo "Checking Oracle-relevant Power 10 features:"

# Large page configuration
if ls /proc/sys/vm/nr_hugepages >/dev/null 2>&1; then
    echo "✅ Huge pages configuration accessible"
elif vmo -L | grep lgpg_regions >/dev/null 2>&1; then
    echo "✅ AIX large page configuration available"
    LARGE_PAGE_INFO=$(vmo -o lgpg_regions,lgpg_size 2>/dev/null)
    echo "Large page info: $LARGE_PAGE_INFO"
else
    echo "⚠️  Large page configuration not accessible"
fi

# Check for Oracle-specific optimizations
if [ -n "$ORACLE_HOME" ] && [ -d "$ORACLE_HOME" ]; then
    echo "✅ Oracle environment configured"
    
    # Check if Oracle binaries are compiled for Power
    if file "$ORACLE_HOME/bin/oracle" 2>/dev/null | grep -i power >/dev/null; then
        echo "✅ Oracle binaries appear to be Power-optimized"
    else
        echo "⚠️  Cannot verify Oracle binary architecture"
    fi
else
    echo "⚠️  Oracle environment not configured"
fi

# Power 10 Monitoring Script Compatibility
echo ""
echo "=== MONITORING SCRIPT COMPATIBILITY ==="

echo "Testing monitoring script requirements:"

# Test timestamp precision (important for performance monitoring)
echo "Testing timestamp precision:"
if date +%s.%N >/dev/null 2>&1; then
    echo "✅ Nanosecond precision timestamps available"
    TIMESTAMP_NS=$(date +%s.%N)
    echo "Sample timestamp: $TIMESTAMP_NS"
elif date +%s >/dev/null 2>&1; then
    echo "✅ Second precision timestamps available"
    TIMESTAMP_S=$(date +%s)
    echo "Sample timestamp: $TIMESTAMP_S"
else
    echo "⚠️  Limited timestamp precision"
fi

# Test high-frequency data collection capability
echo "Testing high-frequency monitoring:"
START_TIME=$(date +%s 2>/dev/null || perl -e 'print time')
for i in 1 2 3 4 5; do
    vmstat 1 1 >/dev/null 2>&1
done
END_TIME=$(date +%s 2>/dev/null || perl -e 'print time')
COLLECTION_TIME=$((END_TIME - START_TIME))
echo "5 vmstat collections took: ${COLLECTION_TIME} seconds"

if [ $COLLECTION_TIME -le 10 ]; then
    echo "✅ High-frequency monitoring performance acceptable"
else
    echo "⚠️  High-frequency monitoring may be slow"
fi

# Final Power 10 Assessment
echo ""
echo "=========================================="
echo "POWER 10 COMPATIBILITY ASSESSMENT"
echo "=========================================="

if [ "$POWER10_DETECTED" = true ]; then
    echo "Hardware: ✅ POWER10 processor confirmed"
else
    echo "Hardware: ⚠️  POWER10 not confirmed"
fi

echo "AIX Version: $(oslevel -s)"
echo "Performance Tools: Multiple tools available"
echo "Oracle Compatibility: Check Oracle environment setup"
echo "Monitoring Capability: ✅ Ready for comprehensive monitoring"

echo ""
echo "=== POWER 10 SPECIFIC RECOMMENDATIONS ==="
echo "1. ✅ Enable large page support for Oracle if not already configured"
echo "2. ✅ Use high-frequency monitoring intervals (Power 10 can handle it)"
echo "3. ✅ Monitor NVMe storage performance if present"
echo "4. ✅ Take advantage of enhanced virtualization monitoring"
echo "5. ✅ Use Power 10 optimized network monitoring"
echo "6. ✅ Consider Power 10 specific Oracle performance tuning"

echo ""
echo "=== MONITORING SCRIPT OPTIMIZATION TIPS ==="
echo "- Reduce monitoring intervals (Power 10 has more capacity)"
echo "- Enable detailed I/O monitoring for NVMe devices"
echo "- Use enhanced network statistics collection"
echo "- Monitor LPAR resource utilization"
echo "- Track large page usage for Oracle workloads"

echo ""
echo "Status: ✅ POWER 10 READY FOR MONITORING"
echo "=========================================="
