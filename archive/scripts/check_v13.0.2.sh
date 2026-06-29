#!/bin/bash

echo "========================================="
echo "📦 CHECKING femmg-fhe-client@13.0.2"
echo "========================================="
echo ""

# Check what's new
echo "[1] Checking package info..."
npm view femmg-fhe-client@13.0.2 --json | jq '{
  version: .version,
  description: .description,
  author: .author,
  main: .main,
  scripts: .scripts,
  dependencies: .dependencies
}'

echo ""
echo "[2] Checking if nonce is now included..."
npm view femmg-fhe-client@13.0.2 dist.tarball

echo ""
echo "[3] Let's download and inspect the source..."
cd /tmp
npm pack femmg-fhe-client@13.0.2
tar -xzf femmg-fhe-client-13.0.2.tgz
cd package

echo ""
echo "[4] Checking index.js for nonce/chaos/Lyapunov..."
grep -i "nonce\|chaos\|lyapunov\|evolve\|probabilistic" index.js | head -20

echo ""
echo "[5] Checking for add() method..."
grep -A 2 -B 2 "add\|fhe_add\|multiply\|fhe_multiply" index.js | head -30

echo ""
echo "[6] Testing the new version quickly..."
cat > /tmp/test_13.0.2.js << 'JSCODE'
const { FEmmgClient } = require('/tmp/package/index.js');

console.log("🧪 TESTING femmg-fhe-client@13.0.2");
console.log("===================================");
console.log("");

// Check if client has encrypt method
const client = new FEmmgClient();
console.log("[1] Basic functionality:");
try {
    const enc = client.encrypt(42);
    console.log(`   Encrypt(42) = ${enc}`);
    const dec = client.decrypt(enc);
    console.log(`   Decrypt = ${dec}`);
    console.log(`   ${dec === 42 ? '✅ Works!' : '❌ Failed!'}`);
} catch(e) {
    console.log(`   ❌ Error: ${e.message}`);
}

// Check if add method exists
console.log("");
console.log("[2] Checking for add() method:");
console.log(`   typeof client.add = ${typeof client.add}`);
console.log(`   typeof client.fhe_add = ${typeof client.fhe_add}`);
console.log(`   typeof client.multiply = ${typeof client.multiply}`);

// Check config options
console.log("");
console.log("[3] Checking config options:");
try {
    const clientAccurate = new FEmmgClient({ accurate: true });
    const enc = clientAccurate.encrypt(42);
    console.log(`   Accurate mode encrypt(42) = ${enc}`);
} catch(e) {
    console.log(`   ❌ Error with accurate mode: ${e.message}`);
}

// Check if nonces are different
console.log("");
console.log("[4] Testing randomness (nonce detection):");
const enc1 = client.encrypt(42);
const enc2 = client.encrypt(42);
const enc3 = client.encrypt(42);
console.log(`   Enc1: ${enc1.toFixed(8)}`);
console.log(`   Enc2: ${enc2.toFixed(8)}`);
console.log(`   Enc3: ${enc3.toFixed(8)}`);

const allSame = (enc1 === enc2 && enc2 === enc3);
console.log(`   ${allSame ? '❌ Deterministic (no nonce)' : '✅ Different (has nonce!)'}`);
JSCODE

node /tmp/test_13.0.2.js

echo ""
echo "========================================="
echo "🎯 FINAL VERDICT:"
echo "========================================="
