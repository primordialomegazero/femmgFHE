#!/bin/bash
# Alternative: Fetch public key from blockchain.com API
# Only works for SPENT coins (where public key was revealed)

# Example: Hal Finney's address from block #170
ADDRESS="1Q2TWvVRjQXwYf3hBxMJYcWqCXHhPqMSW"

echo "=== Fetching transactions for $ADDRESS ==="

# Get transactions for this address
curl -s "https://blockchain.info/rawaddr/$ADDRESS" | jq '.txs[] | {hash: .hash, time: .time}' 2>/dev/null | head -20

# To get the actual public key, we need the spending transaction's scriptSig
# This requires looking at the INPUT of the transaction that SPENT these coins

echo ""
echo "To get the actual public key:"
echo "1. Find the transaction where $ADDRESS was an OUTPUT (received coins)"
echo "2. Find the transaction where those coins were SPENT"
echo "3. Look at the INPUT scriptSig → contains public key"
echo ""
echo "This requires a full blockchain explorer or Bitcoin Core sync."
