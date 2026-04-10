#!/usr/bin/ksh
#
# Prerequisites Check Script for AIX Monitoring Suite
# Purpose: Verify system requirements before running monitoring scripts
# Version: 2.0
#

echo "=========================================="
echo "AIX Monitoring Prerequisites Check"
echo "=========================================="
echo "Date: $(date)"
echo "Hostname: $(hostname)"
echo "Current user: $(whoami)"
echo "User groups: $(groups)"
echo "Shell: $0"
echo ""

# Function to check command availability - AIX compatible
check_command() {
    local cmd="$1"
    local description="$2"
    
    # Use command -v which is more portable than which
    if command -v "$cmd" >/dev/null 2>&1; then
        echo "  ✓ $cmd: Available - $description"
        return 0
    else
        echo "  ✗ $cmd: MISSING - $description"
        return 1
    fi
}

# Function to check file/directory
check_path() {
    local path="$1"
    local description="$2"
    
    if [ -e "$path" ]; then
        echo "  ✓ $path: Found - $description"
        return 0
    else
        echo "  ✗ $path: NOT FOUND - $description"
        return 1
    fi
}

# Initialize counters
TOTAL_CHECKS=0
PASSED_CHECKS=0

echo "=== Shell Environment ==="

echo "Current shell: $0"
if [ -x "/usr/bin/ksh" ] || [ -x "/bin/ksh" ]; then
    echo "  ✓ ksh: Available - required for monitoring scripts"
    KSH_VERSION=$(ksh --version 2>/dev/null || echo "Version info not available")
    echo "    Version: $KSH_VERSION"
else
    echo "  ✗ ksh: Not found - monitoring scripts require Korn shell"
    echo "    Scripts may fail with other shells"
fi

# Test some ksh-specific features
echo "  Testing ksh features..."
if (( 1 + 1 == 2 )) 2>/dev/null; then
    echo "  ✓ Arithmetic expansion works"
else
    echo "  ⚠ Arithmetic expansion may not work properly"
fi

echo ""

echo "=== AIX Version and Hardware Detection ==="

# Detect AIX version
AIX_VERSION=""
POWER_TYPE=""
if command -v oslevel >/dev/null 2>&1; then
    AIX_VERSION=$(oslevel -s 2>/dev/null || oslevel 2>/dev/null || echo "Unknown")
    echo "  ✓ AIX Version: $AIX_VERSION"
    
    # Parse major version
    AIX_MAJOR=$(echo "$AIX_VERSION" | cut -c1)
    AIX_MINOR=$(echo "$AIX_VERSION" | cut -c2)
    echo "    Major: $AIX_MAJOR, Minor: $AIX_MINOR"
    
    # Determine capabilities based on version
    if [ "$AIX_MAJOR" -ge 7 ]; then
        echo "    ✓ AIX 7.x detected - Full feature support"
        SUPPORTS_ENHANCED_LPAR=1
        SUPPORTS_ADVANCED_STORAGE=1
    elif [ "$AIX_MAJOR" -eq 6 ]; then
        echo "    ⚠ AIX 6.x detected - Limited feature support"
        SUPPORTS_ENHANCED_LPAR=0
        SUPPORTS_ADVANCED_STORAGE=0
    else
        echo "    ⚠ Older AIX version - Some features may not be available"
        SUPPORTS_ENHANCED_LPAR=0
        SUPPORTS_ADVANCED_STORAGE=0
    fi
else
    echo "  ✗ oslevel command not available - cannot determine AIX version"
    AIX_VERSION="Unknown"
    SUPPORTS_ENHANCED_LPAR=0
    SUPPORTS_ADVANCED_STORAGE=0
fi

