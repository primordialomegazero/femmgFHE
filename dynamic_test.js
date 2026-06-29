const { FEmmgClient } = require('./npm-package/index.js');
const client = new FEmmgClient();

console.log("=========================================");
console.log("🔥 DYNAMIC TEST - WALANG HARDCORE! 🔥");
console.log("=========================================\n");

// Test 1: Same plaintext, different ciphertexts?
console.log("[1] IND-CPA Test: Encrypt 42 three times");
const e1 = client.encrypt(42);
const e2 = client.encrypt(42);
const e3 = client.encrypt(42);
console.log(`   Ciphertext 1: ${e1}`);
console.log(`   Ciphertext 2: ${e2}`);
console.log(`   Ciphertext 3: ${e3}`);
console.log(`   ${(e1===e2 && e2===e3) ? '❌ SAME! Not IND-CPA secure!' : '✅ Different! IND-CPA maybe secure!'}`);
console.log("");

// Test 2: Addition
console.log("[2] Addition Test: 15 + 27 = ?");
const enc15 = client.encrypt(15);
const enc27 = client.encrypt(27);
const addResult = client.add(enc15, enc27);
const decAdd = client.decrypt(addResult);
console.log(`   15 + 27 = ${decAdd}`);
console.log(`   ${decAdd === 42 ? '✅' : '❌'}`);
console.log("");

// Test 3: Multiplication
console.log("[3] Multiplication Test: 6 × 7 = ?");
const enc6 = client.encrypt(6);
const enc7 = client.encrypt(7);
const mulResult = client.multiply(enc6, enc7);
const decMul = client.decrypt(mulResult);
console.log(`   6 × 7 = ${decMul}`);
console.log(`   ${decMul === 42 ? '✅' : '❌'}`);
console.log("");

// Test 4: Complex operation
console.log("[4] Complex: (5 + 3) × 2 = ?");
const enc5 = client.encrypt(5);
const enc3 = client.encrypt(3);
const enc2 = client.encrypt(2);
const sum = client.add(enc5, enc3);
const prod = client.multiply(sum, enc2);
const decComplex = client.decrypt(prod);
console.log(`   (5 + 3) × 2 = ${decComplex}`);
console.log(`   ${decComplex === 16 ? '✅' : '❌'}`);
console.log("");

// Test 5: Floating point
console.log("[5] Floating Point: 3.14159 × 2 = ?");
const encPi = client.encrypt(3.14159);
const encTwo = client.encrypt(2);
const piMul = client.multiply(encPi, encTwo);
const decPi = client.decrypt(piMul);
console.log(`   3.14159 × 2 = ${decPi.toFixed(5)}`);
console.log(`   ${decPi.toFixed(5) === '6.28318' ? '✅' : '❌'}`);
console.log("");

// Test 6: Chain operations (no bootstrapping daw!)
console.log("[6] Chain Test: 2^8 without bootstrapping");
let result = client.encrypt(2);
for (let i = 1; i < 8; i++) {
    result = client.multiply(result, client.encrypt(2));
}
const decChain = client.decrypt(result);
console.log(`   2^8 = ${decChain}`);
console.log(`   ${decChain === 256 ? '✅' : '❌'}`);
console.log("");

// Test 7: Randomness test (generate fresh every time)
console.log("[7] Randomness Test: Encrypt random values");
const randVals = [];
for (let i = 0; i < 5; i++) {
    const val = Math.floor(Math.random() * 1000);
    const enc = client.encrypt(val);
    const dec = client.decrypt(enc);
    randVals.push({ plain: val, cipher: enc, decrypted: dec });
    console.log(`   ${val} → ${enc} → ${dec}`);
}
const allMatch = randVals.every(v => v.plain === v.decrypted);
console.log(`   ${allMatch ? '✅ All matched!' : '❌ Something wrong!'}`);
console.log("");

// Test 8: THE KILLER TEST - Can we extract constants?
console.log("[8] 🔥 CONSTANT EXTRACTION TEST 🔥");
console.log("   Encrypting 0...");
const encZero = client.encrypt(0);
console.log(`   Ciphertext for 0: ${encZero}`);
console.log("   Since encryption is e = m*PHI + LAMBDA + nonce");
console.log("   For m=0, e = LAMBDA + nonce");
console.log("   The 'random' nonce makes this different each time");
console.log("   BUT... if we decrypt, we get back 0!");
console.log("");

// Test 9: Associativity
console.log("[9] Associativity Test: (a+b)+c == a+(b+c)");
const a = client.encrypt(10);
const b = client.encrypt(20);
const c = client.encrypt(30);
const ab = client.add(a, b);
const abc1 = client.add(ab, c);
const bc = client.add(b, c);
const abc2 = client.add(a, bc);
const d1 = client.decrypt(abc1);
const d2 = client.decrypt(abc2);
console.log(`   (10+20)+30 = ${d1}`);
console.log(`   10+(20+30) = ${d2}`);
console.log(`   ${d1 === d2 ? '✅ Associative!' : '❌ NOT associative!'}`);
console.log("");

// Test 10: Distributivity
console.log("[10] Distributivity Test: a×(b+c) == a×b + a×c");
const va = client.encrypt(5);
const vb = client.encrypt(3);
const vc = client.encrypt(4);
const left = client.multiply(va, client.add(vb, vc));
const right = client.add(client.multiply(va, vb), client.multiply(va, vc));
const dl = client.decrypt(left);
const dr = client.decrypt(right);
console.log(`   5×(3+4) = ${dl}`);
console.log(`   5×3 + 5×4 = ${dr}`);
console.log(`   ${dl === dr ? '✅ Distributive!' : '❌ NOT distributive!'}`);
console.log("");

console.log("=========================================");
console.log("📊 SUMMARY:");
console.log("=========================================");
console.log("✅ Works for arithmetic operations");
console.log("❓ IND-CPA: " + (e1===e2 && e2===e3 ? "❌ NO (deterministic!)" : "✅ YES (randomized!)"));
console.log("❓ The 'random nonce' is generated client-side");
console.log("❓ The server only sees Phase 2 (zero nonce)");
console.log("❓ So the server sees deterministic encryption!");
console.log("");
console.log("💀 BOTTOM LINE:");
console.log("   This is a cool arithmetic library.");
console.log("   This is NOT Fully Homomorphic Encryption.");
console.log("   Real FHE uses polynomials, not golden ratios.");
console.log("");
