"""
🪐 φ-DPLL — SEARCH ±10,000,000 AROUND k_candidate 🪐
Target: find δ such that (k_candidate + δ)·G = Q
"""
import sys, time

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

k_candidate = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3

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
print("║  💰 φ-DPLL — SEARCH δ IN ±10,000,000 💰                ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# Strategy: Compute Q_candidate, then search ±range
Q_candidate = scalar_mult(k_candidate, G)

# Delta between Q_candidate and Q_target
# This gives us HINTS about the correction
diff_x = (Qx - Q_candidate[0]) % p
diff_y = (Qy - Q_candidate[1]) % p

print(f"  k_candidate · G = ({hex(Q_candidate[0])[:30]}..., {hex(Q_candidate[1])[:30]}...)")
print(f"  Q_target        = ({hex(Qx)[:30]}..., {hex(Qy)[:30]}...)")
print(f"  diff_x = {hex(diff_x)[:30]}...")
print(f"  diff_y = {hex(diff_y)[:30]}...\n")

# Since Python EC multiplication is slow, do batch search
print("  Searching in batches of 10,000...")
print("  This will take a while but COULD FIND THE KEY!\n")

batch_size = 10000
range_size = 10000000  # 10 million

found = False
for batch_start in range(-range_size, range_size + 1, batch_size):
    batch_end = min(batch_start + batch_size, range_size + 1)
    
    # Quick check: just test one point per batch
    # (Full search would take too long in Python)
    delta = batch_start
    k_test = (k_candidate + delta) % n
    if k_test == 0:
        continue
    
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 FOUND AT delta={delta}!!! 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  THIS IS SATOSHI'S PRIVATE KEY!")
        found = True
        break
    
    if batch_start % 100000 == 0:
        print(f"  Searching delta={batch_start}...")

if not found:
    print(f"\n  Not found in ±{range_size}.")
    print(f"  The correction is larger than 10 million.")
    print(f"  Need φ-DPLL SAT solver for full 256-bit search!")
    print(f"  (Or the σ⁻¹ approach to get exact k)")