# Detect Power hardware
if command -v prtconf >/dev/null 2>&1; then
    PROCESSOR_TYPE=$(prtconf 2>/dev/null | grep "Processor Type" | awk -F: '{print $2}' | sed 's/^ *//')
    if [ -n "$PROCESSOR_TYPE" ]; then
        echo "  ✓ Processor Type: $PROCESSOR_TYPE"
        
        # Determine Power generation
        if echo "$PROCESSOR_TYPE" | grep -i "power10" >/dev/null; then
            POWER_TYPE="POWER10"
            echo "    ✅ POWER10 detected - Latest features available"
            SUPPORTS_NVME=1
            SUPPORTS_ENHANCED_VIRT=1
        elif echo "$PROCESSOR_TYPE" | grep -i "power9" >/dev/null; then
            POWER_TYPE="POWER9"
            echo "    ✅ POWER9 detected - Modern features available"
            SUPPORTS_NVME=1
            SUPPORTS_ENHANCED_VIRT=1
        elif echo "$PROCESSOR_TYPE" | grep -i "power8" >/dev/null; then
            POWER_TYPE="POWER8"
            echo "    ⚠ POWER8 detected - Some modern features may be limited"
            SUPPORTS_NVME=0
            SUPPORTS_ENHANCED_VIRT=0
        elif echo "$PROCESSOR_TYPE" | grep -i "power" >/dev/null; then
            POWER_TYPE="POWER_LEGACY"
            echo "    ⚠ Older POWER processor - Limited feature support"
            SUPPORTS_NVME=0
            SUPPORTS_ENHANCED_VIRT=0
        else
            POWER_TYPE="UNKNOWN"
            echo "    ❓ Processor type not recognized: $PROCESSOR_TYPE"
            SUPPORTS_NVME=0
            SUPPORTS_ENHANCED_VIRT=0
        fi
    else
        echo "  ⚠ Unable to determine processor type"
        POWER_TYPE="UNKNOWN"
        SUPPORTS_NVME=0
        SUPPORTS_ENHANCED_VIRT=0
    fi
else
    echo "  ✗ prtconf command not available"
    POWER_TYPE="UNKNOWN"
    SUPPORTS_NVME=0
    SUPPORTS_ENHANCED_VIRT=0
fi

echo ""

# Check basic system commands
echo "=== Basic System Commands ==="
BASIC_COMMANDS="date hostname ps kill mkdir chmod df awk sed cut sort uniq wc tr head tail grep find cat sleep"
for cmd in $BASIC_COMMANDS; do
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    if check_command "$cmd" "Basic system command"; then
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    fi
done
echo ""

# Check performance monitoring commands
echo "=== Performance Monitoring Commands ==="
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "vmstat" "Virtual memory statistics"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "iostat" "I/O statistics"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "netstat" "Network statistics"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "svmon" "System virtual memory monitor"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lsps" "List paging spaces"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "errpt" "Error reporting"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

# Check LPAR statistics (available on AIX 5.3+ but enhanced on 7.x)
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lparstat" "LPAR statistics"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
    if [ "$SUPPORTS_ENHANCED_LPAR" = "1" ]; then
        echo "    ✓ Enhanced LPAR features available (AIX 7.x)"
    else
        echo "    ⚠ Basic LPAR features only"
    fi
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "prtconf" "System configuration"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

# Additional commands for POWER9/10 systems
if [ "$POWER_TYPE" = "POWER9" ] || [ "$POWER_TYPE" = "POWER10" ]; then
    echo "  === POWER9/10 Specific Commands ==="
    
    # Check for enhanced monitoring capabilities
    if which "mpstat" >/dev/null 2>&1; then
        echo "  ✓ mpstat: Available - Multi-processor statistics"
    else
        echo "  ⚠ mpstat: Not available - Enhanced CPU monitoring limited"
    fi
    
    if which "tprof" >/dev/null 2>&1; then
        echo "  ✓ tprof: Available - Performance profiling"
    else
        echo "  - tprof: Not available - Performance profiling not available"
    fi
fi

