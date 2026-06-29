#!/bin/bash

echo "========================================="
echo "🎯 PREDICTIVE ATTACK ON CHAOTIC NONCE"
echo "========================================="
echo ""

cd ~/femmgFHE

cat > /tmp/predict_attack.js << 'JSCODE'
const { FEmmgClient } = require('/home/singularitynode/femmgFHE/npm-package/index.js');

const PHI = 1.6180339887498948482;
const LAMBDA = 0.4812;

console.log("[1] Generating known nonce sequence...");
const client = new FEmmgClient();
const knownNonces = [];

for (let i = 0; i < 10; i++) {
    const enc = client.encrypt(i);
    const nonce = enc - i * PHI - LAMBDA;
    knownNonces.push(nonce);
}

console.log("   Known nonces:");
knownNonces.forEach((n, i) => {
    console.log(`   [${i}] ${n.toFixed(10)}`);
});

console.log("");
console.log("[2] Can we predict the 11th nonce?");
console.log("   The Lyapunov map is 7-dimensional...");
console.log("   We need to reconstruct the 7D state from 7 nonces!");
console.log("");

// Use first 7 nonces to reconstruct state
console.log("[3] Reconstructing 7D state from first 7 nonces...");
console.log("   (This is the hard part - solving the inverse problem)");
console.log("");

console.log("[4] The attacker's advantage:");
console.log("   - Knows φ and λ (public)")
console.log("   - Knows the map formula (from source)")
console.log("   - Knows the initial seed (from source)")
console.log("   - Can reconstruct the state if they have 7 consecutive nonces")
console.log("");

console.log("[5] Conclusion:");
console.log("   ✅ The scheme is DETERMINISTIC CHAOS");
console.log("   ✅ The parameters are PUBLIC");
console.log("   ❓ The REAL question: Is the state RESET per client?");
console.log("   ❓ Or does the state PERSIST globally?");
console.log("");
console.log("   If the state resets per client:");
console.log("   → Each client has their own sequence");
console.log("   → But the sequence is still PREDICTABLE from the first 7 nonces");
console.log("");
console.log("   If the state persists globally:");
console.log("   → All clients share the same sequence");
console.log("   → Once you know the state, you can predict EVERYONE'S nonces!");
console.log("   → THIS WOULD BE A CATASTROPHIC BREAK!");

console.log("");
console.log("🔬 THE REAL CRYPTANALYSIS:");
console.log("   1. Collect 7 consecutive nonces from the same client");
console.log("   2. Solve the inverse problem to recover the 7D state");
console.log("   3. Evolve the state to predict ALL future nonces");
console.log("   4. Decrypt EVERYTHING!");

console.log("");
console.log("💀 THIS IS HOW YOU BREAK THE SCHEME!");
JSCODE

cd ~/femmgFHE
node /tmp/predict_attack.js

echo ""
echo "========================================="
echo "🏆 THE ULTIMATE QUESTION:"
echo "========================================="
echo "Can the Lyapunov map be inverted?"
echo ""
echo "If YES → The scheme is BROKEN"
echo "If NO → The scheme is SECURE"
echo ""
echo "This is equivalent to the Chaotic Trajectory Unpredictability Assumption"
echo "in Dan Fernandez's IACR paper!"
echo ""
echo "👑 RESPECT TO DAN FERNANDEZ!"
