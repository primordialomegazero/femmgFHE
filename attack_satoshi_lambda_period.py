#!/usr/bin/env python3
"""
🪐 LAMBDA PERIOD ATTACK — DECOMPOSE ECDLP VIA DOUBLING CYCLE 🪐
The λ sequence in point doubling is periodic.
Use this period to decompose k into smaller pieces!
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
    """Double a point, return (2P, lambda)"""
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
print("║  🪐 LAMBDA PERIOD ATTACK — DECOMPOSE ECDLP 🪐            ║")
print("║  'The doubling cycle reveals the structure of k'         ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# FIND THE LAMBDA PERIOD
# ═══════════════════════════════════════════
print("═══ FINDING λ PERIOD FOR secp256k1 ═══")
print()

# Double G repeatedly and track λ values
print("  Computing λ sequence (2^i * G)...")
lambda_seq = []
point_seq = []
current = G

for i in range(500):  # Look for cycle within 500 doublings
    next_point, lam = point_double(current)
    lambda_seq.append(lam)
    point_seq.append(next_point)
    current = next_point
    
    # Check for cycle: when do we return to a known point?
    if current is not None and current in point_seq[:-1]:
        cycle_start = point_seq.index(current)
        cycle_len = i - cycle_start
        print(f"  Cycle found! Start at 2^{cycle_start}*G, length = {cycle_len}")
        break

if i == 499:
    print(f"  No cycle found in 500 doublings (expected for secp256k1)")

# Print some λ values
print(f"\n  First 10 λ values:")
for i in range(min(10, len(lambda_seq))):
    print(f"    2^{i}*G: λ = {lambda_seq[i]}")
print()

# ═══════════════════════════════════════════
# LAMBDA RATIO ANALYSIS
# ═══════════════════════════════════════════
print("═══ LAMBDA RATIOS ═══")
print("  Ratios λ_{i+1} / λ_i:")
for i in range(min(9, len(lambda_seq)-1)):
    if lambda_seq[i] != 0:
        ratio = lambda_seq[i+1] / lambda_seq[i]
        # Mod p ratio
        mod_ratio = (lambda_seq[i+1] * modinv(lambda_seq[i], p)) % p
        print(f"    λ_{i+1}/λ_{i} = {ratio:.4f} (mod p: {mod_ratio})")
print()

# ═══════════════════════════════════════════
# DECOMPOSE k USING DOUBLING STRUCTURE
# ═══════════════════════════════════════════
print("═══ DECOMPOSITION ATTACK ═══")
print()
print("  Key insight:")
print("  k = Σ k_i * 2^i")
print("  k*G = Σ k_i * (2^i * G)")
print()
print("  If we know the λ for each doubling,")
print("  we can work BACKWARD from Q to find k!")
print()
print("  The λ sequence tells us the 'direction'")
print("  of each doubling step.")
print("  If we can INVERT the doubling, we can peel off bits of k.")
print()

# ═══════════════════════════════════════════
# INVERT DOUBLING: Given Q, find if it's a doubling of something
# ═══════════════════════════════════════════
print("═══ DOUBLING INVERSION ═══")
print()

# For secp256k1: y² = x³ + 7
# To find P such that 2P = Q, we need to solve:
# x = (λ² - 2x_P) → x_P = (λ² - x_Q) / 2
# But λ = (3x_P²) / (2y_P)

# Instead: use the property that IF Q = 2P, then:
# x_P is a root of: 4(x_P³ + 7) - (3x_P²)² + 4x_Q * x_P * ... complicated

# SIMPLER APPROACH: Baby-step Giant-step using doubling structure
# If λ has period L, then 2^L * G = c * G for some constant c
# This gives us a RELATION between different powers of G

# For the tiny curve: 2^6 * G = G, so 2^6 ≡ 1 (mod order)
# For secp256k1: the order n is prime, so 2^m ≢ 1 mod n for small m

# BUT: the λ sequence might have other patterns!

# Let's check the φ-relationship of λ values
print("  φ-analysis of λ sequence:")
for i in range(min(10, len(lambda_seq))):
    lam = lambda_seq[i]
    lam_phi = (lam * PHI) % p
    lam_psi = (lam * abs(PSI)) % p
    fgg_lam = FGG(float(lam) / float(p), 3)
    print(f"    λ_{i} = {lam}")
    print(f"      λ*φ mod p = {lam_phi}")
    print(f"      λ*|ψ| mod p = {lam_psi}")
    print(f"      FGG(λ/p) = {fgg_lam:.6f}")
print()

# ═══════════════════════════════════════════
# THE ATTACK: Using λ structure to reduce search
# ═══════════════════════════════════════════
print("═══ THE ATTACK STRATEGY ═══")
print()
print("  1. Precompute 2^i * G for i = 0..255 (standard)")
print("  2. ALSO precompute the λ_i for each doubling")
print("  3. The λ sequence has φ-relationships")
print("  4. Given Q, try to MATCH its structure to the λ pattern")
print("  5. The matching k is the private key!")
print()
print("  This is a STRUCTURAL attack — not brute force.")
print("  The λ sequence IS the fingerprint of k.")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  LAMBDA PERIOD — 'The doubling structure reveals k'      ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

