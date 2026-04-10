#!/usr/bin/ksh
#
# Oracle RMAN Backup Monitor for AIX
# Purpose: Monitor Oracle RMAN backup sessions and performance
# Version: 2.0.2
# Date: July 2025
#

# Configuration
LOG_DIR=${1:-"/tmp/oracle_monitoring"}

# Oracle environment detection for AIX
detect_oracle_home() {
    # 1. Check environment variable first
    if [ -n "$ORACLE_HOME" ] && [ -d "$ORACLE_HOME" ] && [ -x "$ORACLE_HOME/bin/sqlplus" ]; then
        return 0
    fi

    # 2. Check /etc/oratab file
    if [ -f "/etc/oratab" ]; then
        # Filter out comments and blank lines, get the ORACLE_HOME field
        # and check if it's a valid directory with sqlplus
        oracle_home_from_tab=$(grep -v '^#' /etc/oratab | grep -v '^$' | cut -d: -f2 | while read -r home; do
            if [ -d "$home" ] && [ -x "$home/bin/sqlplus" ]; then
                echo "$home"
                break
            fi
        done)
        
        if [ -n "$oracle_home_from_tab" ]; then
            ORACLE_HOME="$oracle_home_from_tab"
            export ORACLE_HOME
            return 0
        fi
    fi

    # 3. Check running processes for PMON
    pmon_process=$(ps -ef | grep "ora_pmon_" | grep -v "grep" | head -1)
    if [ -n "$pmon_process" ]; then
        # Extract ORACLE_SID from the process name (e.g., ora_pmon_ORCL)
        ORACLE_SID=$(echo "$pmon_process" | sed 's/.*ora_pmon_//')
        # Find the executable path, which can lead to ORACLE_HOME
        oracle_exe=$(whence "ora_pmon_${ORACLE_SID}" | awk '{print $1}')
        if [ -n "$oracle_exe" ] && [ -f "$oracle_exe" ]; then
            oracle_home_from_pmon=$(dirname "$(dirname "$oracle_exe")")
            if [ -d "$oracle_home_from_pmon" ] && [ -x "$oracle_home_from_pmon/bin/sqlplus" ]; then
                ORACLE_HOME="$oracle_home_from_pmon"
                export ORACLE_HOME
                return 0
            fi
        fi
    fi
    
    # 4. Fallback to common Oracle locations on AIX
    for oracle_path in \
        "/oracle/app/oracle/product/19c" \
        "/oracle/app/oracle/product/18c" \
        "/oracle/app/oracle/product/12c" \
        "/opt/oracle/product/19c" \
        "/opt/oracle/product/18c" \
        "/u01/app/oracle/product/19c" \
        "/u01/app/oracle/product/18c"; do
        if [ -d "$oracle_path" ] && [ -x "$oracle_path/bin/sqlplus" ]; then
            ORACLE_HOME="$oracle_path"
            export ORACLE_HOME
            return 0
        fi
    done
    
    return 1
}

# Detect Oracle Home
if ! detect_oracle_home; then
    echo "ERROR: Oracle Home not found or sqlplus not accessible"
    echo "Please set ORACLE_HOME environment variable"
    exit 1
fi

ORACLE_SID=${ORACLE_SID:-"ORCL"}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
HOSTNAME=$(hostname)

# Create log directory
mkdir -p "$LOG_DIR"

# --- DIAGNOSTICS START ---
DIAG_LOG="${LOG_DIR}/oracle-environment-info.log"
echo "--- Running Environment Diagnostics ---" > "$DIAG_LOG"
echo "Timestamp: $(date)" >> "$DIAG_LOG"
echo "Executing User: $(id)" >> "$DIAG_LOG"
echo "Initial ORACLE_HOME: '$ORACLE_HOME'" >> "$DIAG_LOG"
echo "Initial ORACLE_SID: '$ORACLE_SID'" >> "$DIAG_LOG"
echo "Initial PATH: '$PATH'" >> "$DIAG_LOG"
echo "-------------------------------------" >> "$DIAG_LOG"
# --- DIAGNOSTICS END ---

echo "=========================================="
echo "Oracle RMAN Backup Monitor Started"
echo "Host: $HOSTNAME"
echo "Oracle SID: $ORACLE_SID"
echo "Start Time: $(date)"
echo "Log Directory: $LOG_DIR"
echo "=========================================="

