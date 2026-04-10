#!/usr/bin/ksh
#
# AIX Storage Performance Monitor
# Purpose: Deep monitoring of storage subsystem during backup operations
# Version: 2.0.2
# Date: July 2025
#

# Configuration
LOG_DIR=${1:-"/tmp/storage_monitoring"}
DURATION=${2:-60}  # Duration in minutes
INTERVAL=10        # Collection interval in seconds
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
HOSTNAME=$(hostname)
COMMAND_TIMEOUT=30 # Timeout for individual commands in seconds

# Safety check for very short durations (used in testing)
if [ "$DURATION" = "0.1" ] || [ "$DURATION" = "0.2" ] || [ "$DURATION" = "0.25" ]; then
    COMMAND_TIMEOUT=3
    INTERVAL=2
    echo "DEBUG: Short test mode detected - reducing timeouts to $COMMAND_TIMEOUT seconds, interval to $INTERVAL seconds"
    # For very short durations, disable some intensive operations
    export STORAGE_TEST_MODE=1
fi

# Signal handler for graceful shutdown
cleanup() {
    echo ""
    log_with_timestamp "Received interrupt signal - shutting down gracefully"
    # Kill any background processes (AIX compatible)
    JOBS=$(jobs -p)
    if [ -n "$JOBS" ]; then
        echo "$JOBS" | xargs kill 2>/dev/null
    fi
    log_with_timestamp "Storage monitoring terminated by user"
    exit 0
}

trap cleanup INT TERM

# Create log directory
mkdir -p "$LOG_DIR"

echo "=========================================="
echo "AIX Storage Performance Monitor Started"
echo "Host: $HOSTNAME"
echo "Start Time: $(date)"
echo "Duration: $DURATION minutes"
echo "Collection Interval: $INTERVAL seconds"
echo "Command Timeout: $COMMAND_TIMEOUT seconds"
echo "Log Directory: $LOG_DIR"
echo "=========================================="

# Function to run command with timeout and logging
run_with_timeout() {
    local timeout_duration="$1"
    local description="$2"
    shift 2
    local command="$*"
    
    # AIX 7.2+ enhanced validation
    if ! echo "$timeout_duration" | grep -q '^[0-9][0-9]*$' || [ "$timeout_duration" -lt 1 ]; then
        echo "$(date '+%Y-%m-%d %H:%M:%S') - ERROR: Invalid timeout duration '$timeout_duration' for $description"
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Using default timeout of 15 seconds"
        timeout_duration=15  # Reduced default timeout
    fi
    
    # Enhanced timeout command selection for AIX 7.2+
    local timeout_cmd=""
    local script_dir="$(dirname "$0")"
    
    # Prefer our enhanced timeout utility
    if [ -x "$script_dir/aix_enhanced_timeout.sh" ]; then
        timeout_cmd="$script_dir/aix_enhanced_timeout.sh"
    elif command -v timeout >/dev/null 2>&1; then
        timeout_cmd="timeout"
    elif [ -x "$script_dir/aix_timeout.sh" ]; then
        timeout_cmd="$script_dir/aix_timeout.sh"
    fi
    
    if [ -n "$timeout_cmd" ]; then
        if echo "$timeout_cmd" | grep -q "aix_enhanced_timeout.sh"; then
            # Use enhanced timeout with description
            $timeout_cmd "$timeout_duration" "$description" sh -c "$command"
        else
            # Use standard timeout
            $timeout_cmd "$timeout_duration" sh -c "$command"
        fi
        
        local exit_code=$?
        case $exit_code in
            0)
                return 0
                ;;
            124)
                echo "$(date '+%Y-%m-%d %H:%M:%S') - TIMEOUT: $description exceeded ${timeout_duration}s"
                return 124
                ;;
            *)
                echo "$(date '+%Y-%m-%d %H:%M:%S') - ERROR: $description failed with exit code $exit_code"
                return $exit_code
                ;;
        esac
    else
        # Fallback for systems without timeout command
        (
            eval "$command" &
            PID=$!
            (
                sleep "$timeout_duration"
                if kill -0 "$PID" 2>/dev/null; then
                    kill -TERM "$PID" 2>/dev/null
                    sleep 2
                    kill -KILL "$PID" 2>/dev/null
                fi
            ) &
            KILLER_PID=$!
            wait $PID
            EXIT_CODE=$?
            kill $KILLER_PID 2>/dev/null
            exit $EXIT_CODE
        )
        local exit_code=$?
        if [ $exit_code -ne 0 ]; then
            echo "$(date '+%Y-%m-%d %H:%M:%S') - WARNING: $description may have timed out or failed"
            return $exit_code
        fi
    fi
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed: $description"
    return 0
}

