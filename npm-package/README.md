# FEmmg-FHE Client Library v6.1

Zero-Knowledge Fully Homomorphic Encryption client library for Node.js and browsers.

## Features

- **Zero-Knowledge:** Server never receives cryptographic keys
- **Probabilistic IND-CPA:** Same plaintext → different ciphertext every time
- **Chaotic Nonce Injection:** Logistic map with positive Lyapunov exponent
- **Homomorphic Add/Multiply:** Compute on encrypted data
- **Multi-Client Isolation:** Each client has unique emergent keys
- **Key Export/Import:** Save and restore client identity
- **Zero Dependencies:** Uses only Node.js `crypto` module

## Install

```bash
npm install femmg-fhe-client
```

## Quick Start

```javascript
const { FEmmgClient } = require('femmg-fhe-client');

// Alice generates her keys (NEVER shared with server)
const alice = new FEmmgClient();

// Register with server (server sees client_id only, NO keys)
const response = await fetch('http://server:8092/', {
  method: 'POST',
  body: JSON.stringify(alice.getRegistrationPayload())
});

// Encrypt data locally
const enc15 = alice.encrypt(15);
const enc27 = alice.encrypt(27);

// Send encrypted data to server for blind computation
const addResult = await fetch('http://server:8092/', {
  method: 'POST',
  body: JSON.stringify(alice.getAddPayload(enc15, enc27))
});
const { encrypted_result } = await addResult.json();

// Decrypt locally
const sum = alice.decrypt(encrypted_result);
console.log(`15 + 27 = ${sum}`); // 42
```

## API Reference

### `new FEmmgClient(phi?)`
Create a new client. If `phi` is not provided, generates random phi ∈ [1.5, 1.7].

### `client.encrypt(message)`
Encrypt an integer. Returns encrypted float value.

### `client.decrypt(encryptedValue)`
Decrypt an encrypted float value. Returns integer.

### `client.getRegistrationPayload()`
Returns payload for `/register` endpoint. **Does NOT contain phi or lambda.**

### `client.getAddPayload(e1, e2)`
Returns payload for `fhe_add` endpoint.

### `client.getMultiplyPayload(e1, e2)`
Returns payload for `fhe_multiply` endpoint.

### `client.getPublicInfo()`
Returns safe-to-share public info. **Does NOT contain phi or lambda.**

### `client.getSecretKeys()`
Returns full key material. **KEEP SECRET!**

### `FEmmgClient.fromKeys(keys)`
Restore client from saved secret keys.

## Security

- Phi and lambda are **never** transmitted to the server
- Registration only sends a `client_id` string
- Server performs blind computation on encrypted data
- Server cannot decrypt any client data
- Each client has cryptographically unique keys

## License

MIT — Dan Joseph M. Fernandez

## Repository

https://github.com/primordialomegazero/femmgFHE
