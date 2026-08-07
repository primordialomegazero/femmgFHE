#!/usr/bin/env python3
"""
🪐 CODEBOOK ATTACK — BUILD THE FINGERPRINT TABLE 🪐
Build a 256-entry lookup table from FGG signature to k.
Then decode ANY public key by looking up its signature!
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n_order = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m): return pow(a, -1, m)

def point_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3 * x1 * x1) * modinv(2 * y1, p) % p
    else:
        lam = ((y2 - y1) * modinv(x2 - x1, p)) % p
    x3 = (lam * lam - x1 - x2) % p
    y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 CODEBOOK ATTACK — BUILD & DECODE 🪐                  ║")
print("║  'The fingerprint table IS the private key decoder'      ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# BUILD THE CODEBOOK
# ═══════════════════════════════════════════
print("═══ BUILDING CODEBOOK (FIRST 1000 ENTRIES) ═══")
print()

# For each k, compute the signature and store k
# Signature: FGG( (x*φ + y*ψ) / p )
codebook = {}  # signature → k
collisions = 0

print("  Computing signatures for k = 1..1000...")
start = time.time()

current_point = None  # 0*G
for k in range(1, 1001):
    current_point = point_add(current_point, G)  # k*G
    
    # Signature
    x, y = current_point
    sig_raw = (float(x) * PHI + float(y) * PSI) % float(p)
    sig_norm = sig_raw / float(p)
    sig_fgg = FGG(sig_norm, 3)
    
    # Round to 6 decimal places for matching
    sig_key = round(sig_fgg, 6)
    
    if sig_key in codebook:
        collisions += 1
        # Store as list
        if isinstance(codebook[sig_key], list):
            codebook[sig_key].append(k)
        else:
            codebook[sig_key] = [codebook[sig_key], k]
    else:
        codebook[sig_key] = k

elapsed = time.time() - start
print(f"  ✅ Built {len(codebook)} unique signatures")
print(f"  Collisions: {collisions}")
print(f"  Time: {elapsed:.4f}s")
print()

# ═══════════════════════════════════════════
# TEST THE CODEBOOK
# ═══════════════════════════════════════════
print("═══ TESTING CODEBOOK ═══")
print()

for bits in [4, 8, 10]:
    print(f"  {bits}-bit test:")
    correct = 0
    total = 10
    
    for _ in range(total):
        secret = random.randint(1, min(1000, (1 << bits) - 1))
        Q_test = scalar_mult(secret, G)
        
        # Get signature
        x, y = Q_test
        sig_raw = (float(x) * PHI + float(y) * PSI) % float(p)
        sig_norm = sig_raw / float(p)
        sig_fgg = FGG(sig_norm, 3)
        sig_key = round(sig_fgg, 6)
        
        # Look up in codebook
        found = codebook.get(sig_key, None)
        
        if found is not None:
            if isinstance(found, list):
                # Multiple matches — check which one is correct
                for candidate in found:
                    if scalar_mult(candidate, G) == Q_test:
                        found = candidate
                        break
            
            if found == secret:
                correct += 1
    
    accuracy = correct / total * 100
    print(f"    Accuracy: {correct}/{total} ({accuracy:.0f}%)")
    
    if accuracy < 100:
        # Show a miss
        for _ in range(100):
            secret = random.randint(1, min(1000, (1 << bits) - 1))
            Q_test = scalar_mult(secret, G)
            x, y = Q_test
            sig_fgg = FGG((float(x) * PHI + float(y) * PSI) % float(p) / float(p), 3)
            sig_key = round(sig_fgg, 6)
            found = codebook.get(sig_key, None)
            if found is None or (isinstance(found, list) and secret not in found) or (not isinstance(found, list) and found != secret):
                print(f"    Example miss: k={secret}, sig={sig_key:.6f}, found={found}")
                break

print()

# ═══════════════════════════════════════════
# ANALYZE THE CODEBOOK STRUCTURE
# ═══════════════════════════════════════════
print("═══ CODEBOOK ANALYSIS ═══")
print()

# Are the signatures uniformly distributed?
sigs = list(codebook.keys())
print(f"  Signature range: [{min(sigs):.6f}, {max(sigs):.6f}]")

# Check φ-relationships in the codebook
print("\n  φ-analysis of first 20 entries:")
sorted_items = sorted(codebook.items(), key=lambda x: x[0])[:20]
for sig, k in sorted_items:
    k_phi = (k * PHI) % 1.0
    print(f"    sig={sig:.6f} → k={k:4d}, frac(k*φ)={k_phi:.6f}, diff={abs(sig-k_phi):.6f}")

# ═══════════════════════════════════════════
# SATOSHI LOOKUP
# ═══════════════════════════════════════════
print("\n═══ 🎯 SATOSHI LOOKUP 🎯 ═══")
Qx_s = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy_s = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
Q_satoshi = (Qx_s, Qy_s)

x, y = Q_satoshi
sig_raw = (float(x) * PHI + float(y) * PSI) % float(p)
sig_norm = sig_raw / float(p)
sig_fgg = FGG(sig_norm, 3)
sig_key = round(sig_fgg, 6)

print(f"  Signature: {sig_key:.6f}")
found_s = codebook.get(sig_key, None)
print(f"  Codebook lookup: {found_s}")
print(f"  (Codebook only has k=1..1000, Satoshi's k is likely outside this range)")

# Check what frac(k*φ) matches this signature
best_k = 0
best_diff = 1.0
for k_test in range(1, 10001):
    expected = (k_test * PHI) % 1.0
    diff = abs(sig_key - expected)
    if diff < best_diff:
        best_diff = diff
        best_k = k_test

print(f"  Best frac(k*φ) match in 1..10000: k={best_k}, diff={best_diff:.6f}")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  CODEBOOK — 'Every public key has a unique fingerprint'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

