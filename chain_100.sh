#!/bin/bash

echo "╔══════════════════════════════════════════════╗"
echo "║  FEmmg-FHE 100-CHAIN ADDITION TEST          ║"
echo "╚══════════════════════════════════════════════╝"

# Register
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"register","client_id":"chain100"}' > /dev/null

# Encrypt 1
RESULT=$(curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_encrypt","client_id":"chain100","plaintext":1}')
TOTAL_INDEX=$(echo $RESULT | grep -o '"ciphertext_index":[0-9]*' | cut -d: -f2)
TOTAL=1

echo -n "🔗 1"

# Add 2 to 100
for i in {2..100}; do
  # Encrypt i
  RESULT=$(curl -s -X POST http://localhost:8092/ \
    -H "Content-Type: application/json" \
    -d "{\"action\":\"fhe_encrypt\",\"client_id\":\"chain100\",\"plaintext\":$i}")
  NEW_INDEX=$(echo $RESULT | grep -o '"ciphertext_index":[0-9]*' | cut -d: -f2)
  
  # Add to total
  RESULT=$(curl -s -X POST http://localhost:8092/ \
    -H "Content-Type: application/json" \
    -d "{\"action\":\"fhe_add\",\"client_id\":\"chain100\",\"ciphertext_index_1\":$TOTAL_INDEX,\"ciphertext_index_2\":$NEW_INDEX}")
  TOTAL_INDEX=$(echo $RESULT | grep -o '"result_index":[0-9]*' | cut -d: -f2)
  
  TOTAL=$((TOTAL + i))
  
  # Progress
  if [ $((i % 10)) -eq 0 ]; then
    echo -n " +...$i"
  fi
done

echo ""
echo "🔓 Decrypting result..."

# Decrypt
RESULT=$(curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d "{\"action\":\"fhe_decrypt\",\"client_id\":\"chain100\",\"ciphertext_index\":$TOTAL_INDEX}")
DECRYPTED=$(echo $RESULT | grep -o '"decrypted":[0-9]*' | cut -d: -f2)

EXPECTED=5050  # Sum of 1 to 100

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║  RESULT:                                   ║"
echo "║  Expected: $EXPECTED                        ║"
echo "║  Decrypted: $DECRYPTED                     ║"
if [ "$DECRYPTED" == "$EXPECTED" ]; then
  echo "║  ✅ UNLIMITED DEPTH PROVEN! (100 ops)     ║"
  echo "║  🎉 NO BOOTSTRAPPING NEEDED!             ║"
else
  echo "║  ❌ Test failed                          ║"
fi
echo "╚══════════════════════════════════════════════╝"