# Function to collect detailed I/O statistics
collect_io_stats() {
    echo "DEBUG: ENTERED collect_io_stats function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/io_stats_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting I/O statistics collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== IOSTAT DETAILED OUTPUT ==="
        run_with_timeout 120 "iostat detailed" "iostat -d 1 2" 2>/dev/null || echo "iostat detailed failed or timed out"
        echo ""
        
        echo "=== IOSTAT EXTENDED OUTPUT ==="
        run_with_timeout 120 "iostat extended" "iostat -D 1 2" 2>/dev/null || echo "iostat extended failed or timed out"
        echo ""
        
        # Power 10 specific: Enhanced I/O monitoring
        echo "=== POWER 10 ENHANCED I/O MONITORING ==="
        # Check for NVMe devices (common on Power 10)
        NVME_DEVICES=$(lsdev -Cc disk | grep -i nvme | awk '{print $1}')
        if [ -n "$NVME_DEVICES" ]; then
            echo "--- NVMe Device Performance ---"
            for nvme_dev in $NVME_DEVICES; do
                echo "Device: $nvme_dev"
                run_with_timeout 15 "iostat for $nvme_dev" iostat -d "$nvme_dev" 1 1 2>/dev/null || echo "NVMe iostat failed"
            done
        else
            echo "No NVMe devices detected"
        fi
        echo ""
        
        echo "=== LVM STATISTICS ==="
        run_with_timeout 120 "lvmstat" "lvmstat -v" 2>/dev/null || echo "lvmstat failed or timed out"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed I/O statistics collection"
}

# Function to monitor filesystem performance
collect_filesystem_stats() {
    echo "DEBUG: ENTERED collect_filesystem_stats function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/filesystem_stats_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting filesystem statistics collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== FILESYSTEM USAGE ==="
        # AIX compatible df command - try -g first, fallback to -k
        if df -g >/dev/null 2>&1; then
            run_with_timeout "$COMMAND_TIMEOUT" "df -g" "df -g" 2>/dev/null || echo "df -g failed or timed out"
        else
            echo "df -g not supported, using df -k:"
            run_with_timeout "$COMMAND_TIMEOUT" "df -k" "df -k" 2>/dev/null || echo "df -k failed or timed out"
        fi
        echo ""
        
        echo "=== INODE USAGE ==="
        run_with_timeout "$COMMAND_TIMEOUT" "df -i" "df -i" 2>/dev/null || echo "df -i failed or timed out"
        echo ""
        
        echo "=== FILESYSTEM STATISTICS ==="
        df | grep -v "Filesystem" | awk '{print $7}' | grep -E "^/" | head -5 | while read fs; do
            echo "--- Filesystem: $fs ---"
            echo "$(date '+%Y-%m-%d %H:%M:%S') - Checking large files in $fs (limited search)"
            run_with_timeout 10 "find large files in $fs" find "$fs" -maxdepth 2 -type f -size +100M 2>/dev/null | head -5 || echo "find in $fs failed or timed out"
            echo ""
        done
        
        echo "=== MOUNT OPTIONS ==="
        run_with_timeout "$COMMAND_TIMEOUT" "mount options" "mount | grep -E \"(jfs|jfs2|nfs)\"" 2>/dev/null || echo "mount command failed or timed out"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed filesystem statistics collection"
}

