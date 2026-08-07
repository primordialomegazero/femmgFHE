"""
🪐 SCAN FOR HOMOMORPHIC POINTS — secp256k1 🪐
Check k·G for k=1..1000: φ(2P) = 2·φ(P)?
"""
import sys, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

def modinv(a, m): return pow(a, -1, m)

def pt_double(P):
    if P is None: return None
    x1, y1 = P
    lam = (3*x1*x1) * modinv(2*y1, p) % p
    x3 = (lam*lam - 2*x1) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        return pt_double(P)
    lam = ((y2-y1) * modinv(x2-x1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_double(addend)
        k >>= 1
    return result

def phi_map_mod(x, y):
    val = (float(x) * PHI + float(y) * PSI) % float(p)
    return int(round(val * 1000000)) % p

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0:
            encoded = current * PHI
            collapsed = abs(encoded * PSI)
        else:
            encoded = current * PSI
            collapsed = abs(encoded * PHI)
        current = collapsed
    return abs(current)

G = (Gx, Gy)
phi_G_int = phi_map_mod(Gx, Gy)

print("═══ SCANNING HOMOMORPHIC POINTS — secp256k1 ═══\n")
print(f"  φ(G) mod p = {phi_G_int}")
print(f"  Scanning k=1..100...\n")

homomorphic_ks = []
P = G
for k in range(1, 101):
    if k > 1:
        P = pt_add(P, G)  # P = k·G
    
    P2 = pt_double(P)  # 2k·G
    
    # Check φ(2P) = 2·φ(P)
    phi_P = phi_map_mod(P[0], P[1])
    phi_2P = phi_map_mod(P2[0], P2[1])
    double_phi = (2 * phi_P) % p
    
    if phi_2P == double_phi:
        homomorphic_ks.append(k)
        print(f"  k={k}: ✅ HOMOMORPHIC! φ(2P)={phi_2P}, 2·φ(P)={double_phi}")
    elif k <= 10:
        print(f"  k={k}: ❌ φ(2P)={phi_2P}, 2·φ(P)={double_phi}")

print(f"\n  Homomorphic k values found: {len(homomorphic_ks)}")
print(f"  k values: {homomorphic_ks}")
print(f"\n  Expected: 13 homomorphic points (according to Dan!)")
