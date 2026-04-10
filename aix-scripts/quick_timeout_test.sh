#!/usr/bin/ksh
#
# Quick Timeout Test for AIX Monitoring Suite
# Tests various timeout mechanisms
# Note: Uses bash for testing, but AIX scripts use ksh
# Version: 2.0
#

SCRIPT_DIR="$(dirname "$0")"
TEST_DIR="/tmp/timeout_test_$(date +%Y%m%d_%H%M%S)"

echo "=========================================="
echo "AIX Monitoring Timeout Test"
echo "=========================================="
echo "Script Directory: $SCRIPT_DIR"
echo "Test Directory: $TEST_DIR"
echo "Start Time: $(date)"
echo "=========================================="

mkdir -p "$TEST_DIR"

# Test 1: Check if timeout command is available
echo "1. Testing timeout command availability:"
if command -v timeout >/dev/null 2>&1; then
    echo "   System timeout command: AVAILABLE"
    timeout 2 sleep 1 && echo "   System timeout test: OK"
else
    echo "   System timeout command: NOT AVAILABLE"
fi

# Test 2: Test our AIX timeout utility
echo ""
echo "2. Testing AIX timeout utility:"
if [ -x "$SCRIPT_DIR/aix_timeout.sh" ]; then
    echo "   AIX timeout utility: AVAILABLE"
    if "$SCRIPT_DIR/aix_timeout.sh" 2 sleep 1; then
        echo "   AIX timeout test: OK"
    else
        echo "   AIX timeout test: FAILED"
    fi
    
    # Test timeout functionality
    echo "   Testing timeout functionality (should timeout after 2 seconds)..."
    START_TIME=$(date +%s)
    "$SCRIPT_DIR/aix_timeout.sh" 2 sleep 5
    EXIT_CODE=$?
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))
    
    if [ $EXIT_CODE -eq 124 ] && [ $ELAPSED -le 4 ]; then
        echo "   Timeout functionality: OK (elapsed: ${ELAPSED}s, exit: $EXIT_CODE)"
    else
        echo "   Timeout functionality: FAILED (elapsed: ${ELAPSED}s, exit: $EXIT_CODE)"
    fi
else
    echo "   AIX timeout utility: NOT FOUND"
fi

# Test 3: Test storage monitor with very short duration
echo ""
echo "3. Testing storage monitor timeout (30 second limit):"
START_TIME=$(date +%s)

# Use background process to monitor storage script
(
    "$SCRIPT_DIR/storage_monitor.sh" "$TEST_DIR" 0.25 >/dev/null 2>&1 &
    STORAGE_PID=$!
    
    # Wait up to 30 seconds
    COUNT=0
    while [ $COUNT -lt 30 ]; do
        # Use AIX-compatible process check
        if ! ps -ef | awk '{print $2}' | grep -q "^${STORAGE_PID}$"; then
            # Process completed
            wait "$STORAGE_PID"
            EXIT_CODE=$?
            END_TIME=$(date +%s)
            ELAPSED=$((END_TIME - START_TIME))
            echo "   Storage monitor completed in ${ELAPSED}s (exit code: $EXIT_CODE)"
            exit 0
        fi
        sleep 1
        COUNT=$((COUNT + 1))
    done
    
    # If we reach here, the process is still running - kill it
    if ps -ef | awk '{print $2}' | grep -q "^${STORAGE_PID}$"; then
        echo "   Storage monitor TIMED OUT after 30s - killing process"
        kill -TERM "$STORAGE_PID" 2>/dev/null
        sleep 2
        if ps -ef | awk '{print $2}' | grep -q "^${STORAGE_PID}$"; then
            kill -KILL "$STORAGE_PID" 2>/dev/null
        fi
        END_TIME=$(date +%s)
        ELAPSED=$((END_TIME - START_TIME))
        echo "   Storage monitor killed after ${ELAPSED}s"
        exit 1
    fi
) &
TEST_PID=$!

wait $TEST_PID
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo "   Storage monitor timeout test: PASSED"
else
    echo "   Storage monitor timeout test: FAILED"
fi

# Test 4: Test individual command timeouts
echo ""
echo "4. Testing individual command timeouts:"

# Create a test that should timeout
echo "   Testing 2-second timeout on 5-second sleep..."
START_TIME=$(date +%s)

if [ -x "$SCRIPT_DIR/aix_timeout.sh" ]; then
    "$SCRIPT_DIR/aix_timeout.sh" 2 sleep 5
    EXIT_CODE=$?
elif command -v timeout >/dev/null 2>&1; then
    timeout 2 sleep 5
    EXIT_CODE=$?
else
    echo "   No timeout utility available - using manual method"
    (
        sleep 5 &
        PID=$!
        (
            sleep 2
            if ps -ef | awk '{print $2}' | grep -q "^${PID}$"; then
                kill -TERM "$PID" 2>/dev/null
            fi
        ) &
        KILLER_PID=$!
        wait $PID
        EXIT_CODE=$?
        kill $KILLER_PID 2>/dev/null
        if [ $EXIT_CODE -ne 0 ]; then
            EXIT_CODE=124  # Timeout exit code
        fi
    )
fi

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

if [ $EXIT_CODE -eq 124 ] && [ $ELAPSED -le 4 ]; then
    echo "   Command timeout test: PASSED (${ELAPSED}s, exit: $EXIT_CODE)"
else
    echo "   Command timeout test: FAILED (${ELAPSED}s, exit: $EXIT_CODE)"
fi

echo ""
echo "=========================================="
echo "TIMEOUT TEST SUMMARY"
echo "=========================================="
echo "Test completed at: $(date)"
echo "Test logs in: $TEST_DIR"

# Count log files to verify storage monitor actually ran
LOG_COUNT=$(ls -1 "$TEST_DIR"/*.log 2>/dev/null | wc -l | tr -d ' ')
if [ "$LOG_COUNT" -gt 0 ]; then
    echo "Storage monitor created $LOG_COUNT log files"
else
    echo "Storage monitor created no log files"
fi

echo "=========================================="
