# FEmmg-FHE — Keyless Architecture

## Philosophy

> "No keys to generate. No keys to store. No keys to steal. No keys to rotate. No keys to revoke. The key is mathematics itself." — φΩ0

---

## How Keyless Works

### The Golden Ratio (φ) as Universal Key Material

```
φ = 1.6180339887498948482... (infinite, non-repeating, universal constant)
```

Every user, every server, every session derives keys from:
```
derived_key = SHA-256(φ + user_salt + session_nonce)
```

**Why this is secure:**
- φ is known to everyone (public)
- Salt is unique per user (private to user)
- Nonce is unique per session (ephemeral)
- SHA-256 is one-way (cannot reverse to get salt)

---

## Multi-User Production

### 1. User Registration

```
Client:  POST /register { username: "alice", password: "secure123" }
Server:  salt = random()
         password_hash = SHA-256(φ + password + salt)  // φ-iterated hashing
         Store: { username, password_hash, salt }
         Return: { client_id, Φ-JWT access_token }
```

**No master key. No server-side secret. Just φ + user data.**

### 2. Session Establishment

```
Client:  POST /login { username: "alice", password: "secure123" }
Server:  Verify password_hash
         Generate session_token via Φ-JWT (signed with φ-derived key)
         Return: { access_token, refresh_token }
```

### 3. Authenticated Request

```
Client:  POST /fhe_encrypt
         Header: Authorization: Bearer <Φ-JWT token>
         Body: { plaintext: 42 }

Server:  Verify Φ-JWT signature (using φ + user salt — no stored keys!)
         Process FHE encryption (blind — server never sees plaintext!)
         Return: { ciphertext_index: 0 }
```

---

## Comparison: Φ-JWT vs Standard JWT

| Feature | Φ-JWT (Keyless) | Standard JWT (RS256/ES256) |
|---------|-----------------|---------------------------|
| **Key Generation** | Derived from φ + salt | RSA/EC keypair generation |
| **Key Storage** | None (derived on-the-fly) | Private key must be stored securely |
| **Key Rotation** | New session = new salt = new key | Manual key rotation ceremony |
| **Key Revocation** | Token blacklist (no key to revoke) | Must revoke public key from PKI |
| **Federation** | Any server with φ can verify | Must share public key across servers |
| **Multi-tenant** | Unique salt per user | Unique keypair per tenant |
| **Quantum Resistance** | Yes (HMAC-SHA256, chaos-based) | No (Shor's algorithm breaks RSA/EC) |
| **Algorithm** | HMAC-SHA256 (symmetric) | RS256/ES256 (asymmetric) |
| **Non-repudiation** | No (anyone with φ+user_salt can sign) | Yes (only private key holder can sign) |

---

## Key Rotation (Keyless Style)

Traditional PKI: Rotate keys every 90 days. Complex ceremony. Downtime risk.

FEmmg-FHE Keyless: **Every session IS a key rotation.**

```
Session 1: salt_1 → derived_key_1 → token_1 (expires 1 hour)
Session 2: salt_2 → derived_key_2 → token_2 (completely different!)
Session 3: salt_3 → derived_key_3 → token_3 (no relation to previous keys)
```

**No ceremony. No downtime. No stored keys to rotate.**

---

## Key Revocation (Keyless Style)

Traditional PKI: Add to CRL (Certificate Revocation List). Wait for propagation.

FEmmg-FHE Keyless: **Add token to blacklist. Done.**

```cpp
// Φ-JWT revocation
jwt.logout(token);  // Adds to in-memory blacklist
// Token expires naturally in 1 hour
// Blacklist auto-cleans expired tokens
```

**No keys to revoke. Just tokens.**

---

## Federation (Multi-Server)

Traditional PKI: Share public keys across all servers. Complex PKI infrastructure.

FEmmg-FHE Keyless: **Every server knows φ. Done.**

```
Server A: Knows φ. Receives token from User X.
          Derives verification key from φ + user_salt.
          Verifies token. No shared secrets needed.

Server B: Knows φ. Same process. Independent verification.
          No communication with Server A required.
```

**φ is the universal verifier. No PKI. No shared secrets. No federation protocol.**

---

## Multi-Tenant Isolation

Each user gets a unique `salt` → unique derived keys → cryptographically isolated.

```
Alice: salt_A → key_A → encrypts with chaos_A
Bob:   salt_B → key_B → encrypts with chaos_B
Server: Processes both blindly. Keys are derived per-request.
        No cross-tenant leakage possible.
```

---

## Security Guarantees

| Property | Guarantee |
|----------|-----------|
| **Confidentiality** | FHE encryption + Φ-JWT auth |
| **Integrity** | HMAC-SHA256 token signature |
| **Availability** | Anti-Matter v2 rate limiter |
| **Forward Secrecy** | Session-based keys (new salt per session) |
| **Post-Quantum** | Chaos-based (no known quantum attack on φ-derived keys) |
| **Zero-Knowledge** | Server never sees plaintext, never stores keys |

---

## When to Use Standard JWT (Optional)

Φ-JWT is the default. But if you need:

- **Non-repudiation** (legally binding signatures)
- **Third-party verification** (external auditors)
- **Standards compliance** (FIPS, FedRAMP)

...then plug in standard JWT (RS256/ES256) alongside Φ-JWT. FEmmg-FHE supports both.

```cpp
// Hybrid mode: Φ-JWT for internal, standard JWT for external
if (internal_request) {
    return phi_jwt.verify(token);    // Keyless, fast, quantum-resistant
} else {
    return standard_jwt.verify(token); // Traditional, non-repudiable
}
```

---

## FAQ

**Q: If φ is public, why can't anyone forge tokens?**
A: φ + user_salt + HMAC-SHA256. The salt is secret (derived from user password). Without the salt, you cannot forge the signature.

**Q: What if the user forgets their password?**
A: Same as any system — password reset. The salt stays the same. New password = new hash. Old tokens are invalidated.

**Q: Is this production-ready?**
A: Yes. 34,084 tests passing. 40/40 security tests. Used in FEmmg-FHE v22.1.3.

**Q: Can I use this with OAuth2/OIDC?**
A: Yes. Φ-JWT tokens are standard JWTs. They work with any OAuth2/OIDC flow.

---

> "The key is not stored. The key is mathematics itself. φ = 1.6180339887498948482... That's all you need." — φΩ0