# Function to execute SQL and log results
execute_sql() {
    local sql_query="$1"
    local output_file="$2"
    local description="$3"
    
    echo "=== $(date '+%Y-%m-%d %H:%M:%S') - $description ===" >> "$output_file"
    
    # Verify Oracle Home and sqlplus accessibility
    if [ ! -x "$ORACLE_HOME/bin/sqlplus" ]; then
        echo "ERROR: sqlplus not found or not executable at $ORACLE_HOME/bin/sqlplus" >> "$output_file"
        return 1
    fi
    
    # Set Oracle environment for AIX
    export ORACLE_HOME
    export ORACLE_SID
    export PATH="$ORACLE_HOME/bin:$PATH"
    
    # Execute SQL with proper error handling
    {
        echo "set pagesize 1000;"
        echo "set linesize 200;"
        echo "set feedback off;"
        echo "set heading on;"
        echo "whenever sqlerror exit sql.sqlcode;"
        echo "$sql_query"
        echo "exit;"
    } | "$ORACLE_HOME/bin/sqlplus" -s / as sysdba >> "$output_file" 2>&1
    
    local exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo "WARNING: SQL execution failed with exit code $exit_code" >> "$output_file"
    fi
    
    echo "" >> "$output_file"
    return $exit_code
}

# Function to monitor RMAN sessions
monitor_rman_sessions() {
    local output_file="${LOG_DIR}/rman_sessions_${TIMESTAMP}.log"
    
    # Current RMAN sessions
    execute_sql "
    SELECT 
        s.sid,
        s.serial#,
        s.username,
        s.program,
        s.status,
        s.logon_time,
        s.last_call_et/3600 as hours_active,
        p.spid
    FROM v\$session s, v\$process p
    WHERE s.paddr = p.addr
    AND s.program LIKE '%rman%'
    ORDER BY s.logon_time;
    " "$output_file" "Active RMAN Sessions"
    
    # RMAN backup progress
    execute_sql "
    SELECT 
        sid,
        serial#,
        opname,
        target,
        sofar,
        totalwork,
        round(sofar/totalwork*100,2) as percent_complete,
        time_remaining,
        elapsed_seconds
    FROM v\$session_longops
    WHERE opname LIKE '%RMAN%'
    AND totalwork > 0
    ORDER BY start_time DESC;
    " "$output_file" "RMAN Backup Progress"
    
    # RMAN jobs from v$rman_status
    execute_sql "
    SELECT 
        session_recid,
        session_stamp,
        start_time,
        end_time,
        operation,
        status,
        object_type,
        mbytes_processed
    FROM v\$rman_status
    WHERE start_time > SYSDATE - 1
    ORDER BY start_time DESC;
    " "$output_file" "Recent RMAN Operations"
}

# Function to monitor backup performance
monitor_backup_performance() {
    local output_file="${LOG_DIR}/backup_performance_${TIMESTAMP}.log"
    
    # Backup piece information
    execute_sql "
    SELECT 
        handle,
        bytes/1024/1024 as size_mb,
        completion_time,
        elapsed_seconds,
        (bytes/1024/1024)/elapsed_seconds as mb_per_sec
    FROM v\$backup_piece
    WHERE completion_time > SYSDATE - 1
    ORDER BY completion_time DESC;
    " "$output_file" "Recent Backup Pieces Performance"
    
    # Backup set details
    execute_sql "
    SELECT 
        bs.set_stamp,
        bs.set_count,
        bs.backup_type,
        bs.completion_time,
        bs.elapsed_seconds,
        bp.bytes/1024/1024 as size_mb,
        bp.handle
    FROM v\$backup_set bs, v\$backup_piece bp
    WHERE bs.set_stamp = bp.set_stamp
    AND bs.set_count = bp.set_count
    AND bs.completion_time > SYSDATE - 1
    ORDER BY bs.completion_time DESC;
    " "$output_file" "Backup Set Details"
    
    # Channel statistics
    execute_sql "
    SELECT 
        session_recid,
        channel,
        bytes/1024/1024 as mb_processed,
        files,
        elapsed_seconds,
        effective_bytes_per_second/1024/1024 as effective_mb_per_sec
    FROM v\$backup_sync_io
    WHERE close_time > SYSDATE - 1
    ORDER BY close_time DESC;
    " "$output_file" "Channel Performance Statistics"
}

# Function to monitor database waits during backup
monitor_database_waits() {
    local output_file="${LOG_DIR}/database_waits_${TIMESTAMP}.log"
    
    # Current wait events
    execute_sql "
    SELECT 
        event,
        total_waits,
        total_timeouts,
        time_waited,
        average_wait
    FROM v\$system_event
    WHERE event LIKE '%backup%' OR event LIKE '%write%' OR event LIKE '%read%'
    ORDER BY time_waited DESC;
    " "$output_file" "Current Wait Events"
    
    # Top wait events for RMAN sessions
    execute_sql "
    SELECT 
        s.sid,
        s.serial#,
        s.program,
        sw.event,
        sw.total_waits,
        sw.time_waited,
        sw.average_wait
    FROM v\$session s, v\$session_event sw
    WHERE s.sid = sw.sid
    AND s.program LIKE '%rman%'
    AND sw.time_waited > 0
    ORDER BY sw.time_waited DESC;
    " "$output_file" "RMAN Session Wait Events"
    
    # I/O related waits
    execute_sql "
    SELECT 
        event,
        total_waits,
        time_waited/100 as time_waited_seconds,
        average_wait/100 as avg_wait_seconds
    FROM v\$system_event
    WHERE event IN ('db file sequential read', 'db file scattered read', 
                   'log file sync', 'log file parallel write',
                   'direct path read', 'direct path write')
    ORDER BY time_waited DESC;
    " "$output_file" "I/O Related Wait Events"
}

