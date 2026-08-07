#!/usr/bin/env python3
"""
🪐 FINGERPRINT DECODER — FGG(λ) PERMUTATION 🪐
FGG(λ_i/p) is a permutation of frac(i*φ).
Decode the permutation to read k directly!
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

def point_double(P):
    if P is None: return None, 0
    x1, y1 = P
    if y1 == 0: return None, 0
    lam = (3 * x1 * x1) * modinv(2 * y1, p) % p
    x3 = (lam * lam - 2 * x1) % p
    y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3), lam

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FINGERPRINT DECODER — DECODE THE PERMUTATION 🪐      ║")
print("║  'FGG(λ) IS the fingerprint of k'                       ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# BUILD THE FINGERPRINT TABLE
# ═══════════════════════════════════════════
print("═══ BUILDING FINGERPRINT TABLE ═══")
print()

# Compute FGG(λ_i/p) for i = 0..255
fingerprints = {}
current = G

for i in range(256):
    next_point, lam = point_double(current)
    
    # Fingerprint: FGG(λ/p)
    fp = FGG(float(lam) / float(p), 3)
    
    # Expected: frac(i * φ)
    expected = (i * PHI) % 1.0
    
    fingerprints[i] = {
        'lam': lam,
        'fp': fp,
        'expected': expected,
        'diff': abs(fp - expected)
    }
    
    current = next_point

# Check which indices match their expected frac(i*φ)
print("  Matching FGG(λ_i/p) ≈ frac(i*φ):")
matches = []
for i in range(20):
    d = fingerprints[i]['diff']
    if d < 0.05:
        matches.append(i)
        print(f"    i={i:3d}: FGG={fingerprints[i]['fp']:.6f}, "
              f"expected={fingerprints[i]['expected']:.6f}, diff={d:.6f} ✅")

print(f"\n  Found {len(matches)} matches in first 20")
print()

# ═══════════════════════════════════════════
# FIND THE PERMUTATION
# ═══════════════════════════════════════════
print("═══ FINDING THE PERMUTATION ═══")
print()

# The fingerprint at position i should map to some frac(j*φ)
# Find j such that FGG(λ_i/p) ≈ frac(j*φ)

fps = [fingerprints[i]['fp'] for i in range(256)]
expecteds = [(j * PHI) % 1.0 for j in range(256)]

# For each fingerprint, find the closest expected value
permutation = {}
for i in range(20):  # Check first 20
    fp = fps[i]
    best_j = 0
    best_diff = 1.0
    for j in range(20):
        diff = abs(fp - expecteds[j])
        if diff < best_diff:
            best_diff = diff
            best_j = j
    permutation[i] = best_j
    print(f"  i={i:3d} → j={best_j:3d} (diff={best_diff:.6f})")

print()

# ═══════════════════════════════════════════
# CHECK: Is the permutation related to doubling?
# ═══════════════════════════════════════════
print("═══ PERMUTATION ANALYSIS ═══")
print()

# The permutation P: i → j means:
# FGG(λ_i/p) ≈ frac(j*φ)
# where λ_i is the lambda for doubling 2^i * G

# If we can INVERT this: given FGG(λ_Q/p), find j, then i = P^{-1}(j)
# But λ_Q is the lambda IF Q were a doubling of some point
# Q = k*G — we don't double Q directly

# INSTEAD: The fingerprint of Q itself!
# FGG(x_Q * φ + y_Q * ψ) mod p → this is a signature
# This signature should correspond to some frac(k*φ)

print("  Computing signature of Q for different k...")
for k_test in [1, 2, 3, 5, 10]:
    Q_test = scalar_mult(k_test, G)
    if Q_test is None:
        continue
    
    sig = (float(Q_test[0]) * PHI + float(Q_test[1]) * PSI) % float(p)
    sig_norm = sig / float(p)
    fgg_sig = FGG(sig_norm, 3)
    expected_k = (k_test * PHI) % 1.0
    
    print(f"    k={k_test:3d}: FGG(sig(Q)/p)={fgg_sig:.6f}, "
          f"frac(k*φ)={expected_k:.6f}, "
          f"diff={abs(fgg_sig-expected_k):.6f}")

print()

# ═══════════════════════════════════════════
# THE DECODER
# ═══════════════════════════════════════════
print("═══ DECODER TEST ═══")
print()

# Test: can we predict k from FGG(sig(Q)/p)?
for bits in [4, 8]:
    secret = random.randint(1, (1 << bits) - 1)
    Q_secret = scalar_mult(secret, G)
    
    sig = (float(Q_secret[0]) * PHI + float(Q_secret[1]) * PSI) % float(p)
    sig_norm = sig / float(p)
    fgg_sig = FGG(sig_norm, 3)
    
    # Try to find k such that frac(k*φ) ≈ fgg_sig
    best_k = 0
    best_diff = 1.0
    for k_test in range(1, min(10000, (1 << bits) * 2)):
        expected = (k_test * PHI) % 1.0
        diff = abs(fgg_sig - expected)
        if diff < best_diff:
            best_diff = diff
            best_k = k_test
    
    print(f"  Secret: {secret}")
    print(f"    FGG(sig(Q)/p) = {fgg_sig:.6f}")
    print(f"    Best match: k={best_k}, diff={best_diff:.6f}")
    print(f"    {'✅ MATCH!' if best_k == secret else '❌ WRONG'}")
    print()

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  FINGERPRINT — 'FGG(λ) is the Rosetta Stone of ECDLP'    ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

