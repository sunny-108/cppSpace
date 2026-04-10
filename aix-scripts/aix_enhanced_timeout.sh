#!/usr/bin/ksh
#
# AIX 7.2+ Enhanced Timeout Utility
# Provides robust timeout functionality with better process management
# Usage: aix_enhanced_timeout.sh <seconds> <description> <command> [args...]
# Version: 2.1 - AIX 7.2+ Optimized
#

if [ $# -lt 3 ]; then
    echo "Usage: $0 <timeout_seconds> <description> <command> [args...]" >&2
    echo "Example: $0 30 'disk listing' lsdev -Cc disk" >&2
    exit 1
fi

TIMEOUT_DURATION="$1"
DESCRIPTION="$2"
shift 2
COMMAND="$*"

# AIX 7.2+ optimized validation
if ! echo "$TIMEOUT_DURATION" | grep -q '^[0-9][0-9]*$' || [ "$TIMEOUT_DURATION" -lt 1 ]; then
    echo "Error: Timeout duration must be a positive integer (got: $TIMEOUT_DURATION)" >&2
    exit 1
fi

# Enhanced logging function
log_timeout() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - TIMEOUT: $*" >&2
}

# Execute command with enhanced timeout handling
(
    # Start the command in background
    eval "$COMMAND" &
    COMMAND_PID=$!
    
    # Start enhanced timeout monitor
    (
        check_interval=1
        elapsed=0
        
        while [ $elapsed -lt "$TIMEOUT_DURATION" ]; do
            if ! kill -0 "$COMMAND_PID" 2>/dev/null; then
                # Process completed normally
                exit 0
            fi
            
            sleep $check_interval
            elapsed=$((elapsed + check_interval))
            
            # Progress logging for long operations
            if [ $elapsed -gt 10 ] && [ $((elapsed % 10)) -eq 0 ]; then
                log_timeout "$DESCRIPTION: ${elapsed}s elapsed (timeout: ${TIMEOUT_DURATION}s)"
            fi
        done
        
        # Timeout reached
        if kill -0 "$COMMAND_PID" 2>/dev/null; then
            log_timeout "$DESCRIPTION (PID: $COMMAND_PID) timed out after ${TIMEOUT_DURATION}s"
            
            # Graceful termination attempt
            kill -TERM "$COMMAND_PID" 2>/dev/null
            sleep 2
            
            # Force kill if still running
            if kill -0 "$COMMAND_PID" 2>/dev/null; then
                log_timeout "Force killing $DESCRIPTION (PID: $COMMAND_PID)"
                kill -KILL "$COMMAND_PID" 2>/dev/null
                sleep 1
            fi
            
            # Final check
            if kill -0 "$COMMAND_PID" 2>/dev/null; then
                log_timeout "WARNING: Process $COMMAND_PID may still be running"
            else
                log_timeout "$DESCRIPTION (PID: $COMMAND_PID) terminated successfully"
            fi
        fi
    ) &
    TIMEOUT_PID=$!
    
    # Wait for command to complete
    wait "$COMMAND_PID"
    COMMAND_EXIT=$?
    
    # Clean up timeout monitor
    kill "$TIMEOUT_PID" 2>/dev/null
    wait "$TIMEOUT_PID" 2>/dev/null
    
    # Enhanced exit code handling
    case $COMMAND_EXIT in
        0)
            # Success
            exit 0
            ;;
        124)
            # Timeout (GNU timeout compatible)
            exit 124
            ;;
        143|137)
            # Process was killed (SIGTERM=143, SIGKILL=137)
            log_timeout "$DESCRIPTION was terminated"
            exit 124
            ;;
        *)
            # Other error
            if [ $COMMAND_EXIT -ne 0 ]; then
                log_timeout "$DESCRIPTION failed with exit code $COMMAND_EXIT"
            fi
            exit $COMMAND_EXIT
            ;;
    esac
)

exit_code=$?

# Final status logging
case $exit_code in
    0)
        echo "$(date '+%Y-%m-%d %H:%M:%S') - SUCCESS: $DESCRIPTION completed" >&2
        ;;
    124)
        echo "$(date '+%Y-%m-%d %H:%M:%S') - TIMEOUT: $DESCRIPTION exceeded ${TIMEOUT_DURATION}s limit" >&2
        ;;
    *)
        echo "$(date '+%Y-%m-%d %H:%M:%S') - ERROR: $DESCRIPTION failed (exit code: $exit_code)" >&2
        ;;
esac

exit $exit_code