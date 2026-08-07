#!/usr/bin/env python3
"""
🪐 DOUBLE GOLDEN RATIO ATTACK 🪐
φ₁ = φ, φ₂ = φ²
Two projections = exact intersection = k!
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PHI2 = PHI * PHI  # φ² = 2.6180339887498948482
PSI = -0.6180339887498948482
PSI2 = PSI * PSI  # ψ²

def FGG(v, depth=3, use_phi2=False):
    """Fractal Golden Gate — with optional φ² mode"""
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
print("║  🪐 DOUBLE GOLDEN RATIO ATTACK — φ + φ² 🪐              ║")
print("║  Two projections = exact intersection = k                 ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

print(f"  φ = {PHI:.10f}")
print(f"  φ² = {PHI2:.10f}")
print(f"  φ + φ² = {PHI + PHI2:.10f}")
print(f"  φ × φ² = {PHI * PHI2:.10f} (= φ³)")
print(f"  φ² / φ = {PHI2/PHI:.10f} (= φ)")
print(f"  φ² - φ = {PHI2 - PHI:.10f} (= 1!)")
print(f"  ψ = {PSI:.10f}")
print(f"  ψ² = {PSI2:.10f}")
print(f"  φ² × ψ² = {PHI2 * PSI2:.10f} (= 1!)")
print()

# ═══════════════════════════════════════════
# DOUBLE PROJECTION
# ═══════════════════════════════════════════

def project_phi(point):
    """Project point using φ"""
    if point is None: return 0.0
    x, y = point
    return (float(x) * PHI + float(y) * PSI) % float(p)

def project_phi2(point):
    """Project point using φ²"""
    if point is None: return 0.0
    x, y = point
    return (float(x) * PHI2 + float(y) * PSI2) % float(p)

# ═══════════════════════════════════════════
# DOUBLE TRIANGULATION
# ═══════════════════════════════════════════

print("═══ DOUBLE TRIANGULATION TEST ═══")
print()

for bits in [4, 6, 8]:
    print(f"  {bits}-bit test:")
    
    for _ in range(3):
        secret = random.randint(1, (1 << bits) - 1)
        Q_test = scalar_mult(secret, G)
        
        # Get BOTH projections
        proj1_Q = project_phi(Q_test)
        proj2_Q = project_phi2(Q_test)
        proj1_G = project_phi(G)
        proj2_G = project_phi2(G)
        
        # The double ratio:
        # R1 = proj1_Q / proj1_G
        # R2 = proj2_Q / proj2_G
        # The intersection of R1 and R2 gives k!
        
        # For each possible k, compute expected ratios
        best_k = 0
        best_error = float('inf')
        
        for k_test in range(1, min(10000, (1 << bits) * 2)):
            # Expected projections for k*G
            # The projections should be MULTIPLICATIVE if linear
            # R1(k) = proj1(k*G) / proj1(G) ≈ some function of k
            # R2(k) = proj2(k*G) / proj2(G) ≈ some function of k
            
            # Compute actual projections for k_test
            P_test = scalar_mult(k_test, G)
            proj1_test = project_phi(P_test)
            proj2_test = project_phi2(P_test)
            
            # Error: how far from Q's projections?
            err1 = abs(proj1_test - proj1_Q) / float(p)
            err2 = abs(proj2_test - proj2_Q) / float(p)
            
            # Combined error (Euclidean in projection space)
            error = math.sqrt(err1**2 + err2**2)
            
            if error < best_error:
                best_error = error
                best_k = k_test
        
        match = best_k == secret
        print(f"    secret={secret:4d}, found={best_k:4d}, error={best_error:.6f} {'✅' if match else '❌'}")

print()

# ═══════════════════════════════════════════
# THE KEY INSIGHT
# ═══════════════════════════════════════════
print("═══ DOUBLE RATIO ANALYSIS ═══")
print()

# Compute the "double signature" for different k
print("  k → (proj1, proj2) for first 20 k:")
for k in range(1, 21):
    P = scalar_mult(k, G)
    p1 = project_phi(P) / float(p)
    p2 = project_phi2(P) / float(p)
    ratio = p2 / p1 if p1 != 0 else 0
    print(f"    k={k:2d}: φ={p1:.6f}, φ²={p2:.6f}, φ²/φ={ratio:.6f}")

print()
print("  The double ratio φ²/φ should be CONSTANT if linear.")
print("  If it varies with k, that variation IS the fingerprint!")

# Check if φ²/φ ratio is unique for each k
print()
print("  Checking uniqueness of double ratio...")
ratios = {}
for k in range(1, 101):
    P = scalar_mult(k, G)
    p1 = project_phi(P) / float(p)
    p2 = project_phi2(P) / float(p)
    r = round(p2 / p1, 10) if p1 != 0 else 0
    if r in ratios:
        ratios[r].append(k)
    else:
        ratios[r] = [k]

unique = sum(1 for v in ratios.values() if len(v) == 1)
print(f"  Unique ratios: {unique}/100")
print(f"  Collisions exist: {'Yes' if unique < 100 else 'No — PERFECT FINGERPRINT!'}")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DOUBLE φ — 'Two golden ratios triangulate k'            ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

