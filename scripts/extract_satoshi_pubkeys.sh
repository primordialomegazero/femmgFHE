#!/bin/bash
# Extract Satoshi-era public keys from Bitcoin Core
# Usage: bash extract_satoshi_pubkeys.sh

BITCOIN_CLI="bitcoin-cli"

echo "=== Checking Bitcoin Core ==="
$BITCOIN_CLI getblockchaininfo | grep -E '"blocks"|"verificationprogress"'

echo ""
echo "=== Looking for Satoshi-era transactions ==="

# Known early blocks with Satoshi transactions
BLOCKS=(9 170 181)

for BLOCK in "${BLOCKS[@]}"; do
    echo ""
    echo "--- Block #$BLOCK ---"
    
    BLOCK_HASH=$($BITCOIN_CLI getblockhash $BLOCK 2>/dev/null)
    if [ -z "$BLOCK_HASH" ]; then
        echo "  Block not found (not synced yet?)"
        continue
    fi
    
    BLOCK_DATA=$($BITCOIN_CLI getblock $BLOCK_HASH 2)
    
    # Get transaction IDs in this block
    TX_IDS=$(echo "$BLOCK_DATA" | jq -r '.tx[]')
    
    for TXID in $TX_IDS; do
        echo "  TX: $TXID"
        
        RAW_TX=$($BITCOIN_CLI getrawtransaction $TXID 1 2>/dev/null)
        if [ -z "$RAW_TX" ]; then
            echo "    (transaction not found in wallet)"
            continue
        fi
        
        # Check inputs for scriptSig containing public keys
        echo "$RAW_TX" | jq -r '.vin[] | .scriptSig.hex // empty' 2>/dev/null | while read -r SCRIPTSIG; do
            if [ -n "$SCRIPTSIG" ] && [ ${#SCRIPTSIG} -gt 130 ]; then
                echo "    scriptSig: ${SCRIPTSIG:0:60}..."
                # Check for uncompressed public key (65 bytes = 130 hex chars, starts with 04)
                if [[ "$SCRIPTSIG" == *"04"* ]]; then
                    PUBKEY=$(echo "$SCRIPTSIG" | grep -oP '04[0-9a-fA-F]{128}')
                    if [ -n "$PUBKEY" ]; then
                        PUBKEY_X="0x${PUBKEY:2:64}"
                        PUBKEY_Y="0x${PUBKEY:66:64}"
                        echo "    🎉 PUBLIC KEY FOUND!"
                        echo "    X: $PUBKEY_X"
                        echo "    Y: $PUBKEY_Y"
                    fi
                fi
            fi
        done
        
        # Check outputs for P2PK (pay-to-pubkey) scripts
        echo "$RAW_TX" | jq -r '.vout[] | .scriptPubKey.hex // empty' 2>/dev/null | while read -r SCRIPTPK; do
            if [[ "$SCRIPTPK" == *"04"* ]] && [ ${#SCRIPTPK} -gt 130 ]; then
                PUBKEY=$(echo "$SCRIPTPK" | grep -oP '04[0-9a-fA-F]{128}')
                if [ -n "$PUBKEY" ]; then
                    PUBKEY_X="0x${PUBKEY:2:64}"
                    PUBKEY_Y="0x${PUBKEY:66:64}"
                    echo "    🎉 PUBLIC KEY IN OUTPUT!"
                    echo "    X: $PUBKEY_X"
                    echo "    Y: $PUBKEY_Y"
                fi
            fi
        done
    done
done

echo ""
echo "=== Done ==="
