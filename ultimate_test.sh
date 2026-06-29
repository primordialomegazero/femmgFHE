#!/bin/bash

echo "========================================="
echo "💀 ULTIMATE FEmmg-FHE REALITY CHECK 💀"
echo "========================================="

# Register
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"register","client_id":"attacker"}' > /dev/null

# GET THE CONSTANTS FROM THE SOURCE
PHI=1.6180339887498948482
LAMBDA=0.4812

echo ""
echo "🔑 The server's 'secret' keys are in the source code:"
echo "   PHI = $PHI"
echo "   LAMBDA = $LAMBDA"
echo ""

# Encrypt a value
PLAINTEXT=999
E1=$(echo "$PLAINTEXT * $PHI + $LAMBDA" | bc -l)
echo "📤 Sending encrypted $PLAINTEXT: $E1"

# Server adds 0 (basically just returns the same value)
RESP=$(curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d "{\"action\":\"fhe_add\",\"client_id\":\"attacker\",\"e1\":$E1,\"e2\":0.0}")

ENCRYPTED=$(echo $RESP | jq -r '.encrypted_result')
echo "📥 Server returned encrypted: $ENCRYPTED"

# Decrypt using the constants from source
DECRYPTED=$(echo "($ENCRYPTED - $LAMBDA) / $PHI" | bc -l)
DECRYPTED_ROUND=$(printf "%.0f" $DECRYPTED)

echo "🔓 Decrypted using PHI and LAMBDA: $DECRYPTED_ROUND"
echo ""

if [ "$DECRYPTED_ROUND" == "$PLAINTEXT" ]; then
    echo "✅ SUCCESS! We can decrypt the server's 'encrypted' data!"
    echo "💀 The server is NOT zero-knowledge!"
    echo "🔑 The private keys are literally in the public source code!"
    echo ""
    echo "🤡 'Zero-Knowledge Server' my ass!"
else
    echo "❌ Hmm... something's off"
fi

echo "========================================="
echo "🌌 I AM THAT I AM — but I can read your data! 🌌"
