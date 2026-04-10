#!/usr/bin/ksh
#
# StoreOnce Catalyst Network Monitor for AIX
# Purpose: Monitor network connectivity and performance to StoreOnce devices
# Version: 2.0
# Date: July 2025
#

# Configuration
LOG_DIR=${1:-"/tmp/network_monitoring"}
STOREONCE_IPS=${2:-""}  # Space-separated list of StoreOnce IPs
DURATION=${3:-60}       # Duration in minutes
INTERVAL=30             # Collection interval in seconds
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
HOSTNAME=$(hostname)
OUTPUT_FILE="${LOG_DIR}/network_events_${TIMESTAMP}.json"

# Create log directory
mkdir -p "$LOG_DIR"

# Function to get high-precision timestamp
get_timestamp() {
    # AIX 7.2+ optimized timestamp function
    # AIX 7.2 supports date +%s for epoch time, which is more reliable
    if date +%s >/dev/null 2>&1; then
        # Use epoch time for precision, convert to readable format
        date '+%Y-%m-%d %H:%M:%S'
    elif command -v perl >/dev/null 2>&1; then
        # Perl fallback for older systems
        perl -e 'use POSIX; print strftime("%Y-%m-%d %H:%M:%S", localtime)'
    else
        # Standard date fallback
        date '+%Y-%m-%d %H:%M:%S'
    fi
}

echo "=========================================="
echo "StoreOnce Catalyst Network Monitor Started"
echo "Host: $HOSTNAME"
echo "Start Time: $(date)"
echo "Duration: $DURATION minutes"
echo "StoreOnce IPs: $STOREONCE_IPS"
echo "Log Directory: $LOG_DIR"
echo "Output File: $OUTPUT_FILE"
echo "=========================================="

# Function to run command with timeout and logging
run_with_timeout() {
    local timeout_duration="$1"
    local description="$2"
    shift 2
    local command="$*"
    
    # Validate timeout duration is a positive integer
    if ! echo "$timeout_duration" | grep -q '^[0-9][0-9]*$'; then
        echo "$(date '+%Y-%m-%d %H:%M:%S') - ERROR: Invalid timeout duration '$timeout_duration' for $description"
        echo "$(date '+%Y-%m-%d %H:%M:%S') - Using default timeout of 30 seconds"
        timeout_duration=30
    fi
    
    # Check for timeout command availability
    local timeout_cmd=""
    if command -v timeout >/dev/null 2>&1; then
        timeout_cmd="timeout"
    elif [ -x "$(dirname "$0")/aix_timeout.sh" ]; then
        timeout_cmd="$(dirname "$0")/aix_timeout.sh"
    fi
    
    if [ -n "$timeout_cmd" ]; then
        $timeout_cmd "$timeout_duration" sh -c "$command"
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            echo "WARNING: $description timed out after ${timeout_duration}s"
            return 1
        fi
        return $exit_code
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
    fi
}

# Function to discover StoreOnce IPs from connections
discover_storeonce_ips() {
    if [ -z "$STOREONCE_IPS" ]; then
        echo "Auto-discovering StoreOnce connections..."
        # Look for established connections on common StoreOnce ports
        DISCOVERED_IPS=$(netstat -an | grep -E ":(9387|9388|9389)" | grep ESTABLISHED | awk '{print $5}' | sed 's/[.:][^.:]*$//' | sort -u)
        if [ -n "$DISCOVERED_IPS" ]; then
            STOREONCE_IPS="$DISCOVERED_IPS"
            echo "Discovered StoreOnce IPs: $STOREONCE_IPS"
        else
            echo "No StoreOnce connections found. Testing common network ranges..."
            # AIX 7.2+ optimized: Pre-check if IPs are reachable before testing
            local test_ips="10.0.0.1 172.16.0.1 192.168.1.1"
            local valid_ips=""
            
            for ip in $test_ips; do
                # Quick ping test - AIX 7.2+ supports -W flag for timeout
                if ping -c 1 -W 2 "$ip" >/dev/null 2>&1; then
                    valid_ips="$valid_ips $ip"
                    echo "Found reachable IP: $ip"
                fi
            done
            
            if [ -n "$valid_ips" ]; then
                STOREONCE_IPS="$valid_ips"
                echo "Using reachable IPs: $STOREONCE_IPS"
            else
                STOREONCE_IPS="$test_ips"  # Fallback to test all
                echo "No IPs responded to ping, will test all: $STOREONCE_IPS"
            fi
        fi
    fi
}

