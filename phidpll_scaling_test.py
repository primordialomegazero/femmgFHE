#!/usr/bin/env python3
"""
🪐 φ-DPLL SCALING TEST — DOES IT REALLY SCALE? 🪐
Test φ-DPLL on increasing key sizes: 16, 20, 24, 28, 32, 36, 40 bits.
Measure actual nodes visited.
If S(n) ≈ log_φ(n) ≈ 0.694*log2(n), then:
  - 32-bit: S ≈ 22 nodes
  - 40-bit: S ≈ 28 nodes  
  - 128-bit: S ≈ 89 nodes (NOT 16!)
  - 256-bit: S ≈ 178 nodes

This tests whether the "S(256)=24" claim holds up.
"""
import time, math, random, sys

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

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m): return pow(a, -1, m)

def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3 * x1 * x1) * modinv(2 * y1, p) % p
    else:
        lam = ((y2 - y1) * modinv((x2 - x1) % p, p)) % p
    return ((lam * lam - x1 - x2) % p, (lam * (x1 - (lam * lam - x1 - x2) % p) - y1) % p)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def fp(P):
    if P is None: return 0.0
    raw = (P[0] * PHI + P[1] * PSI) % p
    return FGG(raw / p, 3)

G = (Gx, Gy)

def phidpll_search(target_fp, max_k, max_nodes=100000):
    """
    Simplified φ-DPLL: scan the space looking for the fingerprint match.
    Returns (found_k, nodes_visited).
    """
    nodes = 0
    current = None  # 0*G
    step = 1
    
    for k in range(max_k + 1):
        nodes += 1
        current_fp = fp(current)
        
        if abs(current_fp - target_fp) < 1e-12:
            # Verify exact match
            if current_fp == target_fp:  # float comparison for now
                return k, nodes
        
        current = pt_add(current, G)
        
        if nodes >= max_nodes:
            break
    
    return None, nodes


print("╔══════════════════════════════════════════════════════════════╗")
print("║  φ-DPLL SCALING TEST — Brute force as baseline            ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Test increasing bit sizes
bit_sizes = [8, 12, 16, 20, 24]
results = []

for bits in bit_sizes:
    max_k = (1 << bits) - 1
    secret = random.randint(1, max_k)
    Q = scalar_mult(secret, G)
    target_fp = fp(Q)
    
    t0 = time.time()
    found, nodes = phidpll_search(target_fp, max_k)
    elapsed = time.time() - t0
    
    match = "✅" if found == secret else "❌"
    log2_nodes = math.log2(nodes) if nodes > 0 else 0
    predicted_phi = 0.694 * bits  # log_φ(2^bits) = bits * ln(2)/ln(φ)
    
    results.append((bits, nodes, elapsed, match))
    
    print(f"  {bits}-bit: found={found}, nodes={nodes}, "
          f"log2(nodes)={log2_nodes:.1f}, "
          f"predicted φ-DPLL={predicted_phi:.1f}, "
          f"time={elapsed:.4f}s {match}")

print(f"\n═══ SCALING PROJECTION ═══\n")

# Fit: nodes = 2^bits (brute force) or nodes ≈ bits/φ (φ-DPLL claim)
print(f"  If brute force: nodes = 2^bits")
for bits in [32, 64, 128, 256]:
    print(f"    {bits}-bit: 2^{bits} = 10^{int(bits*0.3010)} nodes")

print(f"\n  If φ-DPLL claim (S=24 for 256-bit): nodes ≈ log_φ(2^bits)")
for bits in [32, 64, 128, 256]:
    pred = 0.694 * bits
    print(f"    {bits}-bit: ~{pred:.0f} nodes (claim: {'✅' if pred <= 30 else '❌'})")

print(f"\n  REALITY CHECK:")
print(f"  For the φ-DPLL 'S(256)=24' claim to hold,")
print(f"  the 24-bit test above should show ~17 nodes, not {results[-1][1] if results else '?'}.")
print(f"  If 24-bit takes 2^24 nodes, then 256-bit takes 2^256 nodes.")
print(f"  The fingerprint is a hash — it doesn't create shortcuts.")
print(f"  Sub-linear search requires an ORACLE, not a HASH.")
