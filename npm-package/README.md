# FEmmg-FHE Client v17.4.0

JavaScript client for FEmmg-FHE — True Fully Homomorphic Encryption with Optimal Contraction Coefficient (OCC).

## Install

```bash
npm install femmg-fhe-client@17.4.0
```

## Quick Start

```javascript
const { FEmmgClient } = require('femmg-fhe-client');
const client = new FEmmgClient(); // Default: ind-cpa mode

// Encrypt
const e1 = client.encrypt(42);
const e2 = client.encrypt(7);

// Server-side homomorphic operations
const eAdd = client.serverAdd(e1, e2);
const eMul = client.serverMul(e1, e2);

// Decrypt
console.log(client.decrypt(eAdd)); // 49
console.log(client.decrypt(eMul)); // 294
```

## Features

- **7D Sine-CML IND-CPA**: Chaotic nonce with true random injection
- **Fully Blind Operations**: Server never sees plaintext
- **OCC-Validated**: Optimal Contraction Coefficient (0.618)
- **Zero Dependencies**: Pure JavaScript, crypto.randomBytes only
