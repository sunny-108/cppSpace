#!/usr/bin/ksh
#
# Real-time Issue Detection Script for AIX Oracle/StoreOnce Environment
# Purpose: Continuously monitor for backup failure indicators in real-time
# Version: 2.0
# Date: July 2025
#

# Configuration
LOG_DIR=${1:-"/tmp/realtime_monitoring"}
ALERT_THRESHOLD_CPU=85
ALERT_THRESHOLD_IO_WAIT=30
ALERT_THRESHOLD_MEMORY=90
CHECK_INTERVAL=60  # Check every 60 seconds
HOSTNAME=$(hostname)
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Create log directory
mkdir -p "$LOG_DIR"

# Alert log file
ALERT_LOG="${LOG_DIR}/alerts_${TIMESTAMP}.log"

echo "=========================================="
echo "Real-time Issue Detection Started"
echo "Host: $HOSTNAME"
echo "Start Time: $(date)"
echo "Alert Log: $ALERT_LOG"
echo "=========================================="

# Function to log alerts
log_alert() {
    local severity="$1"
    local message="$2"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    echo "[$timestamp] [$severity] $message" | tee -a "$ALERT_LOG"
    
    # Also log to system if critical
    if [ "$severity" = "CRITICAL" ]; then
        logger -p user.crit "AIX_MONITOR: $message"
    fi
}

# Function to check CPU usage
check_cpu_usage() {
    # AIX-compatible CPU usage calculation
    local cpu_idle=$(vmstat 1 2 | tail -1 | awk '{print $16}')
    local cpu_usage=$((100 - cpu_idle))
    
    if [ $cpu_usage -gt $ALERT_THRESHOLD_CPU ]; then
        log_alert "WARNING" "High CPU usage detected: ${cpu_usage}%"
        
        # Get top CPU processes (AIX ps command)
        echo "Top CPU processes at $(date):" >> "$ALERT_LOG"
        ps -ef | awk '{print $2, $3, $1, $8}' | head -6 | tail -5 >> "$ALERT_LOG"
        echo "" >> "$ALERT_LOG"
    fi
}

# Function to check I/O wait
check_io_wait() {
    local io_wait=$(vmstat 1 2 | tail -1 | awk '{print $17}')
    
    if [ $io_wait -gt $ALERT_THRESHOLD_IO_WAIT ]; then
        log_alert "WARNING" "High I/O wait detected: ${io_wait}%"
        
        # Get I/O statistics (AIX iostat)
        echo "I/O statistics at $(date):" >> "$ALERT_LOG"
        if command -v iostat >/dev/null 2>&1; then
            iostat -d 1 1 | grep hdisk >> "$ALERT_LOG" 2>/dev/null || echo "  iostat data unavailable" >> "$ALERT_LOG"
        else
            echo "  iostat command not available" >> "$ALERT_LOG"
        fi
        echo "" >> "$ALERT_LOG"
    fi
}

# Function to check memory usage
check_memory_usage() {
    local memory_info=$(svmon -G | grep memory)
    local memory_used=$(echo "$memory_info" | awk '{print $3}')
    local memory_total=$(echo "$memory_info" | awk '{print $2}')
    
    if [ $memory_total -gt 0 ]; then
        local memory_percent=$((memory_used * 100 / memory_total))
        
        if [ $memory_percent -gt $ALERT_THRESHOLD_MEMORY ]; then
            log_alert "WARNING" "High memory usage detected: ${memory_percent}%"
            
            # Get top memory processes (AIX ps command)
            echo "Top memory processes at $(date):" >> "$ALERT_LOG"
            ps -ef | awk '{print $2, $1, $8}' | head -6 | tail -5 >> "$ALERT_LOG"
            echo "" >> "$ALERT_LOG"
        fi
    fi
}

# Function to check Oracle RMAN status
check_rman_status() {
    # Check for stuck RMAN processes
    local rman_processes=$(ps -ef | grep -i rman | grep -v grep | wc -l)
    
    if [ $rman_processes -gt 0 ]; then
        # Check if any RMAN process is running for too long (simplified for AIX)
        local long_running=$(ps -ef | grep rman | grep -v grep)
        
        if [ -n "$long_running" ]; then
            log_alert "INFO" "RMAN processes currently active: $rman_processes"
            echo "RMAN processes at $(date):" >> "$ALERT_LOG"
            echo "$long_running" >> "$ALERT_LOG"
            echo "" >> "$ALERT_LOG"
        fi
    fi
}

# Function to check storage errors
check_storage_errors() {
    # Check for recent storage errors (last 5 minutes)
    # AIX 7.2 compatible: Use perl for timestamp calculation
    if command -v perl >/dev/null 2>&1; then
        RECENT_TIME=$(perl -e 'use POSIX; print strftime("%m%d%H%M%y", localtime(time - 300))')
    else
        # Fallback: use current time
        RECENT_TIME=$(date '+%m%d%H%M%y')
    fi
    local recent_errors=$(errpt -d H -s "$RECENT_TIME" 2>/dev/null | grep -E "DISK|hdisk" | wc -l)
    
    if [ $recent_errors -gt 0 ]; then
        log_alert "CRITICAL" "Storage errors detected in last 5 minutes: $recent_errors errors"
        
        echo "Recent storage errors at $(date):" >> "$ALERT_LOG"
        errpt -d H -s "$RECENT_TIME" 2>/dev/null | grep -E "DISK|hdisk" | head -5 >> "$ALERT_LOG"
        echo "" >> "$ALERT_LOG"
    fi
}

