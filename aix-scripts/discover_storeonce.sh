#!/usr/bin/ksh
#
# Quick script to discover StoreOnce appliances in your environment
# Version: 2.0
#

echo "Discovering StoreOnce appliances..."
echo "=========================================="

echo "1. Checking active StoreOnce connections:"
# AIX-compatible network connection check
netstat -an | grep ":938[789]" | grep ESTABLISHED | awk '{print $5}' | sed 's/\.[0-9]*$//' | sort -u 2>/dev/null || echo "  No active StoreOnce connections found"

echo ""
echo "2. Checking Oracle RMAN configuration for StoreOnce:"
# Check if Oracle environment is available
if [ -n "$ORACLE_HOME" ] && [ -x "$ORACLE_HOME/bin/rman" ]; then
    {
        echo "connect target /"
        echo "show all;"
        echo "exit;"
    } | "$ORACLE_HOME/bin/rman" target / 2>/dev/null | grep -i "BACKUP_OPTIMIZATION\|SBT_TAPE" || echo "  No StoreOnce configuration found in RMAN"
elif command -v rman >/dev/null 2>&1; then
    {
        echo "connect target /"
        echo "show all;"
        echo "exit;"
    } | rman target / 2>/dev/null | grep -i "BACKUP_OPTIMIZATION\|SBT_TAPE" || echo "  No StoreOnce configuration found in RMAN"
else
    echo "  RMAN not accessible (Oracle environment not configured)"
fi

echo ""
echo "3. Checking for StoreOnce processes:"
# AIX-compatible process search
ps -ef | grep -i "catalyst\|storeonce\|obosh" | grep -v grep || echo "  No StoreOnce-related processes found"

echo ""
echo "4. Checking network routes to common StoreOnce subnets:"
for subnet in 10.1.1 192.168.10 172.16.10; do
    echo "Routes to ${subnet}.x:"
    # AIX netstat compatibility
    netstat -rn 2>/dev/null | grep "$subnet" || echo "  No routes to $subnet.x"
done

echo ""
echo "5. Checking /etc/hosts for StoreOnce entries:"
grep -i "storeonce\|catalyst" /etc/hosts 2>/dev/null || echo "No StoreOnce entries in /etc/hosts"

echo ""
echo "=========================================="
echo "If you find StoreOnce IPs above, use them in the monitoring command:"
echo "Example: ./master_monitor.sh /logs 2100 \"IP1 IP2\""
