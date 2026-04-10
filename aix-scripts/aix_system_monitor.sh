#!/usr/bin/ksh
#
# AIX System Monitor Script for Oracle/StoreOnce Catalyst Backup Investigation
# Purpose: Monitor system resources during Oracle RMAN backup operations
# Version: 2.1.1
# Date: July 2025
#
# Usage: ./aix_system_monitor.sh [duration_minutes] [log_directory]
#

# Set default values
DURATION=${1:-60}  # Default 60 minutes
LOG_DIR=${2:-"/tmp/aix_monitoring"}
HOSTNAME=$(hostname)
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
INTERVAL=30  # 30-second intervals

# Create log directory with proper error handling
if ! mkdir -p "$LOG_DIR" 2>/dev/null; then
    echo "Error: Cannot create log directory: $LOG_DIR"
    echo "Falling back to /tmp/aix_monitoring"
    LOG_DIR="/tmp/aix_monitoring"
    mkdir -p "$LOG_DIR"
fi

# Convert LOG_DIR to absolute path to avoid path issues
LOG_DIR=$(cd "$LOG_DIR" && pwd)

echo "=========================================="
echo "AIX System Monitor Started"
echo "Host: $HOSTNAME"
echo "Start Time: $(date)"
echo "Duration: $DURATION minutes"
echo "Log Directory: $LOG_DIR"
echo "Working Directory: $(pwd)"
echo "Script Path: $0"
echo "LOG_DIR permissions: $(ls -ld "$LOG_DIR" 2>/dev/null || echo 'Directory check failed')"
echo "=========================================="

# Function to log with timestamp
log_with_timestamp() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

# Function to collect system information
collect_system_info() {
    local timestamp=$1
    local counter=$2
    local logfile="${LOG_DIR}/system_monitor_${TIMESTAMP}.log"
    
    # Verify log directory and file can be created
    if [ ! -d "$LOG_DIR" ]; then
        echo "ERROR: Log directory does not exist: $LOG_DIR" >&2
        return 1
    fi
    
    # Test file creation
    if ! touch "$logfile" 2>/dev/null; then
        echo "ERROR: Cannot create log file: $logfile" >&2
        echo "       Current directory: $(pwd)" >&2
        echo "       LOG_DIR permissions: $(ls -ld "$LOG_DIR" 2>/dev/null || echo 'Directory not accessible')" >&2
        return 1
    fi
    
    # CPU and Memory utilization
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        # Collect OS version information (only on first cycle)
        if [ "$counter" -eq 1 ]; then
            echo "=== SYSTEM INFORMATION ==="
            echo "Hostname: $(hostname)"
            echo "AIX Version: $(oslevel 2>/dev/null || echo 'N/A')"
            echo "AIX Service Pack: $(oslevel -s 2>/dev/null || echo 'N/A')"
            echo "System Details: $(uname -a 2>/dev/null || echo 'N/A')"
            echo "Hardware: $(uname -M 2>/dev/null || echo 'N/A')"
            echo ""
        fi
        
        echo "=== CPU UTILIZATION ==="
        vmstat 1 2 | tail -1
        
        # Power 10 specific: Enhanced CPU monitoring
        if command -v lparstat >/dev/null 2>&1; then
            echo ""
            echo "=== LPAR STATISTICS (Power 10 Enhanced) ==="
            lparstat 1 1 | tail -1
        fi
        echo ""
        
        echo "=== MEMORY UTILIZATION ==="
        svmon -G
        
        # Power 10 specific: Large page monitoring for Oracle
        if svmon -P >/dev/null 2>&1; then
            echo ""
            echo "=== LARGE PAGE UTILIZATION (Power 10) ==="
            svmon -P | head -10
        fi
        echo ""
        
        echo "=== DISK I/O STATISTICS ==="
        # AIX-compatible iostat command
        iostat -d 1 2 2>/dev/null | tail -n +3 || {
            echo "Using basic iostat for AIX:"
            iostat 1 2 2>/dev/null || echo "iostat not available"
        }
        echo ""
        
        echo "=== FILESYSTEM USAGE ==="
        # AIX df command - try -g first, fallback to -k
        df -g 2>/dev/null || {
            echo "Using df -k for AIX compatibility:"
            df -k
        }
        echo ""
        
        echo "=== NETWORK STATISTICS ==="
        netstat -i
        echo ""
        
        echo "=== PROCESS COUNT ==="
        ps -ef | wc -l
        echo ""
        
        echo "=== TOP PROCESSES BY CPU ==="
        # AIX-compatible process listing with CPU usage
        # Test if ps -eo works on this AIX version
        if ps -eo pid,ppid,user,pcpu,pmem,vsz,rss,comm 2>/dev/null | head -1 | grep -q "PID"; then
            ps -eo pid,ppid,user,pcpu,pmem,vsz,rss,comm 2>/dev/null | head -11
        else
            echo "Using AIX native ps format for CPU info:"
            ps aux 2>/dev/null | head -11 || ps -ef | head -11
        fi
        echo ""
        
        echo "=== TOP PROCESSES BY MEMORY ==="
        # AIX-compatible process listing with memory usage, sorted by RSS (Resident Set Size)
        # Test if ps -eo works on this AIX version
        if ps -eo pid,ppid,user,pcpu,pmem,vsz,rss,comm 2>/dev/null | head -1 | grep -q "PID"; then
            ps -eo pid,ppid,user,pcpu,pmem,vsz,rss,comm 2>/dev/null | tail -n +2 | sort -k 7 -n -r 2>/dev/null | head -11
        else
            echo "Using AIX native ps format for memory info:"
            # AIX sort compatibility: try -rn together, then fallback
            ps aux 2>/dev/null | head -12 | tail -n +2 | sort -k 4 -rn 2>/dev/null || \
            ps aux 2>/dev/null | head -12 | tail -n +2 || \
            ps -ef | head -11
        fi
        echo ""
        
    } >> "${LOG_DIR}/system_monitor_${TIMESTAMP}.log" 2>&1
}

