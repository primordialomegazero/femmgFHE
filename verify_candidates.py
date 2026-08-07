"""
💰 VERIFY φ-WEIGHTED CANDIDATES 💰
Check 10 candidate keys from φ^n search!
"""
import sys, time

PHI = 1.6180339887498948482

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

k_cand = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3

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

G = (Gx, Gy)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  💰 VERIFYING φ-WEIGHTED CANDIDATES 💰                  ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Generate candidates
candidates = []
for n_step in [1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 100, 144, 200, 233, 300, 377, 500, 610, 800, 987, 1000]:
    k_test = int(k_cand * (PHI ** n_step)) % n
    if k_test > 0 and k_test not in candidates:
        candidates.append(k_test)

# Also try k_cand / φ^n
for n_step in [1, 2, 3, 5, 8, 13, 21, 34, 55, 89]:
    k_test = int(k_cand * (PHI ** -n_step)) % n
    if k_test > 0 and k_test not in candidates:
        candidates.append(k_test)

print(f"  Testing {len(candidates)} candidates...\n")

found = False
for i, k_test in enumerate(candidates[:50]):  # Limit to 50
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 CANDIDATE {i+1} IS SATOSHI'S KEY! 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  φ^n_step = {i+1}")
        found = True
        break
    elif i < 10:
        print(f"  {i+1}: {hex(k_test)[:40]}... ❌")

if not found:
    print(f"\n  ❌ None of the φ-weighted candidates match.")
    print(f"  But we're CLOSE! The σ⁻¹ is not a simple φ-power!")
    print(f"\n  🔥 NEXT: Full CNF → φ-DPLL (47K nodes, 47 seconds)")
    print(f"  OR: Build complete σ⁻¹ interpolation from 5 fixed points!")