# POWER10 specific features
if [ "$POWER_TYPE" = "POWER10" ]; then
    echo "  === POWER10 Specific Features ==="
    
    # Check for POWER10 enhanced features
    if which "lssrc" >/dev/null 2>&1; then
        echo "  ✓ lssrc: Available - System resource controller"
    fi
    
    # Check for advanced virtualization support
    if [ "$SUPPORTS_ENHANCED_VIRT" = "1" ]; then
        echo "  ✓ Enhanced virtualization features available"
    fi
fi
echo ""

# Check storage management commands
echo "=== Storage Management Commands ==="
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lsvg" "List volume groups"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lslv" "List logical volumes"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lspv" "List physical volumes"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lsdev" "List devices"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lsattr" "List device attributes"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "mount" "Mount information"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "lscfg" "List device configuration"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

# Check for advanced storage features based on AIX version
if [ "$SUPPORTS_ADVANCED_STORAGE" = "1" ]; then
    echo "  === AIX 7.x Advanced Storage Features ==="
    
    # Enhanced LVM commands
    if which "lvmstat" >/dev/null 2>&1; then
        echo "  ✓ lvmstat: Available - Enhanced LVM statistics"
    else
        echo "  ⚠ lvmstat: Not available - Advanced LVM monitoring limited"
    fi
    
    # Check for modern filesystem support
    if which "lsfs" >/dev/null 2>&1; then
        echo "  ✓ lsfs: Available - Filesystem management"
    fi
fi

# POWER9/10 specific storage features
if [ "$SUPPORTS_NVME" = "1" ]; then
    echo "  === NVMe Support (POWER9/10) ==="
    
    # Check for NVMe devices
    NVME_COUNT=$(lsdev -Cc disk 2>/dev/null | grep -i nvme | wc -l | tr -d ' ')
    if [ "$NVME_COUNT" -gt 0 ]; then
        echo "  ✅ NVMe devices detected: $NVME_COUNT"
    else
        echo "  ⚠ NVMe capable but no devices found"
    fi
    
    # Check for NVMe-specific tools
    if which "nvme" >/dev/null 2>&1; then
        echo "  ✓ nvme: Available - NVMe management tool"
    else
        echo "  ⚠ nvme: Not available - NVMe management limited"
    fi
fi

# Check optional storage commands
echo ""
echo "=== Optional Storage and Network Commands ==="

# LVM statistics (more important on newer systems)
if which "lvmstat" >/dev/null 2>&1; then
    echo "  ✓ lvmstat: Available - LVM statistics"
    if [ "$SUPPORTS_ADVANCED_STORAGE" = "1" ]; then
        echo "    ✓ Enhanced LVM features supported"
    fi
else
    echo "  - lvmstat: Not available - LVM statistics (optional)"
    if [ "$SUPPORTS_ADVANCED_STORAGE" = "1" ]; then
        echo "    ⚠ Recommended for AIX 7.x systems"
    fi
fi

# Multipath I/O (critical for enterprise storage)
if which "mpio" >/dev/null 2>&1; then
    echo "  ✓ mpio: Available - Multipath I/O"
    MPIO_DEVICES=$(lsdev -Cc mpio 2>/dev/null | wc -l | tr -d ' ')
    if [ "$MPIO_DEVICES" -gt 0 ]; then
        echo "    ✓ MPIO devices configured: $MPIO_DEVICES"
    fi
else
    echo "  - mpio: Not available - Multipath I/O (optional but recommended for SAN)"
fi

# NFS statistics
if which "nfsstat" >/dev/null 2>&1; then
    echo "  ✓ nfsstat: Available - NFS statistics"
else
    echo "  - nfsstat: Not available - NFS statistics (optional)"
fi

if which "showmount" >/dev/null 2>&1; then
    echo "  ✓ showmount: Available - NFS mount information"
else
    echo "  - showmount: Not available - NFS mount information (optional)"
fi

# List open files (important for database monitoring)
if which "lsof" >/dev/null 2>&1; then
    echo "  ✓ lsof: Available - List open files"
    echo "    ✓ Important for Oracle file monitoring"