# Function to collect Oracle-specific information
collect_oracle_info() {
    local timestamp=$1
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== ORACLE PROCESSES ==="
        ps -ef | grep -E "(ora_|oracle)" | grep -v grep
        echo ""
        
        echo "=== ORACLE MEMORY USAGE ==="
        # AIX-compatible Oracle process memory monitoring
        ps -eo pid,ppid,user,pcpu,pmem,vsz,rss,comm 2>/dev/null | grep -E "(ora_|oracle)" | grep -v grep || {
            echo "Using AIX fallback for Oracle process monitoring:"
            ps aux | grep -E "(ora_|oracle)" | grep -v grep || ps -ef | grep -E "(ora_|oracle)" | grep -v grep
        }
        echo ""
        
        echo "=== ORACLE DATABASE CONNECTIONS ==="
        if command -v sqlplus >/dev/null 2>&1; then
            # Only if Oracle is accessible
            echo "Checking Oracle sessions..."
        fi
        echo ""
        
    } >> "${LOG_DIR}/oracle_monitor_${TIMESTAMP}.log" 2>&1
}

# Function to collect Catalyst-specific information
collect_catalyst_info() {
    local timestamp=$1
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== CATALYST PROCESSES ==="
        ps -ef | grep -i catalyst | grep -v grep
        echo ""
        
        echo "=== RMAN PROCESSES ==="
        ps -ef | grep -i rman | grep -v grep
        echo ""
        
        echo "=== NETWORK CONNECTIONS TO STOREONCE ==="
        netstat -an | grep -E "(10\.|172\.|192\.)" | grep ESTABLISHED
        echo ""
        
    } >> "${LOG_DIR}/catalyst_monitor_${TIMESTAMP}.log" 2>&1
}

