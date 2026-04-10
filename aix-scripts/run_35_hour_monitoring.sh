#!/usr/bin/ksh
#
# 35-Hour Continuous Monitoring Execution Script
# Purpose: Run comprehensive AIX monitoring for 35 hours continuously
# Usage: ./run_35_hour_monitoring.sh [base_log_dir] [storeonce_ips]
# Version: 2.1.1
# AIX Compatible Version - Power 10 Optimized
#

# Configuration
DURATION_MINUTES=2100  # 35 hours = 35 * 60 = 2100 minutes
BASE_LOG_DIR="${1:-/tmp/monitoring_35h_$(date +%Y%m%d_%H%M%S)}"
STOREONCE_IPS="${2:-""}"  # Optional: space-separated StoreOnce IPs
SCRIPT_DIR="$(dirname "$0")"

# Power 10 Detection and Optimization
detect_power10() {
    if command -v prtconf >/dev/null 2>&1; then
        PROCESSOR_TYPE=$(prtconf | grep "Processor Type" | awk -F: '{print $2}' | sed 's/^ *//')
        if echo "$PROCESSOR_TYPE" | grep -i "power10" >/dev/null; then
            echo "true"
        else
            echo "false"
        fi
    else
        echo "false"
    fi
}

POWER10_SYSTEM=$(detect_power10)

# AIX Compatibility Functions
get_timestamp() {
    # AIX 7.2 TL0 SP2 optimized timestamp function
    # Try date +%s first (works on AIX 7.2)
    if date +%s >/dev/null 2>&1; then
        date +%s
    elif command -v perl >/dev/null 2>&1; then
        perl -e 'print time'
    else
        # Fallback: Manual calculation for older AIX systems
        echo $(( $(date +%Y) * 31536000 + $(date +%j) * 86400 + $(date +%H) * 3600 + $(date +%M) * 60 + $(date +%S) ))
    fi
}

calculate_percentage() {
    # Use awk for floating point arithmetic (always available on AIX)
    local numerator="$1"
    local denominator="$2"
    awk "BEGIN {printf \"%.1f\", $numerator * 100 / $denominator}"
}

aix_errpt_time() {
    # Generate AIX errpt compatible timestamp for "X hours ago"
    local hours_ago="$1"
    if command -v perl >/dev/null 2>&1; then
        perl -e "print POSIX::strftime('%m%d%H%M%y', localtime(time - $hours_ago * 3600))"
    else
        # Fallback: use current time (less precise but safe)
        date '+%m%d%H%M%y'
    fi
}

echo "=========================================="
echo "35-HOUR CONTINUOUS MONITORING"
if [ "$POWER10_SYSTEM" = "true" ]; then
    echo "POWER 10 OPTIMIZED VERSION"
fi
echo "=========================================="
echo "Host: $(hostname)"
echo "Start Time: $(date)"
echo "Duration: 35 hours (2100 minutes)"
echo "Expected End Time: $(echo "$(date) + 35 hours" | sed 's/+ 35 hours//')"  # AIX compatible
if [ "$POWER10_SYSTEM" = "true" ]; then
    echo "Hardware: Power 10 system detected"
    echo "Optimizations: Enhanced monitoring enabled"
fi
echo "Base Log Directory: $BASE_LOG_DIR"
echo "StoreOnce IPs: ${STOREONCE_IPS:-'Auto-discover'}"
echo "=========================================="

# Verify scripts exist
if [ ! -f "${SCRIPT_DIR}/master_monitor.sh" ]; then
    echo "ERROR: master_monitor.sh not found in $SCRIPT_DIR"
    exit 1
fi

# Create base directory
mkdir -p "$BASE_LOG_DIR"

# Function to log messages
log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "${BASE_LOG_DIR}/35h_execution.log"
}

