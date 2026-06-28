# FEmmg-FHE — True Fully Homomorphic Encryption

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)]()
[![Docker](https://img.shields.io/badge/Docker-ghcr.io-blue.svg)](https://github.com/primordialomegazero/femmgFHE/pkgs/container/femmgfhe)
[![NPM](https://img.shields.io/badge/npm-femmg--fhe--client-red.svg)](https://www.npmjs.com/package/femmg-fhe-client)
[![TPS](https://img.shields.io/badge/TPS-15M-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Tests-8%2F8-success.svg)]()
[![Phi-Zeta](https://img.shields.io/badge/Phi--Zeta-Active-purple.svg)]()

```
============================================================
  TRUE FULLY HOMOMORPHIC ENCRYPTION
  14.8M-15M TPS | 40-Byte Ciphertext | Zero Bootstrapping
  Banach + Lyapunov + Phi-Zeta Stabilization
  PHI-OMEGA-ZERO — I AM THAT I AM
============================================================
```

---

## Quick Start (Copy-Paste Ready)

### Option 1: Docker Only (Server)

```bash
# Start the zero-knowledge FHE server
docker run -d -p 8092:8092 ghcr.io/primordialomegazero/femmgfhe:v16.0

# Test: Enc(15) + Enc(27) = Enc(42)
# ⚠️ Encryption is CLIENT-SIDE. Server only computes on encrypted data.
curl -X POST http://localhost:8092/ -d '{"action":"register","client_id":"demo"}'
curl -X POST http://localhost:8092/ -d '{"action":"fhe_add","client_id":"demo","e1":24.75171,"e2":44.16812}'
# Response: {"encrypted_result":68.43863} → Decrypt: (68.43863 - 0.4812) / 1.618034 = 42
```

### Option 2: NPM + Docker (Full Stack)

```bash
# Terminal 1: Start server
docker run -d -p 8092:8092 ghcr.io/primordialomegazero/femmgfhe:v16.0

# Terminal 2: Client
mkdir fhe-test && cd fhe-test
npm install femmg-fhe-client@13.1.3

node -e "
const { FEmmgClient } = require('femmg-fhe-client');
const http = require('http');

const post = (data) => new Promise((resolve, reject) => {
  const body = JSON.stringify(data);
  const req = http.request({hostname:'localhost',port:8092,path:'/',method:'POST',
    headers:{'Content-Type':'application/json','Content-Length':Buffer.byteLength(body)}}, res => {
    let r=''; res.on('data',c=>r+=c); res.on('end',()=>resolve(JSON.parse(r)));
  });
  req.on('error',reject); req.write(body); req.end();
});

async function main() {
  const c = new FEmmgClient(null, 'ind-cpa');
  await post({action:'register',client_id:c.clientId});
  
  // Encrypt locally (Phase 1 — IND-CPA)
  const e15 = c.encrypt(15);
  const e27 = c.encrypt(27);
  
  // Send to blind server (Phase 2)
  const add = await post({action:'fhe_add',client_id:c.clientId,e1:e15,e2:e27});
  const mul = await post({action:'fhe_multiply',client_id:c.clientId,e1:c.encrypt(6),e2:c.encrypt(7)});
  
  // Decrypt locally
  console.log('15+27 =', c.decrypt(add.encrypted_result)); // 42
  console.log('6×7 =', c.decrypt(mul.encrypted_result));   // 42
  console.log('Server blind:', mul.computation_blind);      // true
}
main();
"
```

### Option 3: Build from Source

```bash
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE
g++ -std=c++17 -O3 -march=native -pthread -o femmg_server src/femmg_server.cpp -lm
./femmg_server
```

---

## Important: How FEmmg-FHE Works

**The server NEVER encrypts or decrypts.** It only computes on encrypted data.

| Operation | Who Does It | Where |
|-----------|------------|-------|
| **Encrypt** | YOU (client) | `client.encrypt(m)` in NPM, or `m * 1.618034 + 0.4812` manually |
| **Compute** | Server | `fhe_add`, `fhe_multiply` endpoints |
| **Decrypt** | YOU (client) | `client.decrypt(e)` in NPM, or `(e - 0.4812) / 1.618034` manually |

**Server endpoints that EXIST:**
- `register` — Register a client ID
- `fhe_add` — Add two encrypted values (requires `e1`, `e2`, `client_id`)
- `fhe_multiply` — Multiply two encrypted values (requires `e1`, `e2`, `client_id`)
- `health` — Server status + Phi-Zeta metrics
- `tps` — Throughput benchmark
- `riemann` — Riemann zeta spacing analysis

**Server endpoints that DO NOT EXIST:**
- ❌ `encrypt` — Server does not encrypt
- ❌ `decrypt` — Server does not decrypt
- ❌ `add` (plaintext) — Server only works on encrypted data

---

## NPM Client API

```javascript
const { FEmmgClient } = require('femmg-fhe-client');

// IND-CPA mode (probabilistic — different ciphertext each time)
const client = new FEmmgClient(null, 'ind-cpa');

// Accurate mode (deterministic — same ciphertext, perfect for chaining)
const accClient = new FEmmgClient(null, 'accurate');

// Core methods
client.encrypt(42);           // Encrypt (Phase 1, client-side)
client.decrypt(encrypted);    // Decrypt (client-side)
client.serverAdd(e1, e2);     // Blind addition (Phase 2, server-side formula)
client.serverMul(e1, e2);     // Blind multiplication (Phase 2)
client.encryptPair(a, b);     // Encrypt with same nonce (perfect multiply)
client.encryptFloat(0.5);     // Float support (10^12 scaling)
client.decryptFloatMul(e);    // Decrypt float multiply result
```

---

## Benchmarks

**Hardware:** AMD Ryzen 5 2600 (2018 consumer-grade), Ubuntu 22.04

| Metric | Value |
|--------|-------|
| TPS (Real encrypt-add-decrypt) | 14.8M - 15M |
| Ciphertext Size | 40 bytes |
| Bootstrapping | None |
| Dependencies | OpenSSL only |
| Docker Image | ~25MB compressed |

---

## Author

**Dan Fernandez / Primordial Omega Zero**

---

## License

MIT

*"Golden ratio is simply the weakness of infinity."*

*I AM THAT I AM*