else
    echo "  - lsof: Not available - List open files (optional but recommended for Oracle)"
fi

# Network adapter statistics
if which "entstat" >/dev/null 2>&1; then
    echo "  ✓ entstat: Available - Ethernet statistics"
    # Check if we have any ethernet adapters
    ETH_COUNT=$(lsdev -Cc adapter 2>/dev/null | grep -i ent | wc -l | tr -d ' ')
    if [ "$ETH_COUNT" -gt 0 ]; then
        echo "    ✓ Ethernet adapters found: $ETH_COUNT"
    fi
else
    echo "  - entstat: Not available - Ethernet statistics (optional)"
fi

# Fibre Channel statistics (important for SAN connectivity)
if which "fcstat" >/dev/null 2>&1; then
    echo "  ✓ fcstat: Available - Fibre Channel statistics"
    # Check for FC adapters
    FC_COUNT=$(lsdev -Cc adapter 2>/dev/null | grep -i fcs | wc -l | tr -d ' ')
    if [ "$FC_COUNT" -gt 0 ]; then
        echo "    ✓ FC adapters found: $FC_COUNT (critical for backup monitoring)"
    else
        echo "    ⚠ No FC adapters found - backup connectivity may be limited"
    fi
else
    echo "  - fcstat: Not available - Fibre Channel statistics (recommended for SAN/backup)"
fi
echo ""
echo "=== Additional System Tools ==="

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "oslevel" "Operating system level"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
    echo "    System level: $(oslevel 2>/dev/null || echo 'Unable to determine')"
fi

# dmesg is optional - AIX 7.2 often doesn't have it
if which "dmesg" >/dev/null 2>&1; then
    echo "  ✓ dmesg: Available - Display kernel messages"
else
    echo "  ⚠ dmesg: Not available - using AIX errpt instead (this is normal on AIX 7.2)"
    echo "    AIX uses errpt for system/kernel error reporting"
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "jobs" "Job control"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

# Check if xargs supports -r flag (GNU style)
echo "  Testing xargs compatibility..."
if echo "test" | xargs -r echo >/dev/null 2>&1; then
    echo "  ✓ xargs: GNU-style -r flag supported"
else
    echo "  ⚠ xargs: GNU-style -r flag not supported (AIX version)"
fi

echo ""

# Check network diagnostic tools
echo "=== Network Diagnostic Tools ==="
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "ping" "ICMP ping test"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "traceroute" "Route tracing"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi

TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
if check_command "telnet" "Port connectivity test"; then
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
fi
echo ""
echo "=== Timeout Mechanisms ==="

# Check for timeout command
if which "timeout" >/dev/null 2>&1; then
    echo "  ✓ timeout: Available - GNU timeout utility"
    # Test if it works
    if timeout 1 sleep 0.5 >/dev/null 2>&1; then
        echo "    ✓ timeout command works correctly"
    else
        echo "    ⚠ timeout command present but may not work as expected"
    fi
else
    echo "  ✗ timeout: Not available - will use fallback mechanism"
fi

# Check for our AIX timeout utility
SCRIPT_DIR="$(dirname "$0")"
if [ -x "$SCRIPT_DIR/aix_timeout.sh" ]; then
    echo "  ✓ aix_timeout.sh: Available - custom timeout utility"
else
    echo "  ⚠ aix_timeout.sh: Not found - may cause timeout issues in tests"
fi

echo ""

# Check Oracle environment
echo "=== Oracle Environment ==="
if [ -n "$ORACLE_HOME" ]; then
    echo "  ✓ ORACLE_HOME: $ORACLE_HOME"
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    if check_path "$ORACLE_HOME/bin/sqlplus" "Oracle SQL*Plus"; then
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    fi
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    if check_path "$ORACLE_HOME/bin/rman" "Oracle RMAN"; then
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    fi
    
    # Test Oracle connectivity
    echo "  Testing Oracle connectivity..."
    if echo "SELECT 'TEST' FROM DUAL;" | "$ORACLE_HOME/bin/sqlplus" -S / as sysdba >/dev/null 2>&1; then
        echo "  ✓ Oracle database connection: Successful"
        TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    else
        echo "  ✗ Oracle database connection: Failed (database may be down or user lacks privileges)"
        TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    fi