# Function to extract version from a script file
extract_version() {
    local script_file="$1"
    local script_name=$(basename "$script_file")
    
    if [ ! -f "$script_file" ]; then
        echo "NOT FOUND"
        return 1
    fi
    
    # Extract version from comment section (look for "Version: X.X" or "version X.X")
    local version=$(grep -i "^#.*[Vv]ersion[: ]*" "$script_file" | head -1 | sed -e 's/^#//' -e 's/[Vv]ersion[: ]*//' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
    
    if [ -z "$version" ]; then
        echo "UNKNOWN"
    else
        echo "$version"
    fi
}

# Function to generate script versions log
generate_versions_log() {
    local versions_log="${BASE_LOG_DIR}/script_versions.log"
    
    {
        echo "=========================================="
        echo "AIX MONITORING SCRIPTS VERSION INFORMATION"
        echo "=========================================="
        echo "Generated: $(date)"
        echo "Host: $(hostname)"
        echo "Script Directory: $SCRIPT_DIR"
        echo "=========================================="
        echo ""
        echo "SCRIPT VERSIONS:"
        echo "----------------------------------------"
        
        # List of all monitoring scripts (space-separated for ksh compatibility)
        local scripts="run_35_hour_monitoring.sh master_monitor.sh aix_system_monitor.sh network_monitor.sh oracle_rman_monitor.sh storage_monitor.sh realtime_monitor.sh check_prerequisites.sh discover_storeonce.sh aix_timeout.sh quick_timeout_test.sh test_aix72_compatibility.sh test_power10_compatibility.sh"
        
        for script in $scripts; do
            local script_path="${SCRIPT_DIR}/${script}"
            local version=$(extract_version "$script_path")
            printf "%-40s : %s\n" "$script" "$version"
        done
        
        echo ""
        echo "=========================================="
        echo "END OF VERSION INFORMATION"
        echo "=========================================="
        
    } > "$versions_log"
    
    log_message "Script versions logged to: $versions_log"
    
    # Also display versions on console
    echo ""
    echo "Script Versions:"
    echo "----------------------------------------"
    local scripts="run_35_hour_monitoring.sh master_monitor.sh aix_system_monitor.sh network_monitor.sh oracle_rman_monitor.sh storage_monitor.sh realtime_monitor.sh check_prerequisites.sh discover_storeonce.sh aix_timeout.sh quick_timeout_test.sh test_aix72_compatibility.sh test_power10_compatibility.sh"
    for script in $scripts; do
        local script_path="${SCRIPT_DIR}/${script}"
        local version=$(extract_version "$script_path")
        printf "  %-38s : %s\n" "$script" "$version"
    done
    echo "----------------------------------------"
    echo ""
}

# Signal handler for graceful shutdown
cleanup() {
    log_message "Received interrupt signal - initiating graceful shutdown..."
    
    # Kill the master monitor process if it's running
    if [ -f "${BASE_LOG_DIR}/master_monitor.pid" ]; then
        MASTER_PID=$(cat "${BASE_LOG_DIR}/master_monitor.pid")
        # Use AIX-compatible ps command
        if ps -ef | awk '{print $2}' | grep -q "^${MASTER_PID}$"; then
            log_message "Stopping master monitor process (PID: $MASTER_PID)"
            kill -TERM "$MASTER_PID" 2>/dev/null
            sleep 10
            if ps -ef | awk '{print $2}' | grep -q "^${MASTER_PID}$"; then
                log_message "Force killing master monitor process"
                kill -KILL "$MASTER_PID" 2>/dev/null
            fi
        fi
    fi
    
    # Kill any remaining monitoring processes
    for subdir in system oracle storage network; do
        pid_file="${BASE_LOG_DIR}/${subdir}/pid"
        if [ -f "$pid_file" ]; then
            pid=$(cat "$pid_file" 2>/dev/null)
            # Use AIX-compatible ps command
            if [ -n "$pid" ] && ps -ef | awk '{print $2}' | grep -q "^${pid}$"; then
                log_message "Stopping $subdir monitor process (PID: $pid)"
                kill -TERM "$pid" 2>/dev/null
            fi
        fi
    done
    
    log_message "35-hour monitoring terminated by user"
    generate_final_report
    exit 0
}

trap cleanup INT TERM

# Function to monitor disk space
monitor_disk_space() {
    local log_dir="$1"
    local threshold=90  # Alert if over 90% full
    
    # AIX-compatible disk space calculation
    # Try df -g first (shows GB), then fall back to standard df
    local usage=""
    
    if command -v df >/dev/null 2>&1; then
        # Try df -g (AIX format showing GB)
        local df_output=$(df -g "$log_dir" 2>/dev/null | tail -1)
        if [ -n "$df_output" ]; then
            # Parse AIX df -g output: calculate percentage from used/total
            usage=$(echo "$df_output" | awk '{
                if (NF >= 4 && $2 > 0) {
                    used = $3; total = $2;
                    percent = (used * 100) / total;
                    printf "%.0f", percent;
                } else {
                    print "0";
                }
            }')
        fi
        
        # Fallback: try standard df with percentage column
        if [ -z "$usage" ] || [ "$usage" = "0" ]; then
            df_output=$(df "$log_dir" 2>/dev/null | tail -1)
            if [ -n "$df_output" ]; then
                # Look for percentage column (usually has % symbol)
                usage=$(echo "$df_output" | awk '{
                    for(i=1; i<=NF; i++) {
                        if($i ~ /%$/) {
                            gsub(/%/, "", $i);
                            print $i;
                            exit;
                        }
                    }
                    print "0";
                }')
            fi
        fi
    fi
    
    # Validate the usage value
    if [ -z "$usage" ] || ! echo "$usage" | grep -E '^[0-9]+$' >/dev/null; then
        log_message "WARNING: Could not determine disk usage for $log_dir"
        return 1
    fi
    
    # Check if usage exceeds threshold
    if [ "$usage" -gt "$threshold" ]; then
        log_message "WARNING: Log directory filesystem is ${usage}% full"
        log_message "Consider cleaning up old logs or extending filesystem"
    fi
}

# Function to generate periodic status reports
generate_status_report() {
    local hours_elapsed="$1"
    local status_file="${BASE_LOG_DIR}/status_${hours_elapsed}h.txt"
    
    {
        echo "=========================================="
        echo "35-HOUR MONITORING STATUS REPORT"
        echo "=========================================="
        echo "Report Time: $(date)"
        echo "Hours Elapsed: $hours_elapsed / 35"
        # Use awk for percentage calculation (AIX compatible)
        progress=$(awk "BEGIN {printf \"%.1f\", $hours_elapsed * 100 / 35}" 2>/dev/null || echo "N/A")
        echo "Progress: ${progress}%"
        echo ""
        
        echo "=== MONITORING PROCESS STATUS ==="
        for subdir in system oracle storage network; do
            pid_file="${BASE_LOG_DIR}/${subdir}/pid"
            if [ -f "$pid_file" ]; then
                pid=$(cat "$pid_file" 2>/dev/null)
                # Use AIX-compatible ps command
                if [ -n "$pid" ] && ps -ef | awk '{print $2}' | grep -q "^${pid}$"; then
                    echo "$subdir monitor: RUNNING (PID: $pid)"
                else
                    echo "$subdir monitor: STOPPED"
                fi
            else
                echo "$subdir monitor: NOT STARTED"
            fi
        done
        echo ""
        
        echo "=== LOG DIRECTORY STATUS ==="
        echo "Base Directory: $BASE_LOG_DIR"
        echo "Total Size: $(du -sh "$BASE_LOG_DIR" 2>/dev/null | awk '{print $1}')"
        echo "File Count: $(find "$BASE_LOG_DIR" -type f | wc -l)"
        echo "Disk Usage: $(df "$BASE_LOG_DIR" | tail -1 | awk '{print $5}')"
        echo ""
        
        echo "=== RECENT LOG ACTIVITY ==="
        echo "Last 5 modified files:"
        find "$BASE_LOG_DIR" -type f -name "*.log" -exec ls -lt {} + 2>/dev/null | head -5
        echo ""
        
        echo "=== SYSTEM RESOURCE STATUS ==="
        echo "CPU Load: $(uptime | awk -F'load average:' '{print $2}')"
        echo "Memory Usage: $(svmon -G | grep memory | awk '{printf "%.1f%% used\n", $3*100/$2}' 2>/dev/null || echo 'N/A')"
        echo "Available Disk Space:"
        df -g "$BASE_LOG_DIR" | tail -1 || df "$BASE_LOG_DIR" | tail -1
        echo ""
        
    } > "$status_file"
    
    log_message "Status report generated: $status_file"
    echo "$status_file"
}

# Function to generate final report
generate_final_report() {
    local final_report="${BASE_LOG_DIR}/FINAL_35H_REPORT.txt"
    
    {
        echo "=========================================="
        echo "35-HOUR MONITORING FINAL REPORT"
        echo "=========================================="
        echo "Execution Host: $(hostname)"
        echo "Start Time: $(head -1 "${BASE_LOG_DIR}/35h_execution.log" 2>/dev/null | cut -d' ' -f1-2 || echo 'Unknown')"
        echo "End Time: $(date)"
        echo "Planned Duration: 35 hours (2100 minutes)"
        
        # Calculate actual duration
        if [ -f "${BASE_LOG_DIR}/35h_execution.log" ]; then
            start_line=$(head -1 "${BASE_LOG_DIR}/35h_execution.log" 2>/dev/null)
            if [ -n "$start_line" ]; then
                start_time=$(echo "$start_line" | cut -d' ' -f1-2)
                echo "Actual Start: $start_time"
            fi
        fi
        
        echo ""
        echo "=== MONITORING COMPONENTS SUMMARY ==="
        for component in system oracle storage network; do
            comp_dir="${BASE_LOG_DIR}/${component}"
            if [ -d "$comp_dir" ]; then
                echo "--- $component monitoring ---"
                echo "Status: Executed"
                log_count=$(ls "$comp_dir"/*.log 2>/dev/null | wc -l)
                echo "Log files: $log_count"
                echo "Data size: $(du -sh "$comp_dir" 2>/dev/null | awk '{print $1}')"
                
                # Check if execution log exists
                if [ -f "${comp_dir}/execution.log" ]; then
                    # AIX-compatible: Use sed instead of grep -o to extract status
                    status_line=$(tail -1 "${comp_dir}/execution.log" 2>/dev/null)
                    if echo "$status_line" | grep "completed" >/dev/null 2>&1; then
                        echo "Execution status: completed"
                    elif echo "$status_line" | grep "failed" >/dev/null 2>&1; then
                        echo "Execution status: failed"
                    elif echo "$status_line" | grep "terminated" >/dev/null 2>&1; then
                        echo "Execution status: terminated"
                    else
                        echo "Execution status: Unknown"
                    fi
                fi
            else
                echo "--- $component monitoring ---"
                echo "Status: Not executed"
            fi
            echo ""
        done
        
        echo "=== TOTAL DATA COLLECTED ==="
        echo "Base Directory: $BASE_LOG_DIR"
        echo "Total Size: $(du -sh "$BASE_LOG_DIR" 2>/dev/null | awk '{print $1}')"
        echo "Total Files: $(find "$BASE_LOG_DIR" -type f | wc -l)"
        echo "Log Files: $(find "$BASE_LOG_DIR" -name "*.log" | wc -l)"
        echo ""
        
        echo "=== ANALYSIS RECOMMENDATIONS ==="
        echo "1. Use the analysis script: ${BASE_LOG_DIR}/analyze_logs.sh"
        echo "2. Review hourly status reports in: ${BASE_LOG_DIR}/status_*h.txt"
        echo "3. Check execution logs for any errors or warnings"
        echo "4. Correlate system performance with backup timeouts"
        echo "5. Analyze storage I/O patterns during peak hours"
        echo ""
        
        echo "=== AVAILABLE ANALYSIS TOOLS ==="
        if [ -f "${BASE_LOG_DIR}/analyze_logs.sh" ]; then
            echo "- Log analysis script: ${BASE_LOG_DIR}/analyze_logs.sh"
        fi
        if [ -f "${BASE_LOG_DIR}/monitoring_summary_report.txt" ]; then
            echo "- Summary report: ${BASE_LOG_DIR}/monitoring_summary_report.txt"
        fi
        echo "- Status reports: ${BASE_LOG_DIR}/status_*h.txt"
        echo "- Execution log: ${BASE_LOG_DIR}/35h_execution.log"
        if [ -f "${BASE_LOG_DIR}/script_versions.log" ]; then
            echo "- Script versions: ${BASE_LOG_DIR}/script_versions.log"
        fi
        echo ""
        
    } > "$final_report"
    
    log_message "Final report generated: $final_report"
}

# Main execution
log_message "Starting 35-hour continuous monitoring..."

# Generate script versions log
generate_versions_log

log_message "Master monitor will be called with duration: $DURATION_MINUTES minutes"

# Start the master monitor script
log_message "Executing: ${SCRIPT_DIR}/master_monitor.sh \"$BASE_LOG_DIR\" \"$DURATION_MINUTES\" \"$STOREONCE_IPS\""

# Run master monitor in background and capture PID
"${SCRIPT_DIR}/master_monitor.sh" "$BASE_LOG_DIR" "$DURATION_MINUTES" "$STOREONCE_IPS" &
MASTER_PID=$!
echo $MASTER_PID > "${BASE_LOG_DIR}/master_monitor.pid"

log_message "Master monitor started with PID: $MASTER_PID"

# Monitor the execution and generate periodic reports
HOUR_COUNTER=0
START_TIME=$(get_timestamp)  # AIX compatible timestamp
LAST_STATUS_TIME=$START_TIME
STATUS_INTERVAL=3600  # 1 hour = 3600 seconds

# Use AIX-compatible process checking
while ps -ef | awk '{print $2}' | grep -q "^${MASTER_PID}$"; do
    sleep 300  # Check every 5 minutes
    
    # Check if it's time for an hourly status report
    CURRENT_TIME=$(get_timestamp)  # AIX compatible
    if [ $((CURRENT_TIME - LAST_STATUS_TIME)) -ge $STATUS_INTERVAL ]; then
        HOUR_COUNTER=$((HOUR_COUNTER + 1))
        log_message "Generating $HOUR_COUNTER-hour status report..."
        generate_status_report "$HOUR_COUNTER"
        monitor_disk_space "$BASE_LOG_DIR"
        LAST_STATUS_TIME=$CURRENT_TIME
    fi
    
    # Log progress every 30 minutes
    MINUTES_ELAPSED=$(((CURRENT_TIME - START_TIME) / 60))
    if [ $((MINUTES_ELAPSED % 30)) -eq 0 ] && [ $MINUTES_ELAPSED -gt 0 ]; then
        # Use AIX compatible percentage calculation
        PERCENT_COMPLETE=$(calculate_percentage "$MINUTES_ELAPSED" "$DURATION_MINUTES")
        log_message "Progress: ${MINUTES_ELAPSED}/${DURATION_MINUTES} minutes (${PERCENT_COMPLETE}%)"
    fi
done

# Wait for master monitor to complete
wait $MASTER_PID
MASTER_EXIT_CODE=$?

log_message "Master monitor completed with exit code: $MASTER_EXIT_CODE"

# Generate final report
generate_final_report

# Final summary
echo "=========================================="
echo "35-HOUR MONITORING COMPLETED"
echo "=========================================="
echo "Host: $(hostname)"
echo "End Time: $(date)"
echo "Exit Code: $MASTER_EXIT_CODE"
echo "Total Data Size: $(du -sh "$BASE_LOG_DIR" 2>/dev/null | awk '{print $1}')"
echo ""
echo "Key Reports Generated:"
echo "  - Final Report: ${BASE_LOG_DIR}/FINAL_35H_REPORT.txt"
echo "  - Script Versions: ${BASE_LOG_DIR}/script_versions.log"
echo "  - Execution Log: ${BASE_LOG_DIR}/35h_execution.log"
echo "  - Hourly Status: ${BASE_LOG_DIR}/status_*h.txt"
if [ -f "${BASE_LOG_DIR}/monitoring_summary_report.txt" ]; then
    echo "  - Summary Report: ${BASE_LOG_DIR}/monitoring_summary_report.txt"
fi
if [ -f "${BASE_LOG_DIR}/analyze_logs.sh" ]; then
    echo "  - Analysis Script: ${BASE_LOG_DIR}/analyze_logs.sh"
fi
echo ""
echo "To analyze the collected data:"
if [ -f "${BASE_LOG_DIR}/analyze_logs.sh" ]; then
    echo "  ${BASE_LOG_DIR}/analyze_logs.sh"
fi
echo "  cat ${BASE_LOG_DIR}/FINAL_35H_REPORT.txt"
echo "=========================================="

log_message "35-hour monitoring execution completed"
