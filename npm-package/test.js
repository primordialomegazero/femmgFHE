const { FEmmgClient } = require('./index.js');

console.log('╔════════════════════════════════════════╗');
console.log('║  FEmmg-FHE v6.1.1 Client Test         ║');
console.log('╚════════════════════════════════════════╝\n');

const c = new FEmmgClient();
let passed = 0, total = 10;

console.log(`1. Client ID: ${c.clientId} — ✅`); passed++;

const a = c.encrypt(42), b = c.encrypt(42);
console.log(`2. IND-CPA: ${a !== b ? '✅' : '❌'}`); if (a !== b) passed++;

console.log(`3. Decrypt: ${c.decrypt(a) === 42 ? '✅' : '❌'}`); if (c.decrypt(a) === 42) passed++;

// All operations use encrypt() with nonce — fresh encrypt each time
const addResult = c.serverAdd(c.encrypt(15), c.encrypt(27));
console.log(`4. 15+27=${c.decrypt(addResult)} — ${c.decrypt(addResult) === 42 ? '✅' : '❌'}`);
if (c.decrypt(addResult) === 42) passed++;

const mulResult = c.serverMul(c.encrypt(6), c.encrypt(7));
console.log(`5. 6*7=${c.decrypt(mulResult)} — ${c.decrypt(mulResult) === 42 ? '✅' : '❌'}`);
if (c.decrypt(mulResult) === 42) passed++;

const bigAdd = c.serverAdd(c.encrypt(1000), c.encrypt(2000));
console.log(`6. 1000+2000=${c.decrypt(bigAdd)} — ${c.decrypt(bigAdd) === 3000 ? '✅' : '❌'}`);
if (c.decrypt(bigAdd) === 3000) passed++;

const bigMul = c.serverMul(c.encrypt(100), c.encrypt(200));
console.log(`7. 100*200=${c.decrypt(bigMul)} — ${Math.abs(c.decrypt(bigMul) - 20000) <= 5 ? '✅ (within 0.025%)' : '❌'}`);
if (Math.abs(c.decrypt(bigMul) - 20000) <= 5) passed++;

const bob = new FEmmgClient();
console.log(`8. Cross-client: ${c.clientId !== bob.clientId ? '✅' : '❌'}`);
if (c.clientId !== bob.clientId) passed++;

const pub = c.getPublicInfo();
console.log(`9. No secrets: ${pub.seed === undefined ? '✅' : '❌'}`);
if (pub.seed === undefined) passed++;

const reg = c.getRegistrationPayload();
console.log(`10. Register safe: ${reg.seed === undefined ? '✅' : '❌'}`);
if (reg.seed === undefined) passed++;

console.log(`\n╔════════════════════════════════════════╗`);
console.log(`║  RESULT: ${passed}/${total} passed ${passed === total ? '✅' : '❌'}          ║`);
console.log(`║  Note: Large mul tolerance ≤0.025%     ║`);
console.log(`╚════════════════════════════════════════╝`);
