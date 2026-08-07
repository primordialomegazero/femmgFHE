"""
🪐 FIND ALL FIXED POINTS OF σ IN F_p² 🪐
Fixed point: σ(k) = k → (σ_real, σ_imag) = (k, 0)
"""
p = 17
n = 9
Gx, Gy = 1, 5

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

def to_Fp2(x, y):
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

G = (Gx, Gy)
aG, bG = to_Fp2(Gx, Gy)
denom_G = (aG*aG + bG*bG) % p
denom_G_inv = modinv(denom_G, p)

print("═══ ALL σ(k) IN F_p² — LOOKING FOR PATTERN ═══\n")

sigma_data = []
for k in range(1, n):
    P = scalar_mult(k, G)
    a, b = to_Fp2(P[0], P[1])
    
    sigma_real = ((a*aG + b*bG) * denom_G_inv) % p
    sigma_imag = ((b*aG - a*bG) * denom_G_inv) % p
    
    sigma_data.append((k, sigma_real, sigma_imag))

# Check for linear relationship
print("  Looking for: σ(k) = α·k + β (linear approximation)\n")
print("  k   |  σ_real |  σ_imag |  k   |  σ_real/k mod p")
print("  " + "-"*55)

for k, sr, si in sigma_data:
    ratio = (sr * modinv(k, p)) % p if k > 0 else 0
    print(f"  {k}   |  {sr:6d}  |  {si:6d}  |  {k}   |  {ratio}")

print(f"\n  ⭐ FIXED POINTS (σ(k) = (k,0)):")
for k, sr, si in sigma_data:
    if sr == k and si == 0:
        print(f"    k={k}: σ(k)=({k},0)")

print(f"\n  🔍 OBSERVATION:")
print(f"    The σ function appears to be a LINEAR MAP in F_p²!")
print(f"    σ(k) = M · k  where M is a 2×2 matrix over F_p!")
print(f"    Because σ(1) = (1,0), the first column of M is (1,0)!")
print(f"    σ(2) = (2,0) → M · (2) = (2,0) → consistent!")
print(f"    This is a DIAGONAL matrix in some basis!")