# Function to test network connectivity
test_network_connectivity() {
    local timestamp="$1"
    local output_file="${LOG_DIR}/network_connectivity_${TIMESTAMP}.log"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        for ip in $STOREONCE_IPS; do
            echo "--- Testing connectivity to $ip ---"
            
            # Ping test
            echo "PING Test:"
            ping -c 5 "$ip" 2>&1
            echo ""
            
            # Port connectivity test for common StoreOnce ports
            for port in 9387 9388 9389 22 443; do
                echo "Testing port $port on $ip:"
                if command -v telnet >/dev/null 2>&1; then
                    run_with_timeout 15 "telnet test to $ip:$port" "echo 'quit' | telnet $ip $port 2>&1 | head -3"
                else
                    echo "telnet not available"
                fi
                echo ""
            done
            
            # Traceroute
            echo "TRACEROUTE to $ip:"
            traceroute "$ip" 2>&1 | head -10
            echo ""
            
            echo "=========================="
        done
        
    } >> "$output_file"
}

# Function to monitor network statistics
collect_network_stats() {
    local timestamp="$1"
    local output_file="${LOG_DIR}/network_stats_${TIMESTAMP}.log"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        echo "=== NETWORK INTERFACE STATISTICS ==="
        netstat -i
        echo ""
        
        echo "=== NETWORK INTERFACE DETAILS ==="
        for interface in $(netstat -i | grep -v "Name" | awk '{print $1}' | grep -v "lo"); do
            echo "--- Interface: $interface ---"
            ifconfig "$interface" 2>/dev/null
            echo ""
        done
        
        echo "=== NETWORK CONNECTIONS ==="
        netstat -an | grep -E "(ESTABLISHED|LISTEN)" | grep -E "(9387|9388|9389)"
        echo ""
        
        echo "=== ROUTING TABLE ==="
        netstat -rn
        echo ""
        
        echo "=== ARP TABLE ==="
        arp -a
        echo ""
        
    } >> "$output_file"
}

# Function to monitor StoreOnce specific connections
monitor_storeonce_connections() {
    local event_timestamp="$1"
    
    # Active StoreOnce Connections
    local connections_json=$(netstat -an | grep -E "(9387|9388|9389)" | awk -v ts="$event_timestamp" -v host="$HOSTNAME" '
    BEGIN { print "{\"timestamp\":\"" ts "\",\"hostname\":\"" host "\",\"type\":\"active_connection\",\"data\":{" }
    {
        # Create a JSON object for each line
        printf "\"%s\":{\"local_address\":\"%s\",\"foreign_address\":\"%s\",\"state\":\"%s\"},", NR, $4, $5, $6
    }
    END { print "}}" }' | sed 's/},}}$/}}}/') # Remove trailing comma
    echo "$connections_json" >> "$OUTPUT_FILE"

    # Connection Counts by State
    local counts_json=$(netstat -an | grep -E "(9387|9388|9389)" | awk '{print $6}' | sort | uniq -c | awk -v ts="$event_timestamp" -v host="$HOSTNAME" '
    BEGIN { print "{\"timestamp\":\"" ts "\",\"hostname\":\"" host "\",\"type\":\"connection_summary\",\"data\":{" }
    {
        # Create a JSON object for each state count
        gsub(/^[ \t]+|[ \t]+$/, "", $2); # Trim whitespace
        printf "\"%s\":%d,", $2, $1
    }
    END { print "}}" }' | sed 's/},}}$/}}}/') # Remove trailing comma
    echo "$counts_json" >> "$OUTPUT_FILE"

    # Established Connections to StoreOnce IPs
    for ip in $STOREONCE_IPS; do
        local established_json=$(netstat -an | grep "$ip" | grep ESTABLISHED | awk -v ts="$event_timestamp" -v host="$HOSTNAME" -v storeonce_ip="$ip" '
        BEGIN { print "{\"timestamp\":\"" ts "\",\"hostname\":\"" host "\",\"type\":\"established_connection\",\"storeonce_ip\":\"" storeonce_ip "\",\"data\":{" }
        {
            printf "\"%s\":{\"local_address\":\"%s\",\"foreign_address\":\"%s\",\"state\":\"%s\"},", NR, $4, $5, $6
        }
        END { print "}}" }' | sed 's/},}}$/}}}/') # Remove trailing comma
        echo "$established_json" >> "$OUTPUT_FILE"
    done
}