# Function to check network connectivity to StoreOnce
check_storeonce_connectivity() {
    # Discover active StoreOnce connections (AIX-compatible)
    local storeonce_ips=$(netstat -an | grep ":938[789]" | grep ESTABLISHED | awk '{print $5}' | sed 's/\.[0-9]*$//' | sort -u 2>/dev/null)
    
    if [ -n "$storeonce_ips" ]; then
        for ip in $storeonce_ips; do
            # Quick ping test (AIX ping syntax)
            if ! ping -c 1 "$ip" >/dev/null 2>&1; then
                log_alert "CRITICAL" "StoreOnce connectivity lost to $ip"
            fi
        done
    fi
    
    # Check for connection issues (simplified for AIX)
    local active_connections=$(netstat -an | grep ESTABLISHED | wc -l)
    if [ $active_connections -lt 5 ]; then
        log_alert "WARNING" "Low number of active network connections: $active_connections"
    fi
}

# Function to check file system space
check_filesystem_space() {
    # Check for file systems over 90% full (AIX df compatibility)
    local full_filesystems=$(df | awk 'NR>1 && $4 ~ /%/ {gsub(/%/, "", $4); if ($4 > 90) print $7, $4"%"}')
    
    if [ -n "$full_filesystems" ]; then
        log_alert "CRITICAL" "File systems critically full (>90%)"
        echo "Full file systems at $(date):" >> "$ALERT_LOG"
        echo "$full_filesystems" >> "$ALERT_LOG"
        echo "" >> "$ALERT_LOG"
    fi
    
    # Check for file systems over 80% full
    local almost_full=$(df | awk 'NR>1 && $4 ~ /%/ {gsub(/%/, "", $4); if ($4 > 80 && $4 <= 90) print $7, $4"%"}')
    
    if [ -n "$almost_full" ]; then
        log_alert "WARNING" "File systems approaching full (>80%)"
        echo "Nearly full file systems at $(date):" >> "$ALERT_LOG"
        echo "$almost_full" >> "$ALERT_LOG"
        echo "" >> "$ALERT_LOG"
    fi
}

# Function to check for backup piece timeouts
check_backup_timeouts() {
    # Look for backup pieces that haven't been updated in last hour
    local backup_dirs="/backup /rman /oracle/backup"
    
    for dir in $backup_dirs; do
        if [ -d "$dir" ]; then
            local stale_files=$(find "$dir" -name "*.bkp" -o -name "*.dbf" -mtime +0.04 2>/dev/null | wc -l)
            
            if [ $stale_files -gt 0 ]; then
                log_alert "WARNING" "Potential stale backup files in $dir: $stale_files files"
            fi
        fi
    done
}

# Function to generate status summary
generate_status_summary() {
    local summary_file="${LOG_DIR}/status_summary_${TIMESTAMP}.log"
    
    {
        echo "=== SYSTEM STATUS SUMMARY at $(date) ==="
        echo ""
        
        echo "CPU Usage:"
        vmstat 1 1 | tail -1 | awk '{print "  User: "$14"%, System: "$15"%, Idle: "$16"%, I/O Wait: "$17"%"}'
        echo ""
        
        echo "Memory Usage:"
        svmon -G | grep memory | awk '{printf "  Used: %.1f%%, Free: %.1f%%\n", ($3/$2)*100, (($2-$3)/$2)*100}'
        echo ""
        
        echo "Top 5 Processes by CPU:"
        ps -ef | awk '{print $2, $1, $8}' | head -6 | tail -5
        echo ""
        
        echo "Active RMAN Sessions:"
        ps -ef | grep -i rman | grep -v grep | wc -l
        echo ""
        
        echo "StoreOnce Connections:"
        netstat -an | grep ":938[789]" | grep ESTABLISHED | wc -l 2>/dev/null || echo "  0"
        echo ""
        
        echo "Recent Alerts (last 10):"
        tail -10 "$ALERT_LOG" 2>/dev/null || echo "  No alerts"
        echo ""
        
    } > "$summary_file"
    
    echo "$summary_file"
}

# Main monitoring loop
log_alert "INFO" "Real-time monitoring started"

# Trap to handle script termination
trap 'log_alert "INFO" "Real-time monitoring stopped"; exit 0' INT TERM

# Counter for periodic summaries
SUMMARY_COUNTER=0

while true; do
    # Perform all checks
    check_cpu_usage
    check_io_wait
    check_memory_usage
    check_rman_status
    check_storage_errors
    check_storeonce_connectivity
    check_filesystem_space
    check_backup_timeouts
    
    # Generate summary every 10 minutes
    SUMMARY_COUNTER=$((SUMMARY_COUNTER + 1))
    if [ $((SUMMARY_COUNTER % 10)) -eq 0 ]; then
        SUMMARY_FILE=$(generate_status_summary)
        log_alert "INFO" "Status summary generated: $SUMMARY_FILE"
    fi
    
    # Wait for next check
    sleep $CHECK_INTERVAL
done
