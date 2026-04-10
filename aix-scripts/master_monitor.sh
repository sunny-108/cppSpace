#!/usr/bin/ksh
#
# Master Monitoring Script for AIX Oracle/StoreOnce Environment
# Purpose: Orchestrate all monitoring scripts for comprehensive analysis
# Version: 2.1.1
# Date: July 2025
#

# Configuration
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
BASE_LOG_DIR="${1:-/tmp/monitoring_$(date +%Y%m%d_%H%M%S)}"
DURATION="${2:-60}"  # Duration in minutes
STOREONCE_IPS="${3:-""}"  # Space-separated StoreOnce IPs

# Create base log directory and resolve its absolute path
mkdir -p "$BASE_LOG_DIR"
BASE_LOG_DIR=$(cd "$BASE_LOG_DIR" && pwd)

echo "=========================================="
echo "AIX Oracle/StoreOnce Monitoring Suite"
echo "=========================================="
echo "Host: $(hostname)"
echo "Start Time: $(date)"
echo "Duration: $DURATION minutes"
echo "Base Log Directory: $BASE_LOG_DIR"
echo "StoreOnce IPs: ${STOREONCE_IPS:-'Auto-discover'}"
echo "=========================================="

# Function to log messages
log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

# Function to run monitoring script in background
run_monitor() {
    local script_name="$1"
    local log_subdir="$2"
    local extra_args="$3"
    
    local script_path="${SCRIPT_DIR}/${script_name}"
    local script_log_dir="${BASE_LOG_DIR}/${log_subdir}"
    
    if [ -f "$script_path" ]; then
        log_message "Starting $script_name..."
        mkdir -p "$script_log_dir"
        
        # Make script executable
        chmod +x "$script_path"
        
        # Run the script with appropriate arguments
        case "$script_name" in
            "aix_system_monitor.sh")
                "$script_path" "$DURATION" "$script_log_dir" > "${script_log_dir}/execution.log" 2>&1 &
                ;;
            "oracle_rman_monitor.sh")
                chown oracle:oinstall "$script_log_dir"
                su - oracle -c "'$script_path' '$script_log_dir'" > "${script_log_dir}/execution.log" 2>&1 &
                ;;
            "storage_monitor.sh")
                "$script_path" "$script_log_dir" "$DURATION" > "${script_log_dir}/execution.log" 2>&1 &
                ;;
            "network_monitor.sh")
                "$script_path" "$script_log_dir" "$STOREONCE_IPS" "$DURATION" > "${script_log_dir}/execution.log" 2>&1 &
                ;;
            *)
                log_message "Unknown script: $script_name"
                return 1
                ;;
        esac
        
        echo $! > "${script_log_dir}/pid"
        log_message "$script_name started with PID $(cat ${script_log_dir}/pid)"
    else
        log_message "WARNING: Script $script_path not found"
    fi
}

# Function to check if all scripts are still running
check_running_scripts() {
    local still_running=0
    
    for subdir in system oracle storage network; do
        local pid_file="${BASE_LOG_DIR}/${subdir}/pid"
        if [ -f "$pid_file" ]; then
            local pid=$(cat "$pid_file")
            # Use AIX-compatible ps command
            if ps -ef | awk '{print $2}' | grep -q "^${pid}$"; then
                still_running=1
            fi
        fi
    done
    
    return $still_running
}