# Function to monitor disk performance metrics
collect_disk_metrics() {
    echo "DEBUG: ENTERED collect_disk_metrics function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/disk_metrics_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting disk metrics collection"
    
    # AIX 7.2+ Enhanced timeout handling with individual command timeouts
    local INDIVIDUAL_TIMEOUT=5  # Reduced timeout per lsattr command
    local MAX_FUNCTION_TIME=30  # Maximum time for entire function
    local function_start=$(date +%s)
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== DISK QUEUE DEPTHS ==="
        
        # Get disk list with timeout protection
        local disk_list_timeout=10  # Timeout for disk listing
        echo "Getting disk list with ${disk_list_timeout}s timeout..."
        
        local disk_list
        if [ "${STORAGE_TEST_MODE:-0}" = "1" ]; then
            echo "Test mode: Limited disk metrics collection"
            # Test mode with enhanced timeout
            disk_list=$(run_with_timeout $disk_list_timeout "lsdev disk listing" lsdev -Cc disk 2>/dev/null | grep Available | grep -E "hdisk|vdisk|disk[0-9]" | head -3)
        else
            # Full mode with enhanced timeout
            disk_list=$(run_with_timeout $disk_list_timeout "lsdev disk listing" lsdev -Cc disk 2>/dev/null | grep Available | grep -E "hdisk|vdisk|disk[0-9]" | head -10)
        fi
        
        if [ -z "$disk_list" ]; then
            echo "No disk devices found or lsdev timed out"
        else
            echo "$disk_list" | while read disk_line; do
                # Check function-level timeout
                local current_time=$(date +%s)
                if [ $((current_time - function_start)) -gt $MAX_FUNCTION_TIME ]; then
                    echo "Function timeout reached, stopping disk collection"
                    break
                fi
                
                disk=$(echo $disk_line | awk '{print $1}')
                # Enhanced device validation
                if echo "$disk" | grep -qE "^(h|v)?disk[0-9]+$"; then
                    echo "--- Disk: $disk ---"
                    # Individual command with aggressive timeout
                    if ! run_with_timeout $INDIVIDUAL_TIMEOUT "lsattr for $disk" lsattr -El $disk 2>/dev/null | grep -E "(queue_depth|max_transfer)"; then
                        echo "lsattr for $disk failed or timed out after ${INDIVIDUAL_TIMEOUT}s"
                    fi
                else
                    echo "Skipping non-disk device: $disk"
                fi
            done
        fi
        echo ""
        
        echo "=== MULTIPATH STATUS ==="
        if command -v mpio >/dev/null 2>&1; then
            run_with_timeout $COMMAND_TIMEOUT "mpio status" lsdev -Cc mpio 2>/dev/null || echo "mpio status failed or timed out"
        else
            echo "mpio command not available"
        fi
        echo ""
        
        echo "=== LUN INFORMATION ==="
        if [ "${STORAGE_TEST_MODE:-0}" = "1" ]; then
            run_with_timeout 3 "disk LUN info" lsdev -Cc disk | grep Available | head -5 || echo "disk LUN info failed or timed out"
        else
            run_with_timeout $COMMAND_TIMEOUT "disk LUN info" lsdev -Cc disk | grep Available | head -20 || echo "disk LUN info failed or timed out"
        fi
        echo ""
        
        echo "=== DISK ERRORS ==="
        run_with_timeout $COMMAND_TIMEOUT "disk errors" errpt -d H -N "hdisk*" -T PERM,TEMP | head -20 2>/dev/null || echo "disk error check failed or timed out"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed disk metrics collection"
}

# Function to monitor storage pools and volume groups
collect_storage_pools() {
    echo "DEBUG: ENTERED collect_storage_pools function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/storage_pools_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting storage pools collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== VOLUME GROUPS ==="
        run_with_timeout $COMMAND_TIMEOUT "lsvg" lsvg 2>/dev/null || echo "lsvg failed or timed out"
        echo ""
        
        # Get volume groups but limit processing to avoid hanging
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Getting VG list (limited to 5 VGs)"
        VG_LIST=$(lsvg 2>/dev/null | head -5)
        if [ -n "$VG_LIST" ]; then
            echo "$VG_LIST" | while read vg; do
                if [ -n "$vg" ]; then
                    echo "--- Volume Group: $vg ---"
                    echo "$(date '+%Y-%m-%d %H:%M:%S') - Processing VG: $vg"
                    lsvg "$vg" 2>/dev/null || echo "lsvg $vg failed"
                    echo ""
                    
                    echo "--- Logical Volumes in $vg ---"
                    lsvg -l "$vg" 2>/dev/null | head -10 || echo "lsvg -l $vg failed"
                    echo ""
                    
                    echo "--- Physical Volumes in $vg ---"
                    lsvg -p "$vg" 2>/dev/null | head -10 || echo "lsvg -p $vg failed"
                    echo ""
                fi
            done
        fi
        
        echo "=== LOGICAL VOLUME DETAILS (SAMPLE) ==="
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Getting sample LV details (limited to 2 VGs)"
        # Simplified LV details collection - avoid nested timeout calls
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Getting sample LV details (limited to 2 VGs)"
        VG_SAMPLE=$(lsvg 2>/dev/null | head -2)
        if [ -n "$VG_SAMPLE" ]; then
            echo "$VG_SAMPLE" | while read vg; do 
                if [ -n "$vg" ]; then
                    echo "--- VG: $vg LVs ---"
                    # Simple lslv call without nested timeout
                    lsvg -l "$vg" 2>/dev/null | grep -v "LV NAME" | head -3 | while read lv_line; do 
                        lv=$(echo "$lv_line" | awk '{print $1}')
                        if [ -n "$lv" ]; then 
                            echo "LV: $lv"
                            lslv "$lv" 2>/dev/null | head -5 || echo "lslv $lv failed"
                            echo ""
                        fi
                    done
                fi
            done
        else
            echo "No volume groups found"
        fi
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed storage pools collection"
}