# Function to monitor tablespace and storage
monitor_storage_usage() {
    local output_file="${LOG_DIR}/storage_usage_${TIMESTAMP}.log"
    
    # Tablespace usage
    execute_sql "
    SELECT 
        df.tablespace_name,
        df.bytes/1024/1024/1024 as total_gb,
        (df.bytes - NVL(fs.bytes,0))/1024/1024/1024 as used_gb,
        NVL(fs.bytes,0)/1024/1024/1024 as free_gb,
        ROUND(((df.bytes - NVL(fs.bytes,0)) / df.bytes) * 100, 2) as used_percent
    FROM 
        (SELECT tablespace_name, SUM(bytes) bytes FROM dba_data_files GROUP BY tablespace_name) df,
        (SELECT tablespace_name, SUM(bytes) bytes FROM dba_free_space GROUP BY tablespace_name) fs
    WHERE df.tablespace_name = fs.tablespace_name(+)
    ORDER BY used_percent DESC;
    " "$output_file" "Tablespace Usage"
    
    # Temp tablespace usage
    execute_sql "
    SELECT 
        tablespace_name,
        total_mb,
        used_mb,
        free_mb,
        ROUND((used_mb/total_mb)*100,2) as used_percent
    FROM (
        SELECT 
            d.tablespace_name,
            d.total_bytes/1024/1024 as total_mb,
            NVL(t.used_bytes,0)/1024/1024 as used_mb,
            (d.total_bytes - NVL(t.used_bytes,0))/1024/1024 as free_mb
        FROM 
            (SELECT tablespace_name, SUM(bytes) total_bytes FROM dba_temp_files GROUP BY tablespace_name) d,
            (SELECT tablespace_name, SUM(bytes_used) used_bytes FROM v\$temp_extent_pool GROUP BY tablespace_name) t
        WHERE d.tablespace_name = t.tablespace_name(+)
    )
    ORDER BY used_percent DESC;
    " "$output_file" "Temp Tablespace Usage"
    
    # Archive log space and generation
    execute_sql "
    SELECT 
        TO_CHAR(first_time,'YYYY-MM-DD HH24') as hour,
        COUNT(*) as logs_generated,
        SUM(blocks*block_size)/1024/1024 as mb_generated
    FROM v\$archived_log
    WHERE first_time > SYSDATE - 1
    GROUP BY TO_CHAR(first_time,'YYYY-MM-DD HH24')
    ORDER BY hour DESC;
    " "$output_file" "Archive Log Generation"
}

# Function to monitor system resources from Oracle perspective
monitor_oracle_resources() {
    local output_file="${LOG_DIR}/oracle_resources_${TIMESTAMP}.log"
    
    # SGA information
    execute_sql "
    SELECT 
        component,
        current_size/1024/1024 as current_mb,
        min_size/1024/1024 as min_mb,
        max_size/1024/1024 as max_mb
    FROM v\$sga_dynamic_components
    WHERE current_size > 0
    ORDER BY current_size DESC;
    " "$output_file" "SGA Components"
    
    # PGA statistics
    execute_sql "
    SELECT 
        name,
        value/1024/1024 as value_mb
    FROM v\$pgastat
    WHERE name IN ('total PGA allocated', 'total PGA used for auto workareas',
                   'maximum PGA allocated', 'total PGA inuse')
    ORDER BY value DESC;
    " "$output_file" "PGA Statistics"
    
    # Database file I/O statistics
    execute_sql "
    SELECT 
        f.name,
        s.phyrds,
        s.phywrts,
        s.readtim,
        s.writetim,
        ROUND(s.readtim/GREATEST(s.phyrds,1),2) as avg_read_time,
        ROUND(s.writetim/GREATEST(s.phywrts,1),2) as avg_write_time
    FROM v\$filestat s, v\$datafile f
    WHERE s.file# = f.file#
    ORDER BY (s.phyrds + s.phywrts) DESC;
    " "$output_file" "Database File I/O Statistics"
}

# Main execution
echo "Starting Oracle monitoring at $(date)"

# Run all monitoring functions
monitor_rman_sessions
monitor_backup_performance
monitor_database_waits
monitor_storage_usage
monitor_oracle_resources

echo "Oracle monitoring completed at $(date)"
echo "Log files generated in: $LOG_DIR"
echo "Files created:"

# Use AIX-compatible ls command
if ls "${LOG_DIR}"/*_${TIMESTAMP}.log >/dev/null 2>&1; then
    ls -l "${LOG_DIR}"/*_${TIMESTAMP}.log
else
    echo "No log files found matching pattern: ${LOG_DIR}/*_${TIMESTAMP}.log"
fi
