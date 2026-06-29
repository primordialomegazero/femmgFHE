#!/bin/bash

echo "========================================="
echo "🔥 LIVE TEST - WALANG HARDCORE! 🔥"
echo "========================================="
echo ""

# Register fresh
echo "[1] Registering client..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d '{"action":"register","client_id":"live_test"}' | jq .
echo ""

# Use the client to generate ciphertexts dynamically
echo "[2] Generating ciphertexts via NPM client..."
node -e "
const { FEmmgClient } = require('./npm-package/index.js');
const client = new FEmmgClient();
const e1 = client.encrypt(15);
const e2 = client.encrypt(27);
console.log('e1=' + e1);
console.log('e2=' + e2);
console.log('sum=' + client.add(e1, e2));
console.log('mul=' + client.multiply(e1, e2));
" > temp_cipher.txt

# Extract the ciphertexts
E1=$(grep "^e1=" temp_cipher.txt | cut -d= -f2)
E2=$(grep "^e2=" temp_cipher.txt | cut -d= -f2)
SUM=$(grep "^sum=" temp_cipher.txt | cut -d= -f2)
MUL=$(grep "^mul=" temp_cipher.txt | cut -d= -f2)

echo "   e1 (15) = $E1"
echo "   e2 (27) = $E2"
echo "   sum = $SUM"
echo "   mul = $MUL"
echo ""

echo "[3] Sending to server for verification..."
curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d "{\"action\":\"fhe_add\",\"client_id\":\"live_test\",\"e1\":$E1,\"e2\":$E2}" | jq '.encrypted_result'

curl -s -X POST http://localhost:8092/ \
  -H "Content-Type: application/json" \
  -d "{\"action\":\"fhe_multiply\",\"client_id\":\"live_test\",\"e1\":$E1,\"e2\":$E2}" | jq '.encrypted_result'

echo ""
echo "[4] Decrypting results..."
node -e "
const { FEmmgClient } = require('./npm-package/index.js');
const client = new FEmmgClient();
const e1 = $E1;
const e2 = $E2;
const sum = client.add(e1, e2);
const mul = client.multiply(e1, e2);
console.log('15 + 27 = ' + client.decrypt(sum));
console.log('15 × 27 = ' + client.decrypt(mul));
"

rm temp_cipher.txt