# Function to monitor NFS if applicable
collect_nfs_stats() {
    echo "DEBUG: ENTERED collect_nfs_stats function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/nfs_stats_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting NFS statistics collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== NFS MOUNTS ==="
        run_with_timeout $COMMAND_TIMEOUT "NFS mounts" mount | grep nfs 2>/dev/null || echo "No NFS mounts found or command failed"
        echo ""
        
        echo "=== NFS STATISTICS ==="
        run_with_timeout $COMMAND_TIMEOUT "nfsstat" nfsstat 2>/dev/null || echo "NFS not active or nfsstat not available"
        echo ""
        
        echo "=== NFS CLIENT INFO ==="
        run_with_timeout $COMMAND_TIMEOUT "showmount" showmount -e 2>/dev/null || echo "No NFS exports or showmount not available"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed NFS statistics collection"
}

# Function to collect storage error analysis
collect_storage_errors() {
    echo "DEBUG: ENTERED collect_storage_errors function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/storage_errors_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting storage errors collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== RECENT STORAGE ERRORS ==="
        # AIX 7.2 compatible: Generate timestamp for 1 hour ago
        if command -v perl >/dev/null 2>&1; then
            RECENT_TIME=$(perl -e 'use POSIX; print strftime("%m%d%H%M%y", localtime(time - 3600))')
        else
            # Fallback: use current time (less precise but safe)
            RECENT_TIME=$(date '+%m%d%H%M%y')
        fi
        run_with_timeout "$COMMAND_TIMEOUT" "recent storage errors" "errpt -d H -s \"$RECENT_TIME\" | grep -E \"(DISK|hdisk|ADAPTER|fcs|ent)\"" 2>/dev/null || echo "Recent storage error check failed or timed out"
        echo ""
        
        echo "=== DISK ERROR SUMMARY ==="
        run_with_timeout "$COMMAND_TIMEOUT" "disk error summary" "errpt -d H -N \"hdisk*\" | head -30" 2>/dev/null || echo "Disk error summary failed or timed out"
        echo ""
        
        echo "=== FC ADAPTER ERRORS ==="
        run_with_timeout "$COMMAND_TIMEOUT" "FC adapter errors" "errpt -d H -N \"fcs*\" | head -20" 2>/dev/null || echo "FC adapter error check failed or timed out"
        echo ""
        
        echo "=== SCSI ERRORS ==="
        # AIX compatible: Use same recent time calculation
        run_with_timeout "$COMMAND_TIMEOUT" "SCSI errors" "errpt -d H -s \"$RECENT_TIME\" | grep -i scsi" 2>/dev/null || echo "SCSI error check failed or timed out"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed storage errors collection"
}

# Function to monitor backup-specific storage areas
collect_backup_storage() {
    echo "DEBUG: ENTERED collect_backup_storage function"
    local timestamp="$1"
    local output_file="${LOG_DIR}/backup_storage_${TIMESTAMP}.log"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting backup storage collection"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        echo "=== ORACLE DATA DIRECTORIES ==="
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Checking Oracle data directories (limited search)"
        for dir in /oracle* /u0* /oradata*; do
            if [ -d "$dir" ]; then
                echo "--- Directory: $dir ---"
                echo "$(date '+%Y-%m-%d %H:%M:%S') - Processing directory: $dir"
                run_with_timeout 15 "du for $dir" du -sg "$dir" 2>/dev/null || echo "du for $dir failed or timed out"
                run_with_timeout 5 "ls for $dir" ls -la "$dir" 2>/dev/null | head -5 || echo "ls for $dir failed or timed out"
                echo ""
            fi
        done
        
        echo "=== BACKUP DIRECTORIES ==="
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Checking backup directories (limited search)"
        for dir in /backup* /rman* /export*; do
            if [ -d "$dir" ]; then
                echo "--- Directory: $dir ---"
                echo "$(date '+%Y-%m-%d %H:%M:%S') - Processing directory: $dir"
                run_with_timeout 15 "du for $dir" du -sg "$dir" 2>/dev/null || echo "du for $dir failed or timed out"
                run_with_timeout 5 "ls for $dir" ls -la "$dir" 2>/dev/null | head -5 || echo "ls for $dir failed or timed out"
                echo ""
            fi
        done
        
        echo "=== LARGE FILES (>500MB) - SAMPLE ==="
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Searching for large files (limited and shallow search)"
        run_with_timeout 20 "find large files" find /oracle* /u0* /backup* -maxdepth 1 -type f -size +500M 2>/dev/null | head -10 || echo "Large file search failed or timed out"
        echo ""
        
        echo "=== RECENTLY MODIFIED FILES - SAMPLE ==="
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Searching for recent files (limited and shallow search)"
        run_with_timeout 20 "find recent files" find /oracle* /u0* /backup* -maxdepth 1 -type f -mtime -1 2>/dev/null | head -10 || echo "Recent file search failed or timed out"
        echo ""
        
    } >> "$output_file" 2>&1
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed backup storage collection"
}

