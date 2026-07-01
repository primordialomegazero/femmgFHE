# FEmmg-FHE Security Hardening Plan

## Priority: 🔴 HIGH (Must fix before production)

### Step 1: Session Authentication
**Risk:** Session spoofing — anyone can impersonate any client.
**Fix:** Add API key authentication.

### Step 2: Rate Limiter Activation
**Risk:** DoS attack — server crash from millions of requests.
**Fix:** Enable Anti-Matter in production mode.

### Step 3: Memory Protection
**Risk:** Plaintext visible in `value_int` if memory is compromised.
**Fix:** Encrypt or obfuscate value_int at rest.

## Priority: 🟡 MEDIUM (Should fix)

### Step 4: Buffer Overflow Audit
**Risk:** Malformed requests could overflow 8192-byte buffer.
**Fix:** Proper bounds checking on recv().

### Step 5: Mathematical Derivation of 1.82815
**Risk:** Noise floor is empirically derived, not mathematically proven.
**Fix:** Derive from Banach fixed point theorem.

## Priority: 🟢 LOW (Nice to have)

### Step 6: Remove Unused Includes
**Risk:** Cosmetic only.
**Fix:** Clean up dead code references.

### Step 7: Formal CTU Security Proof
**Risk:** CTU Assumption not yet peer-reviewed.
**Fix:** Wait for IACR review + CTU Challenge results (Jan 2027).