# Function to collect storage performance metrics
collect_storage_info() {
    local timestamp=$1
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== DETAILED DISK I/O STATISTICS ==="
        # AIX-compatible iostat with timestamps
        iostat -d -t 1 2 2>/dev/null || {
            echo "Using basic iostat for AIX:"
            iostat 1 2 2>/dev/null || echo "iostat not available"
        }
        echo ""
        
        echo "=== DISK USAGE BY MOUNT POINT ==="
        # AIX df compatibility - try -g first, fallback to -k
        df -g 2>/dev/null | grep -v "Filesystem" || {
            echo "Using df -k for AIX compatibility:"
            df -k | grep -v "Filesystem"
        }
        echo ""
        
        echo "=== INODE USAGE ==="
        df -i 2>/dev/null || echo "Inode information not available"
        echo ""
        
        echo "=== LARGE FILES (>1GB) ==="
        # AIX-compatible find command for large files
        find /oracle* /u0* /backup* -type f -size +1000000k 2>/dev/null | head -20 || {
            echo "Searching with basic find:"
            find / -type f -size +1000000k 2>/dev/null | grep -E "(oracle|u0|backup)" | head -10 2>/dev/null || echo "Large file search not available"
        }
        echo ""
        
        echo "=== FILE SYSTEM ACTIVITY ==="
        # Check if lsof is available, use AIX alternatives if not
        if command -v lsof >/dev/null 2>&1; then
            lsof +D /oracle* 2>/dev/null | wc -l 2>/dev/null || echo "0"
        else
            echo "lsof not available, using AIX fuser alternative:"
            fuser /oracle* 2>/dev/null | wc -w 2>/dev/null || echo "File system activity check not available"
        fi
        echo ""
        
    } >> "${LOG_DIR}/storage_monitor_${TIMESTAMP}.log" 2>&1
}

# Function to collect error logs
collect_error_logs() {
    local timestamp=$1
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== RECENT ERROR MESSAGES ==="
        errpt -d H -T PERM,TEMP,PERF,INFO | head -20
        echo ""
        
        echo "=== SYSTEM LOG ERRORS ==="
        # Check multiple AIX log sources
        if [ -f /var/adm/messages ]; then
            tail -50 /var/adm/messages 2>/dev/null | grep -i error
        fi
        
        # AIX syslog
        if [ -f /var/log/syslog ]; then
            tail -20 /var/log/syslog 2>/dev/null | grep -i error
        fi
        
        # Console messages (AIX specific)
        if [ -f /var/adm/sulog ]; then
            tail -10 /var/adm/sulog 2>/dev/null
        fi
        echo ""
        
        echo "=== KERNEL MESSAGES ==="
        # AIX alternative to dmesg - use errpt for kernel/system errors
        if command -v dmesg >/dev/null 2>&1; then
            dmesg | tail -20
        else
            echo "Using AIX errpt instead of dmesg:"
            errpt -a -N "console,SYSPROC,KERNEL" | head -20 2>/dev/null || echo "errpt kernel messages not available"
        fi
        echo ""
        
    } >> "${LOG_DIR}/error_monitor_${TIMESTAMP}.log" 2>&1
}

# Main monitoring loop
log_with_timestamp "Starting system monitoring for $DURATION minutes"

END_TIME=$(($(date +%s) + $DURATION * 60))
COUNTER=0

while [ $(date +%s) -lt $END_TIME ]; do
    CURRENT_TIME=$(date '+%Y-%m-%d %H:%M:%S')
    COUNTER=$((COUNTER + 1))
    
    log_with_timestamp "Collection cycle $COUNTER - $CURRENT_TIME"
    
    # Collect all monitoring data
    collect_system_info "$CURRENT_TIME" "$COUNTER"
    collect_oracle_info "$CURRENT_TIME"
    collect_catalyst_info "$CURRENT_TIME"
    collect_storage_info "$CURRENT_TIME"
    collect_error_logs "$CURRENT_TIME"
    
    # Wait for next interval
    sleep $INTERVAL
done

log_with_timestamp "Monitoring completed. Logs saved in: $LOG_DIR"

# Generate summary report
echo "=========================================="
echo "MONITORING SUMMARY"
echo "=========================================="
echo "Host: $HOSTNAME"
# AIX 7.2 compatible start time calculation
if command -v perl >/dev/null 2>&1; then
    START_TIME_STR=$(perl -e "use POSIX; print strftime('%Y-%m-%d %H:%M:%S', localtime(time - $DURATION * 60))")
    echo "Start Time: $START_TIME_STR"
else
    echo "Start Time: Approximately $DURATION minutes ago"
fi
echo "End Time: $(date)"
echo "Duration: $DURATION minutes"
echo "Total Collections: $COUNTER"
echo "Log Files Generated:"
ls -la "${LOG_DIR}"/*_${TIMESTAMP}.log
echo "=========================================="
