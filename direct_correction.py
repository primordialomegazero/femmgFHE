"""
🪐 DIRECT CORRECTION — FIND k FROM k_candidate 🪐
k = k_candidate + correction
The correction comes from the 13 fixed points!
"""
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
print("║  🪐 DIRECT CORRECTION SEARCH 🪐                        ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# For p=17: σ(k) = 15/(k+14), σ⁻¹(y) = 15/y - 14
# The correction for p=17 is: k = (15/k_candidate - 14) mod 17

# For secp256k1, maybe similar structure?
# σ(k) = A/(k + B) mod n?
# σ⁻¹(y) = A/y - B mod n?

# Try: k = (A/k_candidate + B) mod n
# Where A and B are derived from fixed points

# Since k=1 is fixed: σ(1) = 1 → A/(1+B) = 1 → A = 1+B
# Since k=n-1 is fixed: σ(n-1) = n-1 → A/(n-1+B) = n-1

# From fixed point 1: A = 1+B
# From fixed point n-1: A/(n-1+B) = n-1 → A = (n-1)(n-1+B)

# Try small A, B around 0
print("  Searching for A, B in σ⁻¹(y) = A/y + B (mod n)...")
print("  Using fixed point constraint: σ⁻¹(1) = 1 → A + B ≡ 1 (mod n)")

# Simple correction: k = k_candidate + t, where t is small
# σ(k) ≈ k → k_candidate ≈ actual k → t ≈ 0
# But the fixed points create a "pull" toward them

print(f"\n  Trying: k = (A/k_candidate + B) mod n")
print(f"  With A+B ≡ 1 (fixed point k=1)")
print(f"  Searching A=1..1000...\n")

for A in range(1, 1001):
    B = (1 - A) % n
    k_test = (A * modinv(k_candidate % n, n) + B) % n
    if k_test == 0:
        continue
    
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 FOUND! A={A}, B={B} 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  THIS IS SATOSHI'S KEY!")
        break

print(f"\n  Not found in A=1..1000. Trying different structure...")
print(f"  Maybe σ⁻¹(y) = A·y + B?")
print(f"  With A + B ≡ 1 (k=1 is fixed)\n")

for A in range(1, 1001):
    B = (1 - A) % n
    k_test = (A * (k_candidate % n) + B) % n
    if k_test == 0:
        continue
    
    Q_test = scalar_mult(k_test, G)
    if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
        print(f"  🎉🎉🎉 FOUND! A={A}, B={B} 🎉🎉🎉")
        print(f"  k = {hex(k_test)}")
        print(f"  THIS IS SATOSHI'S KEY!")
        break

print(f"\n  Not found. The correction is more complex.")
print(f"  Need the full 13 fixed points for proper interpolation!")