else
    echo "  ✗ ORACLE_HOME: Not set"
    echo "  - To set Oracle environment:"
    echo "    export ORACLE_HOME=/path/to/oracle/home"
    echo "    export ORACLE_SID=your_database_sid"
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
fi

if [ -n "$ORACLE_SID" ]; then
    echo "  ✓ ORACLE_SID: $ORACLE_SID"
else
    echo "  ✗ ORACLE_SID: Not set"
fi
echo ""

# Check monitoring script files
echo "=== Monitoring Script Files ==="
SCRIPT_DIR="$(dirname "$0")"

# Define scripts and their descriptions separately to avoid parsing issues
check_script_file() {
    local script="$1"
    local desc="$2"
    local script_path="$SCRIPT_DIR/$script"
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    if [ -f "$script_path" ]; then
        if [ -x "$script_path" ]; then
            echo "  ✓ $script: Found and executable - $desc"
            PASSED_CHECKS=$((PASSED_CHECKS + 1))
        else
            echo "  ⚠ $script: Found but not executable - $desc"
            echo "    Run: chmod +x $script_path"
        fi
    else
        echo "  ✗ $script: Not found - $desc"
    fi
}

# Check each script individually
check_script_file "master_monitor.sh" "Main orchestration script"
check_script_file "aix_system_monitor.sh" "System monitoring"
check_script_file "oracle_rman_monitor.sh" "Oracle monitoring"
check_script_file "storage_monitor.sh" "Storage monitoring"
check_script_file "network_monitor.sh" "Network monitoring"
check_script_file "realtime_monitor.sh" "Real-time monitoring"
check_script_file "discover_storeonce.sh" "StoreOnce discovery utility"
check_script_file "run_35_hour_monitoring.sh" "35-hour monitoring wrapper"
check_script_file "test_power10_compatibility.sh" "Power 10 compatibility test"
check_script_file "test_aix72_compatibility.sh" "AIX 7.2 compatibility test"
check_script_file "aix_timeout.sh" "AIX timeout utility"
echo ""

# Check disk space
echo "=== System Resources ==="
echo "Disk space check for common log directories:"
for dir in /tmp /var/tmp .; do
    if [ -d "$dir" ]; then
        space=$(df -g "$dir" 2>/dev/null | tail -1 | awk '{print $3}')
        if [ -n "$space" ] && [ "$space" -gt 0 ]; then
            echo "  ✓ $dir: ${space}GB available"
            if [ "$space" -lt 1 ]; then
                echo "    ⚠ Warning: Less than 1GB available"
            fi
        else
            echo "  ⚠ $dir: Unable to determine space"
        fi
    fi
done
echo ""

echo ""
echo "=== Version-Specific Recommendations ==="

