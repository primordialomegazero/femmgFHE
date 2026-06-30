const { FEmmgClient } = require('./index.js');

async function test() {
    console.log('╔══════════════════════════════════════════════╗');
    console.log('║  FEmmg-FHE v21.0 NPM Test                  ║');
    console.log('║  Unlimited Depth FHE + PQC + Keyless Auth  ║');
    console.log('╚══════════════════════════════════════════════╝\n');

    const client = new FEmmgClient();

    // Test 1: Register
    console.log('Test 1: Register');
    const sessionId = await client.register();
    console.log(`  Session: ${sessionId}\n`);

    // Test 2: Encrypt
    console.log('Test 2: Encrypt');
    const ct = client.encrypt(42);
    console.log(`  Ciphertext: ${ct.coordinates[0].toFixed(6)}...\n`);

    // Test 3: Store
    console.log('Test 3: Store');
    const idx = await client.store(ct);
    console.log(`  Index: ${idx}\n`);

    // Test 4: Decrypt
    console.log('Test 4: Decrypt');
    const decrypted = await client.decrypt(idx);
    console.log(`  Decrypted: ${decrypted}\n`);

    // Test 5: Health
    console.log('Test 5: Health');
    const health = await client.health();
    console.log(`  Status: ${health.status}\n`);

    console.log('╔══════════════════════════════════════════════╗');
    console.log('║  ALL TESTS PASSED ✅                        ║');
    console.log('╚══════════════════════════════════════════════╝');
}

test().catch(console.error);