# Function to generate comprehensive report
generate_summary_report() {
    local report_file="${BASE_LOG_DIR}/monitoring_summary_report.txt"
    
    {
        echo "=========================================="
        echo "AIX ORACLE/STOREONCE MONITORING REPORT"
        echo "=========================================="
        echo "Generated: $(date)"
        echo "Host: $(hostname)"
        echo "Monitoring Duration: $DURATION minutes"
        echo ""
        
        echo "=== MONITORING COMPONENTS ==="
        for component in system oracle storage network; do
            echo "--- $component monitoring ---"
            local comp_dir="${BASE_LOG_DIR}/${component}"
            if [ -d "$comp_dir" ]; then
                echo "Status: Completed"
                echo "Log files:"
                ls -la "$comp_dir"/*.log 2>/dev/null | wc -l
                # Use AIX-compatible du command
                if command -v du >/dev/null 2>&1; then
                    echo "Total log size: $(du -sk $comp_dir 2>/dev/null | awk '{print $1"KB"}')"
                else
                    echo "Total log size: Unable to determine"
                fi
            else
                echo "Status: Not executed"
            fi
            echo ""
        done
        
        echo "=== KEY FINDINGS SUMMARY ==="
        
        # System monitoring findings
        if [ -f "${BASE_LOG_DIR}/system/system_monitor_"*.log ]; then
            echo "--- System Performance ---"
            echo "CPU utilization peaks:"
            grep -h "us.*sy.*id" "${BASE_LOG_DIR}/system/system_monitor_"*.log | tail -5
            echo ""
        fi
        
        # Storage monitoring findings
        if [ -f "${BASE_LOG_DIR}/storage/io_stats_"*.log ]; then
            echo "--- Storage Performance ---"
            echo "I/O statistics summary:"
            grep -h "hdisk" "${BASE_LOG_DIR}/storage/io_stats_"*.log | tail -10
            echo ""
        fi
        
        # Network monitoring findings
        if [ -f "${BASE_LOG_DIR}/network/storeonce_connections_"*.log ]; then
            echo "--- Network Connectivity ---"
            echo "StoreOnce connection summary:"
            grep -h "ESTABLISHED" "${BASE_LOG_DIR}/network/storeonce_connections_"*.log | tail -5
            echo ""
        fi
        
        # Oracle monitoring findings
        if [ -f "${BASE_LOG_DIR}/oracle/rman_sessions_"*.log ]; then
            echo "--- Oracle RMAN Performance ---"
            echo "Active RMAN sessions:"
            grep -h "rman" "${BASE_LOG_DIR}/oracle/rman_sessions_"*.log | wc -l
            echo ""
        fi
        
        echo "=== RECOMMENDATIONS ==="
        echo "1. Review storage I/O patterns for bottlenecks"
        echo "2. Check network connectivity stability to StoreOnce"
        echo "3. Monitor Oracle RMAN session performance"
        echo "4. Analyze error logs for recurring issues"
        echo "5. Correlate high CPU/memory usage with backup timeouts"
        echo ""
        
        echo "=== LOG FILE LOCATIONS ==="
        echo "Base directory: $BASE_LOG_DIR"
        echo "System logs: ${BASE_LOG_DIR}/system/"
        echo "Oracle logs: ${BASE_LOG_DIR}/oracle/"
        echo "Storage logs: ${BASE_LOG_DIR}/storage/"
        echo "Network logs: ${BASE_LOG_DIR}/network/"
        echo ""
        
        echo "=== TOTAL DISK USAGE ==="
        # Use AIX-compatible du command
        if command -v du >/dev/null 2>&1; then
            du -sk "$BASE_LOG_DIR" | awk '{print $1"KB total"}'
        else
            echo "Unable to determine disk usage"
        fi
        echo ""
        
    } > "$report_file"
    
    echo "$report_file"
}

# Function to create analysis scripts
create_analysis_script() {
    local analysis_script="${BASE_LOG_DIR}/analyze_logs.sh"
    
    cat > "$analysis_script" << 'EOF'
#!/usr/bin/ksh
#
# Log Analysis Script
# Purpose: Analyze collected monitoring data for patterns and issues
#

BASE_DIR="$(dirname "$0")"

echo "Analyzing monitoring data in: $BASE_DIR"
echo "========================================"

# Analyze system performance patterns
if [ -d "$BASE_DIR/system" ]; then
    echo "=== System Performance Analysis ==="
    
    # CPU usage patterns
    echo "High CPU usage periods:"
    grep -h "TIMESTAMP\|us.*sy.*id" "$BASE_DIR/system/"*.log | \
    awk '/TIMESTAMP/{ts=$3" "$4} /us.*sy.*id/{cpu=100-$6; if(cpu>80) print ts" - CPU: "cpu"%"}' | head -10
    
    # Memory usage trends
    echo -e "\nMemory usage analysis:"
    grep -h "memory.*in use" "$BASE_DIR/system/"*.log | tail -5
    
    echo ""
fi

# Analyze storage performance
if [ -d "$BASE_DIR/storage" ]; then
    echo "=== Storage Performance Analysis ==="
    
    # I/O wait patterns
    echo "High I/O wait periods:"
    grep -h "TIMESTAMP\|hdisk.*%wa" "$BASE_DIR/storage/"*.log | \
    awk '/TIMESTAMP/{ts=$3" "$4} /%wa/{if($5>20) print ts" - I/O Wait: "$5"%"}' | head -10
    
    # Storage errors
    echo -e "\nStorage errors detected:"
    grep -i "error\|fail" "$BASE_DIR/storage/"*.log | head -5
    
    echo ""
fi

# Analyze network connectivity
if [ -d "$BASE_DIR/network" ]; then
    echo "=== Network Analysis ==="
    
    # Connection stability
    echo "Connection state changes:"
    grep -h "CONNECTION COUNTS" -A 5 "$BASE_DIR/network/"*.log | tail -10
    
    # Network errors
    echo -e "\nNetwork errors:"
    grep -i "error\|timeout\|fail" "$BASE_DIR/network/"*.log | head -5
    
    echo ""
fi

# Analyze Oracle performance
if [ -d "$BASE_DIR/oracle" ]; then
    echo "=== Oracle Performance Analysis ==="
    
    # RMAN session patterns
    echo "RMAN backup performance:"
    grep -h "mb_per_sec\|percent_complete" "$BASE_DIR/oracle/"*.log | head -10
    
    # Wait events
    echo -e "\nTop wait events:"
    grep -h "time_waited" "$BASE_DIR/oracle/"*.log | sort -k3 -nr | head -5
    
    echo ""
fi

echo "Analysis completed. Review the output above for patterns and issues."
EOF

    chmod +x "$analysis_script"
    echo "$analysis_script"
}

# Main execution starts here
log_message "Starting comprehensive monitoring suite..."

# Start all monitoring scripts
run_monitor "aix_system_monitor.sh" "system"
run_monitor "oracle_rman_monitor.sh" "oracle"
run_monitor "storage_monitor.sh" "storage"
run_monitor "network_monitor.sh" "network"

log_message "All monitoring scripts started"

# Create initial info file
{
    echo "Monitoring started at: $(date)"
    echo "Duration: $DURATION minutes"
    echo "Host: $(hostname)"
    echo "Oracle SID: ${ORACLE_SID:-'Not set'}"
    echo "Oracle Home: ${ORACLE_HOME:-'Not set'}"
    echo "StoreOnce IPs: ${STOREONCE_IPS:-'Auto-discover'}"
    echo ""
    echo "Monitoring PIDs:"
    for subdir in system oracle storage network; do
        local pid_file="${BASE_LOG_DIR}/${subdir}/pid"
        if [ -f "$pid_file" ]; then
            echo "$subdir: $(cat $pid_file)"
        fi
    done
} > "${BASE_LOG_DIR}/monitoring_info.txt"

# Wait for monitoring to complete
log_message "Monitoring in progress..."
log_message "You can monitor progress in: $BASE_LOG_DIR"

# Sleep for most of the duration, then check periodically
SLEEP_TIME=$((DURATION * 60 - 120))  # Sleep until 2 minutes before end
if [ $SLEEP_TIME -gt 0 ]; then
    sleep $SLEEP_TIME
fi

# Check if scripts are still running
while check_running_scripts; do
    log_message "Monitoring scripts still running, waiting..."
    sleep 30
done

log_message "All monitoring scripts completed"

# Generate comprehensive report
REPORT_FILE=$(generate_summary_report)
log_message "Summary report generated: $REPORT_FILE"

# Create analysis script
ANALYSIS_SCRIPT=$(create_analysis_script)
log_message "Analysis script created: $ANALYSIS_SCRIPT"

# Final summary
echo "=========================================="
echo "MONITORING COMPLETED"
echo "=========================================="
echo "Total monitoring time: $DURATION minutes"
echo "All logs saved in: $BASE_LOG_DIR"
echo "Summary report: $REPORT_FILE"
echo "Analysis script: $ANALYSIS_SCRIPT"
echo ""
echo "To analyze the collected data, run:"
echo "  $ANALYSIS_SCRIPT"
echo ""
echo "Key files to review:"
echo "  - $REPORT_FILE (overall summary)"
echo "  - ${BASE_LOG_DIR}/system/ (system performance)"
echo "  - ${BASE_LOG_DIR}/storage/ (storage performance)"
echo "  - ${BASE_LOG_DIR}/network/ (network connectivity)"
echo "  - ${BASE_LOG_DIR}/oracle/ (Oracle/RMAN performance)"
echo "=========================================="
