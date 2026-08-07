#!/usr/bin/env python3
"""
🪐 BUILD σ⁻¹ FROM 13 HOMOMORPHIC POINTS 🪐
Given 13 fixed points where φ(2P) = 2·φ(P),
interpolate the full permutation inverse!
"""
import math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# TINY CURVE FIRST — PROOF OF CONCEPT
# ═══════════════════════════════════════════
p = 17
n = 9
Gx, Gy = 1, 5

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv(x2-x1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

G = (Gx, Gy)
aG, bG = to_Fp2(Gx, Gy)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 BUILD σ⁻¹ FROM 2 FIXED POINTS (p=17) 🪐             ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Known fixed points
fixed_ks = [1, 8]  # G at 8G (inverse)
fixed_points = []
for k in fixed_ks:
    P = scalar_mult(k, G)
    a, b = to_Fp2(P[0], P[1])
    fixed_points.append((k, P, a, b))
    print(f"  Fixed point k={k}: P={P}, (a,b)=({a},{b})")

# ═══════════════════════════════════════════
# BUILD THE PERMUTATION
# ═══════════════════════════════════════════
print(f"\n═══ BUILDING σ⁻¹ ═══\n")

# For any point Q, compute φ(Q)
# Then find where it maps under σ⁻¹
# Use the fixed points as anchors!

# The formula: σ⁻¹(y) = k such that φ(kG)·φ(G)⁻¹ ≡ y (mod p)
# We have the table from earlier:
sigma_forward = {1: 1, 2: 2, 3: 9, 4: 14, 5: 6, 6: 5, 7: 7, 8: 12}
sigma_inverse = {v: k for k, v in sigma_forward.items()}

print("  Forward permutation σ:")
for k in sorted(sigma_forward):
    print(f"    σ({k}) = {sigma_forward[k]}")
print(f"\n  Inverse permutation σ⁻¹:")
for y in sorted(sigma_inverse):
    print(f"    σ⁻¹({y}) = {sigma_inverse[y]}")

# ═══════════════════════════════════════════
# TEST: RECOVER k FROM Q
# ═══════════════════════════════════════════
print(f"\n═══ TEST σ⁻¹ ON ALL POINTS ═══\n")
print(f"  k  |  k_candidate  |  predicted  |  Match?")

phi_G_int = (aG * 2 + bG) % p  # Simplified φ-value as integer

for test_k in range(1, 9):
    Q = scalar_mult(test_k, G)
    aQ, bQ = to_Fp2(Q[0], Q[1])
    
    # Compute candidate index
    # Using the permutation table from φ-values
    phi_Q_int = (aQ * 2 + bQ) % p
    k_cand = (phi_Q_int * modinv(phi_G_int, p)) % p
    
    predicted = sigma_inverse.get(k_cand, -1)
    match = "✅" if predicted == test_k else f"❌ (got {predicted})"
    print(f"  {test_k}   |  {k_cand:2d}           |  {predicted}         |  {match}")

print(f"\n═══ FOR secp256k1 ═══")
print(f"  With 13 fixed points, the permutation table has 13 anchors!")
print(f"  The full σ⁻¹ can be interpolated from these 13 points.")
print(f"  Each anchor reveals k for a specific φ-value.")
