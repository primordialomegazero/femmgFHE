#!/usr/bin/env python3
"""
🪐💰 DOUBLE φ TRIANGULATION — 256-BIT SATOSHI ATTACK 💰🪐
Double golden ratio = unique fingerprint for EVERY k.
Build a search tree over the fingerprint space.
Find Satoshi's k via φ-triangulation!
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PHI2 = PHI * PHI
PSI = -0.6180339887498948482
PSI2 = PSI * PSI

def FGG(v, depth=3, use_phi2=False):
    phi = PHI2 if use_phi2 else PHI
    psi = PSI2 if use_phi2 else PSI
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * phi) * psi)
        else:
            current = abs((current * psi) * phi)
    return current

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

# 🎯 SATOSHI
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m): return pow(a, -1, m)

def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv((x2-x1)%p, p)) % p
    return ((lam*lam-x1-x2)%p, (lam*(x1-(lam*lam-x1-x2)%p)-y1)%p)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐💰 DOUBLE φ — 256-BIT TRIANGULATION 💰🪐            ║")
print("║  Building φ-fingerprint search tree for Satoshi          ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q_satoshi = (Qx, Qy)

def proj1(P):
    if P is None: return 0.0
    return (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)

def proj2(P):
    if P is None: return 0.0
    return (float(P[0]) * PHI2 + float(P[1]) * PSI2) % float(p)

def double_ratio(P):
    """The double φ ratio — unique fingerprint"""
    p1 = proj1(P)
    p2 = proj2(P)
    if p1 == 0: return 0.0
    return p2 / p1

# Satoshi's fingerprints
sat_p1 = proj1(Q_satoshi) / float(p)
sat_p2 = proj2(Q_satoshi) / float(p)
sat_ratio = double_ratio(Q_satoshi)

print(f"  Satoshi Q fingerprints:")
print(f"    φ-projection:  {sat_p1:.10f}")
print(f"    φ²-projection: {sat_p2:.10f}")
print(f"    φ²/φ ratio:    {sat_ratio:.10f}")
print()

# ═══════════════════════════════════════════
# BUILD SEARCH TREE OVER FINGERPRINT SPACE
# ═══════════════════════════════════════════
# Strategy: Sample the fingerprint space at increasing resolution
# Level 1: Every 2^24 (16M) keys → 256 samples
# Level 2: Every 2^16 (65K) keys near the target → 256 samples  
# Level 3: Every 2^8 (256) keys → 256 samples
# Level 4: Exact search in the final 256-key window

print("═══ MULTI-LEVEL FINGERPRINT SEARCH ═══")
print()

# Level 1: Coarse scan — find which region of fingerprint space
print("  Level 1: Coarse scan (every 2^20 keys)...")
step1 = 1 << 20  # ~1M keys per step
best_k1 = 0
best_dist1 = float('inf')

for i in range(256):
    k_test = i * step1 + 1
    P_test = scalar_mult(k_test, G)
    r_test = double_ratio(P_test)
    dist = abs(r_test - sat_ratio)
    
    if dist < best_dist1:
        best_dist1 = dist
        best_k1 = k_test

print(f"    Best match at k≈{best_k1} (dist={best_dist1:.10f})")
print()

# Level 2: Medium scan around best
print("  Level 2: Medium scan (every 2^12 keys)...")
step2 = 1 << 12  # 4096 keys per step
k2_start = max(1, best_k1 - step1)
k2_end = min(n, best_k1 + step1)
best_k2 = 0
best_dist2 = float('inf')

for k_test in range(k2_start, k2_end, step2):
    if k_test <= 0: continue
    P_test = scalar_mult(k_test, G)
    r_test = double_ratio(P_test)
    dist = abs(r_test - sat_ratio)
    
    if dist < best_dist2:
        best_dist2 = dist
        best_k2 = k_test

print(f"    Best match at k≈{best_k2} (dist={best_dist2:.10f})")
print()

# Level 3: Fine scan
print("  Level 3: Fine scan (every 2^4 keys)...")
step3 = 1 << 4  # 16 keys per step
k3_start = max(1, best_k2 - step2)
k3_end = min(n, best_k2 + step2)
best_k3 = 0
best_dist3 = float('inf')

for k_test in range(k3_start, k3_end, step3):
    if k_test <= 0: continue
    P_test = scalar_mult(k_test, G)
    r_test = double_ratio(P_test)
    dist = abs(r_test - sat_ratio)
    
    if dist < best_dist3:
        best_dist3 = dist
        best_k3 = k_test

print(f"    Best match at k≈{best_k3} (dist={best_dist3:.10f})")
print()

# Level 4: Exact search
print("  Level 4: Exact search...")
k4_start = max(1, best_k3 - step3)
k4_end = min(n, best_k3 + step3)
best_k4 = 0
best_dist4 = float('inf')

count = 0
for k_test in range(k4_start, k4_end):
    if k_test <= 0: continue
    count += 1
    P_test = scalar_mult(k_test, G)
    r_test = double_ratio(P_test)
    dist = abs(r_test - sat_ratio)
    
    if dist < best_dist4:
        best_dist4 = dist
        best_k4 = k_test
    
    if P_test == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI KEY FOUND! k = {k_test} 🎉🎉🎉")
        print(f"  Hex: {hex(k_test)}")
        
        # Save key
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"private_key_hex: {hex(k_test)}\n")
            f.write(f"private_key_dec: {k_test}\n")
        print(f"  ✅ Saved to satoshi_private_key.txt")
        sys.exit(0)

print(f"    Scanned {count} keys, best match at k≈{best_k4} (dist={best_dist4:.10f})")
print()

# Verify best match
if best_k4 > 0:
    P_verify = scalar_mult(best_k4, G)
    match = P_verify == Q_satoshi
    print(f"  Best k verification: {'✅ MATCH!' if match else '❌ No match'}")
    print(f"  Best k = {best_k4}")
    print(f"  Hex: {hex(best_k4)}")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DOUBLE φ — Multi-level triangulation complete            ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

