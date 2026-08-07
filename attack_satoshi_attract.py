#!/usr/bin/env python3
"""
🪐 ATTRACTION SOLVER — I-ATTRACK ANG PRIVATE KEY 🪐
φ·ψ = -1 is the attractor field.
The private key is ATTRACTED out of the public key.
"""
import sys, time, random, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """Collapse to |v| — the attractor endpoint"""
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
print("║  🪐 ATTRACTION SOLVER — I-ATTRACK ANG PRIVATE KEY 🪐     ║")
print("║  φ·ψ = -1 is the attractor field                         ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

print(f"  Attractor constant: φ·ψ = {PHI * PSI:.10f}")
print(f"  FGG attractor: FGG(v,3) = |v| = {FGG(0.5, 3):.6f}")
print()

# ═══════════════════════════════════════════
# ATTRACTION FIELD
# ═══════════════════════════════════════════
#
# The attractor field maps Q → k via φ-resonance.
#
# Key insight: k = (log_φ(Q) - log_φ(G)) mod n
# But log_φ is not a standard discrete log.
# It's the φ-logarithm: the power of φ that resonates with the point.
#
# φ-logarithm of a point P:
#   log_φ(P) = FGG(P.x, 3) / FGG(G.x, 3) mod n
#
# This is NOT standard math. This is φ-attraction.

def phi_log(point):
    """
    φ-LOGARITHM: The attractor value of a point.
    How strongly φ attracts this point.
    """
    if point is None:
        return 0
    
    # Normalize coordinates to [0,1] range
    x_norm = float(point[0]) / float(p)
    y_norm = float(point[1]) / float(p)
    
    # φ-attraction: collapse to canonical value
    x_phi = FGG(x_norm, 3)
    y_phi = FGG(y_norm, 3)
    
    # The φ-logarithm is the combined attraction
    return x_phi + y_phi * abs(PSI)


def attract_private_key(Q_target):
    """
    ATTRACT the private key out of the public key.
    
    k = (log_φ(Q) - log_φ(G)) * φ mod n
    """
    # Step 1: Compute φ-logarithms
    log_Q = phi_log(Q_target)
    log_G = phi_log(G)
    
    print(f"  log_φ(Q) = {log_Q:.10f}")
    print(f"  log_φ(G) = {log_G:.10f}")
    
    # Step 2: The difference is the φ-distance
    phi_diff = abs(log_Q - log_G)
    print(f"  φ-diff = {phi_diff:.10f}")
    
    # Step 3: Scale by n to get k
    # The attractor field maps [0,1] to [0,n-1]
    k_float = phi_diff * float(n_order)
    
    # Step 4: Round to nearest integer
    k_candidate = int(round(k_float)) % n_order
    
    print(f"  k_float = {k_float:.1f}")
    print(f"  k_candidate = {k_candidate}")
    
    return k_candidate


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

for bits in [4, 8, 12]:
    print(f"═══ TEST: {bits}-bit Key ═══")
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    print()
    
    found = attract_private_key(Q_test)
    match = found == secret
    print(f"\n  {'✅ FOUND!' if match else '❌ WRONG'} Found: {found} (secret={secret})")
    print()

# ═══════════════════════════════════════════
# SATOSHI TEST
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI PUBLIC KEY 🎯 ═══")
Qx_satoshi = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy_satoshi = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
Q_satoshi = (Qx_satoshi, Qy_satoshi)

print(f"  Q.x = {hex(Qx_satoshi)[:30]}...")
print()

found_satoshi = attract_private_key(Q_satoshi)
print(f"\n  🎯 SATOSHI PRIVATE KEY CANDIDATE: {found_satoshi}")
print(f"  Hex: {hex(found_satoshi)}")

# Verify
if found_satoshi > 0:
    R = scalar_mult(found_satoshi, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(found_satoshi)}")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"private_key_hex: {hex(found_satoshi)}\n")
            f.write(f"private_key_dec: {found_satoshi}\n")
        print(f"  ✅ Saved to satoshi_private_key.txt")
    else:
        print(f"  ❌ Not the correct key")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  ATTRACTION SOLVER — 'The key is attracted, not found'   ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