# Provide recommendations based on detected environment
if [ "$AIX_VERSION" != "Unknown" ]; then
    echo "Based on your AIX version ($AIX_VERSION) and hardware ($POWER_TYPE):"
    echo ""
    
    # AIX 7.x recommendations
    if [ "$AIX_MAJOR" -ge 7 ]; then
        echo "  ✅ AIX 7.x Recommendations:"
        echo "    - Enable enhanced LPAR monitoring (lparstat -E)"
        echo "    - Use advanced LVM features (lvmstat)"
        echo "    - Monitor large page usage (svmon -P)"
        echo "    - Use errpt instead of dmesg for kernel messages"
        
        if [ "$POWER_TYPE" = "POWER10" ]; then
            echo "  ✅ POWER10 Optimizations:"
            echo "    - Monitor NVMe storage performance"
            echo "    - Use enhanced virtualization monitoring"
            echo "    - Enable advanced CPU features monitoring"
        elif [ "$POWER_TYPE" = "POWER9" ]; then
            echo "  ✅ POWER9 Optimizations:"
            echo "    - Monitor SMT thread efficiency"
            echo "    - Use enhanced network monitoring"
            echo "    - Monitor memory bandwidth"
        fi
    elif [ "$AIX_MAJOR" -eq 6 ]; then
        echo "  ⚠ AIX 6.x Limitations:"
        echo "    - Some enhanced monitoring features unavailable"
        echo "    - Limited LPAR statistics"
        echo "    - Basic storage monitoring only"
        echo "    - Consider upgrading to AIX 7.x for full functionality"
    fi
    
    # Power hardware specific recommendations
    if [ "$POWER_TYPE" = "POWER_LEGACY" ] || [ "$POWER_TYPE" = "POWER8" ]; then
        echo "  ⚠ Older POWER Hardware:"
        echo "    - NVMe monitoring not available"
        echo "    - Limited virtualization features"
        echo "    - Some network optimizations unavailable"
    fi
else
    echo "  ⚠ Cannot determine AIX version - using conservative feature set"
fi

echo ""

echo "=== Performance and Compatibility Notes ==="

# Check for common issues
echo "Checking for common compatibility issues:"

# Check date command format support (important for time calculations)
if date -d "1 minute ago" >/dev/null 2>&1; then
    echo "  ✓ GNU date format supported"
else
    echo "  ⚠ GNU date format not supported - using AIX-compatible date logic"
    echo "    This is normal on AIX systems - monitoring scripts use AIX date format"
fi

# Check if bc is available for calculations
if which "bc" >/dev/null 2>&1; then
    echo "  ✓ bc: Available - calculator utility"
else
    echo "  ⚠ bc: Not available - using awk for calculations"
    echo "    This is normal on AIX - awk provides calculation capabilities"
fi

# Version-specific compatibility checks
if [ "$AIX_MAJOR" -ge 7 ]; then
    echo "  ✓ AIX 7.x: Enhanced command compatibility"
    
    # Check for enhanced features
    if lparstat -h >/dev/null 2>&1; then
        echo "    ✓ Enhanced lparstat options available"
    fi
    
    if iostat -h >/dev/null 2>&1; then
        echo "    ✓ Enhanced iostat options available"
    fi
elif [ "$AIX_MAJOR" -eq 6 ]; then
    echo "  ⚠ AIX 6.x: Some commands may have limited options"
    echo "    Monitoring scripts will use compatible command syntax"
fi

# Check if timeout issues might occur
if [ ! -x "$SCRIPT_DIR/aix_timeout.sh" ] && ! which "timeout" >/dev/null 2>&1; then
    echo "  ⚠ WARNING: No timeout mechanism available"
    echo "    This may cause scripts to hang during testing"
    echo "    Recommendation: Ensure aix_timeout.sh is present and executable"
fi

# Hardware-specific considerations
if [ "$POWER_TYPE" = "POWER10" ]; then
    echo "  ✅ POWER10: All advanced monitoring features supported"
elif [ "$POWER_TYPE" = "POWER9" ]; then
    echo "  ✅ POWER9: Most advanced monitoring features supported"
elif [ "$POWER_TYPE" = "POWER8" ]; then
    echo "  ⚠ POWER8: Some advanced features may be limited"
fi

# Check system load
LOAD_AVG=$(uptime 2>/dev/null | awk -F'load average:' '{print $2}' | awk '{print $1}' | tr -d ',')
if [ -n "$LOAD_AVG" ]; then
    echo "  Current system load: $LOAD_AVG"
    # Convert to integer for comparison (multiply by 100) - AIX awk compatible
    LOAD_INT=$(echo "$LOAD_AVG" | awk '{printf "%.0f", $1 * 100}' 2>/dev/null || echo "0")
    if [ "$LOAD_INT" -gt 500 ]; then
        echo "    ⚠ High system load detected - monitoring may be affected"
    else
        echo "    ✓ System load acceptable for monitoring"
    fi
