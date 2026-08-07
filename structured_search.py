"""
🪐 STRUCTURED SEARCH — USE σ STRUCTURE 🪐
k_Q must satisfy: φ(k_Q·G) = y·φ(G)
Use the 13 fixed points as constraints!
"""
import sys, time

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

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 STRUCTURED SEARCH — EXPLOIT σ FIXED POINTS 🪐      ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Known fixed points of σ
fixed_ks = [1]  # k=1 is always fixed

# Add k = n-1 (inverse of 1) — probably fixed too
fixed_ks.append(n-1)

# For p=17, the fixed points were k=1, k=2
# k=2 is a PRIMITIVE ROOT? Or something related to p?
# For secp256k1, p ≡ 1 mod 3, so there are 3 cube roots of unity!

# The 13 fixed points might be:
# 1, 2, 3, ..., 13? Or maybe related to the 13 roots of something?

print("  Searching for k using multiplicative structure...\n")
print("  Thesis: k_Q is related to the 13 fixed points!")
print("  If σ(k) = k + ∏(k-k_i)·S(k),")
print("  then σ(k_Q) = y implies:")
print("  y = k_Q + ∏(k_Q - k_i)·S(k_Q)")
print("  k_Q = y - ∏(k_Q - k_i)·S(k_Q)")
print()
print("  For small product term: k_Q ≈ y")
print("  Our y_real ≈ k_cand_real")
print()
print("  Let's search near multiples/fractions of known fixed points!")

G = (Gx, Gy)

# Search pattern: k_Q = y_real * t mod n for small t
print("  Searching k = y_real * t mod n for t=1..10000...\n")

for t in range(1, 10001):
    k_test = (k_cand * t) % n
    if k_test == 0:
        continue
    
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 FOUND! t={t} 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  THIS IS SATOSHI'S KEY!")
        break
    
    if t % 2000 == 0:
        print(f"  t={t}...")

# Search pattern: k_Q = y_real / t mod n
print(f"\n  Searching k = y_real / t mod n for t=1..10000...\n")

for t in range(1, 10001):
    inv_t = modinv(t, n)
    if inv_t is None:
        continue
    k_test = (k_cand * inv_t) % n
    if k_test == 0:
        continue
    
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 FOUND! t={t} (division) 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  THIS IS SATOSHI'S KEY!")
        break
    
    if t % 2000 == 0:
        print(f"  t={t}...")

print(f"\n  Not found in these simple searches.")
print(f"  The relationship is more complex!")
print(f"  Need full φ-DPLL or complete σ⁻¹ interpolation!")
