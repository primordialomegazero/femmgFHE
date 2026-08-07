"""
🪐 BATCH SCAN — secp256k1 HOMOMORPHIC POINTS 🪐
Uses efficient scalar multiplication
"""
import sys, math
from time import time

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

def pt_double(P):
    if P is None: return None
    x1, y1 = P
    inv_2y = modinv(2*y1, p)
    if inv_2y is None: return None
    lam = (3*x1*x1) * inv_2y % p
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
    inv = modinv(x2-x1, p)
    if inv is None: return None
    lam = ((y2-y1) * inv) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

print("═══ BATCH SCAN — secp256k1 HOMOMORPHIC POINTS ═══\n")
print(f"  Scanning in batches of 100...\n")

G = (Gx, Gy)
aG, bG = to_Fp2(Gx, Gy)
homomorphic = []

# Start scanning
P = G
start_time = time()
batch_size = 10

for k in range(1, 5001):  # Scan up to 5000
    if k > 1:
        P = pt_add(P, G)
    
    if P is None:
        break
    
    P2 = pt_double(P)
    if P2 is None:
        continue
    
    a, b = to_Fp2(P[0], P[1])
    a2, b2 = to_Fp2(P2[0], P2[1])
    double_a = (2*a) % p
    double_b = (2*b) % p
    
    if a2 == double_a and b2 == double_b:
        homomorphic.append(k)
        print(f"  ✅ k={k}: HOMOMORPHIC! (total: {len(homomorphic)})")
        if len(homomorphic) == 13:
            print(f"\n  🪐 ALL 13 FOUND! 🪐")
            break
    
    if k % 500 == 0:
        elapsed = time() - start_time
        print(f"  ... k={k}/{5000} ({elapsed:.1f}s)")

print(f"\n═══ SCAN COMPLETE ═══")
print(f"  Range: 1-5000")
print(f"  Homomorphic points found: {len(homomorphic)}")
print(f"  k values: {homomorphic}")