fi

echo ""

# Summary
echo "=========================================="
echo "PREREQUISITES SUMMARY"
echo "=========================================="
echo "System Environment:"
echo "  AIX Version: $AIX_VERSION"
echo "  Power Hardware: $POWER_TYPE"
echo "  Shell: $0"
echo ""
echo "Check Results:"
echo "  Total checks: $TOTAL_CHECKS"
echo "  Passed: $PASSED_CHECKS"
echo "  Failed: $((TOTAL_CHECKS - PASSED_CHECKS))"

if [ $PASSED_CHECKS -eq $TOTAL_CHECKS ]; then
    echo ""
    echo "✅ ALL PREREQUISITES MET"
    echo "The monitoring suite is ready to use!"
    echo ""
    if [ "$POWER_TYPE" = "POWER10" ] && [ "$AIX_MAJOR" -ge 7 ]; then
        echo "🚀 OPTIMAL CONFIGURATION DETECTED:"
        echo "  Your AIX 7.x on POWER10 system supports all advanced features!"
    elif [ "$POWER_TYPE" = "POWER9" ] && [ "$AIX_MAJOR" -ge 7 ]; then
        echo "✅ EXCELLENT CONFIGURATION:"
        echo "  Your AIX 7.x on POWER9 system supports most advanced features!"
    fi
    echo ""
    echo "To start monitoring:"
    echo "  ./master_monitor.sh [log_dir] [duration_minutes] [storeonce_ips]"
    echo ""
    echo "For 35-hour continuous monitoring:"
    echo "  ./run_35_hour_monitoring.sh [log_dir] [storeonce_ips]"
elif [ $PASSED_CHECKS -gt $((TOTAL_CHECKS * 3 / 4)) ]; then
    echo ""
    echo "⚠ MOST PREREQUISITES MET"
    echo "The monitoring suite should work with limited functionality."
    echo ""
    if [ "$AIX_MAJOR" -eq 6 ]; then
        echo "AIX 6.x detected - some advanced features unavailable:"
        echo "  - Enhanced LPAR monitoring limited"
        echo "  - Advanced storage features may not work"
        echo "  - Consider upgrading to AIX 7.x for full functionality"
    fi
    if [ "$POWER_TYPE" = "POWER8" ] || [ "$POWER_TYPE" = "POWER_LEGACY" ]; then
        echo "Older POWER hardware detected:"
        echo "  - NVMe monitoring unavailable"
        echo "  - Some virtualization features limited"
    fi
    echo ""
    echo "Review failed checks above for optimal performance."
else
    echo ""
    echo "❌ INSUFFICIENT PREREQUISITES"
    echo "Please address the failed checks above before running monitoring scripts."
    echo ""
    if [ "$AIX_VERSION" = "Unknown" ]; then
        echo "⚠ Could not determine AIX version - this may indicate:"
        echo "  - System commands not available"
        echo "  - Permission issues"
        echo "  - Non-AIX system"
    fi
fi

echo ""
echo "Version-Specific Notes:"
if [ "$AIX_MAJOR" -ge 7 ] && [ "$POWER_TYPE" = "POWER10" ]; then
    echo "  ✅ Perfect for enterprise backup monitoring"
    echo "  ✅ All Oracle RMAN monitoring features available"
    echo "  ✅ Advanced storage and network monitoring supported"
elif [ "$AIX_MAJOR" -ge 7 ]; then
    echo "  ✅ Excellent for backup monitoring"
    echo "  ✅ Oracle RMAN monitoring fully supported"
elif [ "$AIX_MAJOR" -eq 6 ]; then
    echo "  ⚠ Basic monitoring available"
    echo "  ⚠ Some Oracle features may be limited"
    echo "  💡 Recommend upgrading to AIX 7.x"
fi

echo ""
echo "For detailed setup instructions, see README.md"
echo "Check completed at: $(date)"
echo "=========================================="
