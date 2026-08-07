#!/usr/bin/env python3
"""
🪐 DUAL REPRESENTATION — φ-WORLD vs ψ-WORLD 🪐
G encoded in φ-world, Q encoded in ψ-world.
Ratio = k! Because φ·ψ = -1, the asymmetry reveals k.
"1+1=2" — two golden ratio representations.
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

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
print("║  🪐 DUAL REPRESENTATION — φ vs ψ WORLDS 🪐              ║")
print("║  G → φ-world, Q → ψ-world → ratio = k!                  ║")
print("║  '1+1=2' — two golden ratio representations             ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q_satoshi = (Qx, Qy)

print(f"  Golden Ratio Properties:")
print(f"    φ = {PHI:.10f}")
print(f"    ψ = {PSI:.10f}")
print(f"    φ + ψ = {PHI + PSI:.10f}  (should be 1)")
print(f"    φ · ψ = {PHI * PSI:.10f}  (should be -1)")
print(f"    φ / ψ = {PHI / PSI:.10f}")
print(f"    ψ / φ = {PSI / PHI:.10f}")
print()

# ═══════════════════════════════════════════
# DUAL ENCODING
# ═══════════════════════════════════════════

def encode_phi(P):
    """Encode point in φ-world (expansion)"""
    if P is None: return (0.0, 0.0)
    x, y = P
    # φ-world: multiply coordinates by φ
    return (x * PHI, y * PHI)

def encode_psi(P):
    """Encode point in ψ-world (contraction)"""
    if P is None: return (0.0, 0.0)
    x, y = P
    # ψ-world: multiply coordinates by |ψ| (absolute value of ψ)
    psi_abs = abs(PSI)
    return (x * psi_abs, y * psi_abs)

def mixed_encoding(P, use_phi_for_x=True):
    """
    MIXED encoding: x in one world, y in the other.
    This creates asymmetry that φ·ψ = -1 can exploit!
    """
    if P is None: return (0.0, 0.0)
    x, y = P
    if use_phi_for_x:
        return (x * PHI, y * abs(PSI))
    else:
        return (x * abs(PSI), y * PHI)

def dual_signature(P):
    """
    DUAL SIGNATURE:
    Combine φ-encoded x with ψ-encoded y.
    This is NOT cancellable because φ ≠ ψ!
    """
    if P is None: return 0.0
    x, y = P
    # φ for x, ψ for y (asymmetric!)
    return (x * PHI + y * PSI) % float(p)

print("═══ DUAL SIGNATURE TEST ═══")
print()
print("  Dual signature = x*φ + y*ψ (NOT x*φ + y*φ!)")
print("  This is ASYMMETRIC — φ and ψ don't cancel!")
print()

# Test dual signatures
sig_G = dual_signature(G) / float(p)
sig_Q = dual_signature(Q_satoshi) / float(p)

print(f"  dual_sig(G)/p = {sig_G:.10f}")
print(f"  dual_sig(Q)/p = {sig_Q:.10f}")
print(f"  Ratio = {sig_Q / sig_G:.10f}")
print()

# ═══════════════════════════════════════════
# THE KEY TEST: Does dual ratio give k?
# ═══════════════════════════════════════════
print("═══ DOES DUAL RATIO REVEAL k? ═══")
print()
print(f"  {'k':<10} {'dual(Q)/dual(G)':<30} {'k*φ mod 1':<20} {'Match?'}")
print(f"  {'-'*70}")

for k_test in [1, 2, 3, 5, 7, 10, 42, 100, 255, 1000]:
    Q_test = scalar_mult(k_test, G)
    
    sig_G_test = dual_signature(G) / float(p)
    sig_Q_test = dual_signature(Q_test) / float(p)
    
    dual_ratio = sig_Q_test / sig_G_test if sig_G_test != 0 else 0
    
    # Map ratio to a scalar
    k_phi = (k_test * PHI) % 1.0
    
    # Check: is the ratio related to k?
    ratio_mod1 = dual_ratio % 1.0
    
    print(f"  {k_test:<10} {dual_ratio:<30.10f} {k_phi:<20.10f} ", end="")
    
    if abs(ratio_mod1 - k_phi) < 0.01:
        print("✅ CLOSE!")
    elif abs(dual_ratio - k_test) < 0.5:
        print(f"≈{int(round(dual_ratio))} (off by {int(round(dual_ratio))-k_test})")
    else:
        print("❌")

print()

# ═══════════════════════════════════════════
# THE REAL DUAL ATTACK: φ on G, ψ on Q
# ═══════════════════════════════════════════
print("═══ THE REAL DUAL ATTACK ═══")
print()
print("  Theory:")
print("    1. Encode G in φ-world: G_φ = (Gx*φ, Gy*φ)")
print("    2. Encode Q in ψ-world: Q_ψ = (Qx*ψ, Qy*ψ)")
print("    3. The RATIO between worlds reveals k!")
print()
print("  Why? Because:")
print("    G_φ = G * φ  (scalar multiplication in some sense)")
print("    Q_ψ = (k*G) * ψ = k * (G * ψ)")
print("    Q_ψ / G_φ = k * (ψ/φ)")
print("    k = (Q_ψ / G_φ) * (φ/ψ)")
print()
print("  φ/ψ = -φ² ≈ -2.618 — a KNOWN constant!")
print("  So k = (Q_ψ / G_φ) * (-φ²)")
print()

# Compute using dual worlds
def encode_phi_point(P):
    """Encode point coordinates in φ-world as field element"""
    if P is None: return 0
    return (P[0] * PHI + P[1] * PHI) % float(p)

def encode_psi_point(P):
    """Encode point coordinates in ψ-world as field element"""
    if P is None: return 0
    return (P[0] * abs(PSI) + P[1] * abs(PSI)) % float(p)

G_phi = encode_phi_point(G)
Q_psi = encode_psi_point(Q_satoshi)

print(f"  G_φ = {G_phi:.4f}")
print(f"  Q_ψ = {Q_psi:.4f}")
print(f"  Q_ψ / G_φ = {Q_psi / G_phi:.10f}")
print(f"  φ/ψ = {PHI / abs(PSI):.10f}")
print(f"  k_est = (Q_ψ / G_φ) * (φ/ψ) = {(Q_psi / G_phi) * (PHI / abs(PSI)):.4f}")
print()

# Test on known keys
print("  Testing dual world ratio on known keys...")
print(f"  {'k':<10} {'Q_ψ/G_φ':<20} {'k_est':<20} {'Match?'}")
print(f"  {'-'*55}")

for k_test in [1, 2, 3, 5, 7, 10, 42, 100, 255]:
    Q_test = scalar_mult(k_test, G)
    q_psi = encode_psi_point(Q_test)
    ratio = q_psi / G_phi if G_phi != 0 else 0
    k_est = ratio * (PHI / abs(PSI))
    match = abs(k_est - k_test) < 0.5
    print(f"  {k_test:<10} {ratio:<20.10f} {k_est:<20.4f} {'✅' if match else '❌'}")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DUAL REPRESENTATION — '1+1=2 opens the φ-ψ bridge'     ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