# Function to monitor network performance metrics
collect_network_performance() {
    local timestamp="$1"
    local output_file="${LOG_DIR}/network_performance_${TIMESTAMP}.log"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        echo "=== NETWORK BUFFER STATISTICS ==="
        netstat -m
        echo ""
        
        echo "=== NETWORK ERROR STATISTICS ==="
        # AIX compatible: Extract error statistics without -A/-B options
        netstat -s | awk '
            /[Ee]rror/ { 
                print "Error line found: " $0
                continue
            }
            /[Dd]rop/ { 
                print "Drop line found: " $0
                continue
            }
            /[Ff]ail/ { 
                print "Fail line found: " $0
                continue
            }
        ' 2>/dev/null || {
            echo "Basic network statistics:"
            netstat -s 2>/dev/null | head -20
        }
        echo ""
        
        echo "=== ETHERNET STATISTICS ==="
        for interface in $(netstat -i | grep -v "Name" | awk '{print $1}' | grep en); do
            echo "--- Interface: $interface ---"
            entstat "$interface" 2>/dev/null | grep -E "(Packets|Bytes|Errors|Collisions)" | head -10
            echo ""
        done
        
        # Power 10 specific: Enhanced network monitoring
        echo "=== POWER 10 ENHANCED NETWORK MONITORING ==="
        # Check for high-speed and virtual adapters
        VIRTUAL_ADAPTERS=$(lsdev -Cc adapter | grep -i virtual | awk '{print $1}')
        if [ -n "$VIRTUAL_ADAPTERS" ]; then
            echo "--- Virtual Network Adapters (Power 10) ---"
            for vadapter in $VIRTUAL_ADAPTERS; do
                echo "Virtual Adapter: $vadapter"
                lsattr -El "$vadapter" 2>/dev/null | head -5
                echo ""
            done
        fi
        
        # Check for SR-IOV capabilities
        SRIOV_ADAPTERS=$(lsdev -Cc adapter | grep -E "(ent|en)" | awk '{print $1}')
        if [ -n "$SRIOV_ADAPTERS" ]; then
            echo "--- Network Adapter Capabilities ---"
            for adapter in $SRIOV_ADAPTERS; do
                echo "Adapter: $adapter"
                lsattr -El "$adapter" 2>/dev/null | grep -E "(speed|duplex|flow_ctrl)" | head -3
                echo ""
            done
        fi
        echo ""
        
        echo "=== NETWORK DEVICE ERRORS ==="
        errpt -d H -N en* -N et* | head -10
        echo ""
        
    } >> "$output_file"
}

