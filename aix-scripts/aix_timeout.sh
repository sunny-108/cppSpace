#!/usr/bin/ksh
#
# AIX-Compatible Timeout Utility
# Provides timeout functionality for systems without GNU timeout command
# Usage: aix_timeout.sh <seconds> <command> [args...]
# Note: For AIX deployment - uses ksh for compatibility
# Version: 2.0
#

if [ $# -lt 2 ]; then
    echo "Usage: $0 <timeout_seconds> <command> [args...]" >&2
    exit 1
fi

TIMEOUT_DURATION="$1"
shift
COMMAND="$*"

# Validate timeout duration - AIX-compatible regex
if ! echo "$TIMEOUT_DURATION" | grep -q '^[0-9][0-9]*$'; then
    echo "Error: Timeout duration must be a positive integer" >&2
    exit 1
fi

# Execute command with timeout
(
    # Start the command in background
    eval "$COMMAND" &
    COMMAND_PID=$!
    
    # Start timeout monitor in background
    (
        sleep "$TIMEOUT_DURATION"
        if kill -0 "$COMMAND_PID" 2>/dev/null; then
            echo "aix_timeout: Process $COMMAND_PID timed out after ${TIMEOUT_DURATION}s" >&2
            kill -TERM "$COMMAND_PID" 2>/dev/null
            sleep 2
            if kill -0 "$COMMAND_PID" 2>/dev/null; then
                kill -KILL "$COMMAND_PID" 2>/dev/null
            fi
        fi
    ) &
    TIMEOUT_PID=$!
    
    # Wait for command to complete
    wait "$COMMAND_PID"
    COMMAND_EXIT=$?
    
    # Clean up timeout monitor
    kill "$TIMEOUT_PID" 2>/dev/null
    
    # Return command exit code, or 124 if timed out (same as GNU timeout)
    if [ $COMMAND_EXIT -eq 143 ] || [ $COMMAND_EXIT -eq 137 ]; then
        # Process was killed (SIGTERM=143, SIGKILL=137)
        exit 124
    else
        exit $COMMAND_EXIT
    fi
)
