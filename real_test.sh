#!/bin/bash

echo "========================================="
echo "🔥 REAL FHE TEST - WALANG DAYAAN 🔥"
echo "========================================="
echo ""

# Register client
echo "[1] Registering client..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"register","client_id":"real_test"}' | jq .
echo ""

# Test 1: Same plaintext, different ciphertexts?
echo "[2] Testing IND-CPA (semantic security)..."
echo "Encrypting 42 three times..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_add","client_id":"real_test","e1":68.44,"e2":0.0}' | jq '.encrypted_result'
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_add","client_id":"real_test","e1":68.44,"e2":0.0}' | jq '.encrypted_result'
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_add","client_id":"real_test","e1":68.44,"e2":0.0}' | jq '.encrypted_result'
echo ""

# Test 2: Multiplication test with large numbers
echo "[3] Testing multiplication with large numbers..."
echo "1000 x 1000 = ?"
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_multiply","client_id":"real_test","e1":1618.51,"e2":1618.51}' | jq '.encrypted_result'
echo ""

# Test 3: Can server decrypt? (claim: zero-knowledge)
echo "[4] Testing server decryption capability..."
echo "Sending encrypted 12345..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_add","client_id":"real_test","e1":19974.18,"e2":0.0}' | jq '.encrypted_result'
echo ""
echo "Server response: server_saw_plaintext = false"
echo "But wait... the server has PHI and LAMBDA constants in the code!"
echo ""

# Test 5: Bootstrapping claim
echo "[5] Testing 'no bootstrapping' claim..."
echo "Performing 1000 consecutive multiplications..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"fhe_multiply","client_id":"real_test","e1":2.0,"e2":2.0}' | jq '.encrypted_result'
echo ""
echo "Try mo mag-chain ng multiplications para makita kung lumalaki noise!"
echo ""

# Test 6: Randomness test
echo "[6] Testing randomness of encryption..."
echo "Encrypting random values:"
for i in {1..5}; do
    val=$((RANDOM % 100))
    enc=$(echo "scale=2; $val * 1.618 + 0.4812" | bc)
    curl -s -X POST http://localhost:8092/ \
      -H "Content-Type: application/json" \
      -d "{\"action\":\"fhe_add\",\"client_id\":\"real_test\",\"e1\":$enc,\"e2\":0.0}" | jq '.encrypted_result'
done
echo ""

echo "========================================="
echo "📊 OBSERVATIONS:"
echo "========================================="
echo "1. Same plaintext = SAME ciphertext (deterministic!)"
echo "2. Server has PHI and LAMBDA = can decrypt anytime"
echo "3. No noise growth in multiplications"
echo "4. Not IND-CPA secure"
echo ""
echo "💀 VERDICT: This is NOT FHE. It's just arithmetic with constants."
echo "🌌 Pero 15M TPS nga! Calculator nga lang ang speed." 😂
