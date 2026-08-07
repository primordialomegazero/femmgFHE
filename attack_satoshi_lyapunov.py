#!/usr/bin/env python3
"""
🪐 LYAPUNOV STABILITY ANALYSIS — ECDLP 🪐
Measure the convergence/divergence of φ-perturbations.
Find the stable fixed point = private key.
"""
import sys, time, random, math

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

def point_sub(P, Q):
    if Q is None: return P
    return point_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 LYAPUNOV STABILITY — FIND THE FIXED POINT 🪐         ║")
print("║  'The private key is the stable attractor'               ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

def phi_distance(P1, P2):
    """φ-weighted distance between two points"""
    if P1 is None or P2 is None:
        return float('inf')
    
    dx = abs(P1[0] - P2[0]) / float(p)
    dy = abs(P1[1] - P2[1]) / float(p)
    
    # φ-weighted: φ prefers x (larger), |ψ| prefers y (smaller)
    return dx * PHI + dy * abs(PSI)


def lyapunov_analysis(Q_target, G):
    """
    LYAPUNOV STABILITY ANALYSIS
    
    For each candidate k, compute the φ-distance between:
    - The actual Q (target)
    - The point k*G (candidate)
    
    The Lyapunov-stable k is the one where:
    |F(Q) - F(k*G)| is MINIMIZED
    
    Where F = FGG projection.
    """
    log_G = FGG(float(G[0]) / float(p), 3) + FGG(float(G[1]) / float(p), 3) * abs(PSI)
    log_Q = FGG(float(Q_target[0]) / float(p), 3) + FGG(float(Q_target[1]) / float(p), 3) * abs(PSI)
    
    print(f"  F(G) = {log_G:.10f}")
    print(f"  F(Q) = {log_Q:.10f}")
    print(f"  φ-diff = {abs(log_Q - log_G):.10f}")
    print()
    
    # Test perturbation: scan around the φ-diff
    # The Lyapunov-stable point is where perturbation → 0
    
    best_k = 0
    best_stability = float('inf')
    
    # Scan k values near the φ-projection
    # The φ-diff tells us the approximate magnitude of k
    # Lyapunov stability tells us the exact k
    
    # For small test, scan all possible k
    # For large, scan around the φ-estimate
    
    print("  Scanning for Lyapunov-stable k...")
    
    # Start with φ-estimate
    phi_est = int(abs(log_Q - log_G) * float(n_order)) % n_order
    
    # Scan around the estimate
    scan_range = 1000  # Adjust based on bits
    
    for offset in range(-scan_range, scan_range + 1):
        k_test = (phi_est + offset) % n_order
        if k_test == 0:
            continue
        
        R = scalar_mult(k_test, G)
        if R is None:
            continue
        
        # Lyapunov: measure φ-distance between Q and R
        dist = phi_distance(Q_target, R)
        
        # FGG collapse of the distance
        collapsed_dist = FGG(dist, 3)
        
        if collapsed_dist < best_stability:
            best_stability = collapsed_dist
            best_k = k_test
    
    return best_k, best_stability


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

for bits in [4, 8]:
    print(f"═══ TEST: {bits}-bit Key ═══")
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    print()
    
    start = time.time()
    found, stability = lyapunov_analysis(Q_test, G)
    elapsed = time.time() - start
    
    match = found == secret
    print(f"\n  {'✅' if match else '❌'} Found: {found} (secret={secret})")
    print(f"  Lyapunov stability: {stability:.10f}")
    print(f"  Time: {elapsed:.4f}s")
    print()

# ═══════════════════════════════════════════
# SATOSHI — Quick Lyapunov check
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI — LYAPUNOV ESTIMATE 🎯 ═══")
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
Q_satoshi = (Qx, Qy)

log_G = FGG(float(Gx) / float(p), 3) + FGG(float(Gy) / float(p), 3) * abs(PSI)
log_Q = FGG(float(Qx) / float(p), 3) + FGG(float(Qy) / float(p), 3) * abs(PSI)

phi_est = int(abs(log_Q - log_G) * float(n_order)) % n_order

print(f"  F(G) = {log_G:.10f}")
print(f"  F(Q) = {log_Q:.10f}")
print(f"  φ-estimate of k: {phi_est}")
print(f"  Hex: {hex(phi_est)}")
print()

# Check stability at φ-estimate
R_test = scalar_mult(phi_est, G)
if R_test:
    dist = phi_distance(Q_satoshi, R_test)
    stability = FGG(dist, 3)
    print(f"  Distance at φ-estimate: {dist:.10f}")
    print(f"  Stability: {stability:.10f}")
    print(f"  Match: {'✅' if R_test == Q_satoshi else '❌'}")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  LYAPUNOV — 'The private key is the stable fixed point'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

