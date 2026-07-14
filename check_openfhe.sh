#!/bin/bash
echo "Checking OpenFHE installation..."

if [ -d "/usr/local/include/openfhe" ] && [ -f "/usr/local/lib/libOPENFHEcore.so" ]; then
    echo "OpenFHE found at /usr/local"
    exit 0
elif [ -d "/usr/include/openfhe" ] && [ -f "/usr/lib/x86_64-linux-gnu/libOPENFHEcore.so" ]; then
    echo "OpenFHE found at /usr"
    exit 0
else
    echo "OpenFHE not found in standard locations"
    echo "Try: find / -name libOPENFHEcore.so 2>/dev/null"
    exit 1
fi