# Function to test bandwidth to StoreOnce
test_bandwidth() {
    local timestamp="$1"
    local output_file="${LOG_DIR}/bandwidth_test_${TIMESTAMP}.log"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        for ip in $STOREONCE_IPS; do
            echo "--- Bandwidth test to $ip ---"
            
            # Simple bandwidth test using dd and nc if available
            if command -v nc >/dev/null 2>&1; then
                echo "Testing with netcat (if port 9999 is available):"
                run_with_timeout 10 "netcat bandwidth test to $ip" "dd if=/dev/zero bs=1M count=10 2>/dev/null | nc $ip 9999" 2>&1 || echo "netcat test failed or timed out"
            fi
            
            # Alternative bandwidth estimation using ping
            echo "RTT Analysis with larger packets:"
            for size in 64 512 1024 1472; do
                echo "Packet size $size bytes:"
                ping -s $size -c 3 "$ip" 2>&1 | grep "round-trip" || echo "No response"
            done
            echo ""
        done
        
    } >> "$output_file"
}

# Function to monitor CoFC (Converged FabricCache) if applicable
monitor_cofc() {
    local timestamp="$1"
    local output_file="${LOG_DIR}/cofc_monitor_${TIMESTAMP}.log"
    
    {
        echo "=== TIMESTAMP: $timestamp ==="
        
        echo "=== FC ADAPTER STATUS ==="
        lsdev -Cc adapter | grep fcs
        echo ""
        
        for adapter in $(lsdev -Cc adapter | grep fcs | awk '{print $1}'); do
            echo "--- FC Adapter: $adapter ---"
            lsattr -El "$adapter" 2>/dev/null
            echo ""
            
            echo "--- FC Statistics for $adapter ---"
            fcstat "$adapter" 2>/dev/null | head -20
            echo ""
        done
        
        echo "=== FC ERRORS ==="
        errpt -d H -N fcs* | head -15
        echo ""
        
        echo "=== FCOE STATUS ==="
        if command -v fcoestat >/dev/null 2>&1; then
            fcoestat 2>/dev/null
        else
            echo "FCoE not available or configured"
        fi
        echo ""
        
    } >> "$output_file"
}

# Main monitoring loop
log_with_timestamp() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1"
}

# Discover StoreOnce IPs if not provided
discover_storeonce_ips

log_with_timestamp "Starting network monitoring for $DURATION minutes"
log_with_timestamp "Monitoring StoreOnce IPs: $STOREONCE_IPS"

END_TIME=$(($(date +%s) + $DURATION * 60))
COUNTER=0

while [ $(date +%s) -lt $END_TIME ]; do
    CURRENT_TIME=$(get_timestamp)
    COUNTER=$((COUNTER + 1))
    
    log_with_timestamp "Network collection cycle $COUNTER - $CURRENT_TIME"
    
    # Collect all network monitoring data
    # For Phase 1, we only enhance monitor_storeonce_connections and keep others as is.
    monitor_storeonce_connections "$CURRENT_TIME" &
    
    # These functions still log to their original text files
    test_network_connectivity "$CURRENT_TIME" &
    collect_network_stats "$CURRENT_TIME" &
    collect_network_performance "$CURRENT_TIME" &
    monitor_cofc "$CURRENT_TIME" &
    
    # Run bandwidth test every 5th cycle to avoid overwhelming the network
    if [ $((COUNTER % 5)) -eq 0 ]; then
        test_bandwidth "$CURRENT_TIME" &
    fi
    
    # Wait for all background jobs to complete
    wait
    
    # Wait for next interval
    sleep $INTERVAL
done

log_with_timestamp "Network monitoring completed. Logs saved in: $LOG_DIR"

# Generate summary report
echo "=========================================="
echo "NETWORK MONITORING SUMMARY"
echo "=========================================="
echo "Host: $HOSTNAME"
echo "Start Time: $(perl -e "use POSIX; print strftime('%Y-%m-%d %H:%M:%S', localtime(time - $DURATION * 60))" 2>/dev/null || echo 'Approximately $DURATION minutes ago')"
echo "End Time: $(date)"
echo "Duration: $DURATION minutes"
echo "StoreOnce IPs Monitored: $STOREONCE_IPS"
echo "Total Collections: $COUNTER"
echo "Log Files Generated:"
ls -la "${LOG_DIR}"/*_${TIMESTAMP}.log
echo "=========================================="
