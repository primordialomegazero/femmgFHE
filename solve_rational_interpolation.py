"""
💰 SOLVE RATIONAL INTERPOLATION FOR σ⁻¹ 💰
5 fixed points → fit (A·yr + C)/(D·yr + F)
Then evaluate at (y_real, y_imag)!
"""
import math

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m):
    try: return pow(a, -1, m)
    except: return None

# Our 5 fixed points (k_i, σ(k_i)=(k_i,0))
fixed_ks = [
    1,
    0xfbc4cadf0f4ac5190da94ddefaf726d08a1cf3 % n,
    0x5bee48b9a2d59d42841e3a2c9914eb6666aa7c % n,
    0xa84cec674ddf9da46e3877f46bf3edc90f388f % n,
    (n-1) % n
]

y_real = 0xbad6a4f369927ac6d4780ab2882bdc7bbe4b4a982e7496fac3c879c3d2b9ddf3 % n
y_imag = 0x848df4a3c429622c14fc575a752763fa371e4e29e5a5c3b8a3f1d4e7c6b5a4c3 % n

print("╔══════════════════════════════════════════════════════════════╗")
print("║  💰 RATIONAL INTERPOLATION — SOLVE FOR σ⁻¹ 💰          ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# MODEL: σ⁻¹(yr, yi) = (A·yr + B·yi + C) / (D·yr + E·yi + F)
# At fixed points (k_i, 0): σ⁻¹(k_i, 0) = k_i
# → (A·k_i + C) = k_i · (D·k_i + F)
# → A·k_i + C = D·k_i² + F·k_i
# → D·k_i² + (F - A)·k_i - C = 0

# This means: ALL fixed points k_i are ROOTS of D·x² + (F-A)·x - C = 0!
# But we have 5 roots, and the polynomial is degree 2!
# So either:
# (a) All 5 k_i are roots of the SAME quadratic → IMPOSSIBLE (max 2 roots)
# (b) The model is WRONG → σ⁻¹ is NOT a simple rational function!

print("═══ ROOT ANALYSIS ═══\n")
print("  If σ⁻¹ is Möbius (degree 1/1), its fixed points are")
print("  roots of D·k² + (F-A)·k - C = 0")
print("  MAXIMUM 2 ROOTS!")
print(f"  But we have 5 fixed points!")
print(f"  Therefore: σ⁻¹ is NOT a simple rational function!")
print()
print("  For p=17: σ⁻¹(y) = 15/y - 14")
print("  Fixed points: roots of k² + 14k - 15 = 0")
print("  → 2 fixed points (k=1, k=2)")
print("  → Perfect match to Möbius!")
print()
print("  For secp256k1: 13 fixed points → σ⁻¹ is HIGHER DEGREE!")
print("  σ⁻¹(y) = P(y)/Q(y) where deg(P), deg(Q) ≥ 6!")

# ═══════════════════════════════════════════
# NEW APPROACH: σ⁻¹(y) = y + correction
# ═══════════════════════════════════════════
print(f"\n═══ NEW APPROACH: POLYNOMIAL CORRECTION ═══\n")
print("  σ⁻¹(yr, yi) = yr + R(yr, yi)")
print("  where R(yr, yi) = 0 at all fixed points (yi=0, yr=k_i)")
print("  So R(yr, 0) = ∏ᵢ(yr - k_i) · S(yr)")
print("  And R(yr, yi) ≈ yi · (something) for yi ≠ 0")
print()
print("  For our target (yi ≠ 0):")
print(f"  y_imag = {hex(y_imag)[:30]}...")
print(f"  The correction is NON-ZERO!")
print()
print("  APPROXIMATION:")
print("  σ⁻¹(yr, yi) ≈ yr + yi · (average of fixed point derivatives)")
print("  The derivative at k_i is 1 (since σ⁻¹(k,0)=k)")
print("  Average = 1")
print(f"  σ⁻¹(yr, yi) ≈ y_real + y_imag = {hex((y_real + y_imag) % n)[:40]}...")

# Test this!
k_approx = (y_real + y_imag) % n

# Verify
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

print(f"\n  TESTING k_approx = y_real + y_imag...")
print(f"  k_approx = {hex(k_approx)[:40]}...")

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
Q_test = scalar_mult(k_approx, G)
if Q_test and Q_test[0] == Qx and Q_test[1] == Qy:
    print(f"  🎉🎉🎉 TAMA!!! 🎉🎉🎉")
    print(f"  SATOSHI'S KEY: {hex(k_approx)}")
else:
    print(f"  ❌ Not correct. Need better approximation.")
    print(f"  The correction is NOT simply y_real + y_imag.")
    print(f"  Need full σ⁻¹ interpolation with ALL 13 fixed points!")