# Main monitoring loop
log_with_timestamp() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_with_timestamp "Starting storage monitoring for $DURATION minutes"

# Handle decimal duration properly using awk
END_TIME=$(awk "BEGIN {print int($(date +%s) + $DURATION * 60)}" 2>/dev/null)
if [ -z "$END_TIME" ]; then
    # Fallback for very basic awk or missing awk
    END_TIME=$(($(date +%s) + 60))  # Default to 1 minute
fi

log_with_timestamp "Monitoring will run until END_TIME=$END_TIME"
COUNTER=0

while [ $(date +%s) -lt $END_TIME ]; do
    CURRENT_TIME=$(date '+%Y-%m-%d %H:%M:%S')
    COUNTER=$((COUNTER + 1))
    
    log_with_timestamp "Storage collection cycle $COUNTER - $CURRENT_TIME"
    log_with_timestamp "Starting background collection processes..."
    echo "DEBUG: About to start background collection processes"
    
    # Collect all storage monitoring data with timeout protection
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting I/O stats collection in background..."
    echo "DEBUG: Starting collect_io_stats"
    (
        collect_io_stats "$CURRENT_TIME"
    ) &
    PID_IO=$!
    echo "DEBUG: collect_io_stats PID=$PID_IO"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting filesystem stats collection in background..."
    echo "DEBUG: Starting collect_filesystem_stats"
    (
        collect_filesystem_stats "$CURRENT_TIME"
    ) &
    PID_FS=$!
    echo "DEBUG: collect_filesystem_stats PID=$PID_FS"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting disk metrics collection in background..."
    echo "DEBUG: Starting collect_disk_metrics"
    (
        collect_disk_metrics "$CURRENT_TIME"
    ) &
    PID_DISK=$!
    echo "DEBUG: collect_disk_metrics PID=$PID_DISK"
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting storage pools collection in background..."
    (
        collect_storage_pools "$CURRENT_TIME"
    ) &
    PID_POOLS=$!
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting NFS stats collection in background..."
    (
        collect_nfs_stats "$CURRENT_TIME"
    ) &
    PID_NFS=$!
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting storage errors collection in background..."
    (
        collect_storage_errors "$CURRENT_TIME"
    ) &
    PID_ERRORS=$!
    
    echo "$(date '+%Y-%m-%d %H:%M:%S') - Starting backup storage collection in background..."
    (
        collect_backup_storage "$CURRENT_TIME"
    ) &
    PID_BACKUP=$!
    
    log_with_timestamp "Waiting for collection processes to complete (max ${COMMAND_TIMEOUT}s each)..."
    log_with_timestamp "Background PIDs: IO=$PID_IO FS=$PID_FS DISK=$PID_DISK POOLS=$PID_POOLS NFS=$PID_NFS ERRORS=$PID_ERRORS BACKUP=$PID_BACKUP"
    
    # AIX 7.2+ Enhanced timeout handling with aggressive monitoring
    WAIT_START=$(date +%s)
    MAX_WAIT=60  # Reduced to 60 seconds max wait for all processes
    WARN_THRESHOLD=30  # Warn after 30 seconds
    CHECK_INTERVAL=2   # Check every 2 seconds for faster response
    
    while [ $(($(date +%s) - WAIT_START)) -lt $MAX_WAIT ]; do
        # Check if all processes are done
        PROCESSES_RUNNING=0
        RUNNING_PIDS=""
        COMPLETED_PROCESSES=""
        
        for pid in $PID_IO $PID_FS $PID_DISK $PID_POOLS $PID_NFS $PID_ERRORS $PID_BACKUP; do
            if kill -0 "$pid" 2>/dev/null; then
                PROCESSES_RUNNING=$((PROCESSES_RUNNING + 1))
                RUNNING_PIDS="$RUNNING_PIDS $pid"
            else
                # Process completed
                case $pid in
                    $PID_IO) COMPLETED_PROCESSES="$COMPLETED_PROCESSES IO" ;;
                    $PID_FS) COMPLETED_PROCESSES="$COMPLETED_PROCESSES FS" ;;
                    $PID_DISK) COMPLETED_PROCESSES="$COMPLETED_PROCESSES DISK" ;;
                    $PID_POOLS) COMPLETED_PROCESSES="$COMPLETED_PROCESSES POOLS" ;;
                    $PID_NFS) COMPLETED_PROCESSES="$COMPLETED_PROCESSES NFS" ;;
                    $PID_ERRORS) COMPLETED_PROCESSES="$COMPLETED_PROCESSES ERRORS" ;;
                    $PID_BACKUP) COMPLETED_PROCESSES="$COMPLETED_PROCESSES BACKUP" ;;
                esac
            fi
        done
        
        if [ $PROCESSES_RUNNING -eq 0 ]; then
            log_with_timestamp "All collection processes completed successfully"
            break
        fi
        
        ELAPSED=$(($(date +%s) - WAIT_START))
        
        # Log completed processes
        if [ -n "$COMPLETED_PROCESSES" ]; then
            echo "$(date '+%Y-%m-%d %H:%M:%S') - Completed:$COMPLETED_PROCESSES"
        fi
        
        # Warning threshold
        if [ $ELAPSED -gt $WARN_THRESHOLD ] && [ $((ELAPSED % 10)) -eq 0 ]; then
            echo "$(date '+%Y-%m-%d %H:%M:%S') - WARNING: Still waiting for $PROCESSES_RUNNING processes (${ELAPSED}s elapsed) PIDs:$RUNNING_PIDS"
        fi
        
        sleep $CHECK_INTERVAL
    done
    
    # Kill any remaining processes
    KILLED_PROCESSES=0
    for pid in $PID_IO $PID_FS $PID_DISK $PID_POOLS $PID_NFS $PID_ERRORS $PID_BACKUP; do
        if kill -0 "$pid" 2>/dev/null; then
            log_with_timestamp "WARNING: Terminating hanging process $pid"
            kill -TERM "$pid" 2>/dev/null
            KILLED_PROCESSES=$((KILLED_PROCESSES + 1))
            sleep 1
            if kill -0 "$pid" 2>/dev/null; then
                log_with_timestamp "WARNING: Force killing process $pid"
                kill -KILL "$pid" 2>/dev/null
            fi
        fi
    done
    
    if [ $KILLED_PROCESSES -gt 0 ]; then
        log_with_timestamp "WARNING: Had to terminate $KILLED_PROCESSES hanging processes"
    fi
    
    log_with_timestamp "Collection cycle $COUNTER completed. Waiting ${INTERVAL}s for next cycle..."
    
    # Wait for next interval, but break early if we're near the end time
    INTERVAL_END=$(($(date +%s) + INTERVAL))
    while [ $(date +%s) -lt $INTERVAL_END ] && [ $(date +%s) -lt $END_TIME ]; do
        sleep 1
    done
done

log_with_timestamp "Storage monitoring completed. Logs saved in: $LOG_DIR"

# Final cleanup
log_with_timestamp "Performing final cleanup..."

# Generate summary report
echo "=========================================="
echo "STORAGE MONITORING SUMMARY"
echo "=========================================="
echo "Host: $HOSTNAME"
echo "Start Time: $(awk "BEGIN {print strftime(\"%Y-%m-%d %H:%M:%S\", systime() - $DURATION * 60)}" 2>/dev/null || echo 'Start time calculation not available')"
echo "End Time: $(date)"
echo "Duration: $DURATION minutes"
echo "Total Collections: $COUNTER"
echo "Command Timeout: $COMMAND_TIMEOUT seconds"
echo "Collection Interval: $INTERVAL seconds"
echo ""
echo "Log Files Generated:"
ls -la "${LOG_DIR}"/*_${TIMESTAMP}.log 2>/dev/null || echo "No log files found"
echo "=========================================="
echo "Storage monitoring completed successfully!"
