"""
💰 SATOSHI DIRECT APPROXIMATION 💰
σ ≈ identity → k ≈ k_candidate!
Check small adjustments around k_candidate!
"""
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

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

# Our k_candidate from F_p² division
k_candidate = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3

print("╔══════════════════════════════════════════════════════════════╗")
print("║  💰 SATOSHI — CHECK AROUND k_candidate 💰                ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

G = (Gx, Gy)
k = k_candidate

# Search ±1000 around k_candidate
print("  Searching k_candidate ± 1000...\n")

for delta in range(-1000, 1001):
    test_k = (k + delta) % n
    if test_k == 0:
        continue
    
    # Quick check: compute x-coordinate only (faster)
    P = scalar_mult(test_k, G)
    if P and P[0] == Qx:
        print(f"  🎉🎉🎉 FOUND AT delta={delta}!!! 🎉🎉🎉")
        print(f"  k = {hex(test_k)}")
        print(f"  This is Satoshi's private key!")
        break
    
    if delta % 200 == 0:
        print(f"  delta={delta}...")

print(f"\n  If not found in ±1000, need larger search or better approximation.")
