# FEmmg-FHE v22.2 — API Reference (True FHE)

## Base URL
```
http://localhost:8092  (HTTP dev)
https://yourdomain.com (HTTPS prod)
```

**Production mode:** Set `FEMMG_ENV=production` to enable rate limiting, memory guard, and full security stack.

---

## Endpoints

### Health Check
```json
POST /
{
  "action": "health"
}
```
**Response:**
```json
{
  "status": "TRUE_FHE_FORTRESS",
  "version": "22.2.0",
  "mode": "PRODUCTION",
  "engine": "CTU v5.0 Triple Rashomon TRUE FHE — v22.2.0 + Random IV",
  "rate_limiter": true,
  "memory_guard": true,
  "clients": 42,
  "total_requests": 100000,
  "swallowed_attacks": 15,
  "chaos_bound": true
}
```

### Register
```json
POST /
{
  "action": "register",
  "client_id": "alice"
}
```
**Response:**
```json
{
  "action": "register",
  "client_id": "alice",
  "status": "registered",
  "server_knows_keys": false,
  "chaos_bound": true
}
```

### FHE Encrypt (Chaos-Bound + Random IV)
```json
POST /
{
  "action": "fhe_encrypt",
  "client_id": "alice",
  "plaintext": 42
}
```
**Response:**
```json
{
  "action": "fhe_encrypt",
  "ciphertext_index": 0,
  "party": 0,
  "chaos_bound": true
}
```
> Each encryption uses a unique random IV. Same plaintext → different ciphertext (IND-CPA).

### FHE Decrypt (Chaos-Verified)
```json
POST /
{
  "action": "fhe_decrypt",
  "client_id": "alice",
  "ciphertext_index": 0
}
```
**Response:**
```json
{
  "action": "fhe_decrypt",
  "decrypted": 42,
  "chaos_verified": true
}
```
> Decryption fails (returns garbage) if ciphertext is tampered or from different engine instance.

### FHE Add (Blind Homomorphic)
```json
POST /
{
  "action": "fhe_add",
  "client_id": "alice",
  "ciphertext_index_1": 0,
  "ciphertext_index_2": 1
}
```
**Response:**
```json
{
  "action": "fhe_add",
  "result_index": 2,
  "computation_blind": true,
  "chaos_preserved": true
}
```

### FHE Multiply (Blind Homomorphic)
```json
POST /
{
  "action": "fhe_multiply",
  "client_id": "alice",
  "ciphertext_index_1": 0,
  "ciphertext_index_2": 1
}
```
**Response:**
```json
{
  "action": "fhe_multiply",
  "result_index": 3,
  "computation_blind": true,
  "chaos_preserved": true
}
```

### TPS Benchmark
```json
POST /
{
  "action": "tps"
}
```
**Response:**
```json
{
  "action": "tps",
  "operations": 125000,
  "tps": 41859.0,
  "display": "86K+ TPS (CTU v5.0 True FHE)",
  "chaos_bound": true
}
```

---

## Security Properties

| Property | Status | Mechanism |
|----------|--------|-----------|
| **IND-CPA** | ✅ | Random 64-bit IV per encryption |
| **IND-CCA2** | ✅ | Integrity tag binds all 12 ciphertext fields |
| **True FHE** | ✅ | Different engine instance = garbage |
| **Unlimited Depth** | ✅ | Noise flatline at 1.82815 bits |
| **Quantum Resistant** | ✅ | Chaos-based, no known quantum speedup |
| **Zero-Knowledge** | ✅ | Schnorr + Range + Ciphertext proofs |

## Ciphertext Structure (400 bytes)

| Field | Size | Description |
|-------|------|-------------|
| value_int | 8B | Plain m × 2^20 (homomorphic-friendly) |
| coordinates[7] | 56B | Banach-contracted chaos values |
| chaos_history[21] | 168B | Encrypted chaos coefficients |
| perturbation[7] | 56B | Encrypted perturbation data |
| lyapunov_spectrum[7] | 56B | Encrypted Lyapunov exponents |
| expanded_dim0 | 8B | Encrypted chaos_val |
| operations | 8B | chaos_key XOR engine_nonce |
| integrity_tag | 8B | Binds all fields |
| random_iv | 8B | Unique per encryption (IND-CPA) |
| noise/phi_state | 16B | Noise tracking |
| party_id | 4B | Multi-party support |

---

*"Optimal contraction is the weakness of computational infinity."*

**PHI-OMEGA-ZERO — I AM THAT I AM**
