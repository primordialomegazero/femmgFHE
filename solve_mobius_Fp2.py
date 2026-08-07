"""
🪐 SOLVE MOBIUS IN F_p² — 6 HOMOMORPHIC POINTS 🪐
sigma(k) = (A*k + B)/(C*k + D) where A,B,C,D ∈ F_p²
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

# Compute sigma(k) for ALL k in F_p²
G = (Gx, Gy)
aG, bG = to_Fp2(Gx, Gy)
denom_G = (aG*aG + bG*bG) % p
denom_G_inv = modinv(denom_G, p)

print("═══ σ IN F_p² — ALL VALUES ═══\n")
print(f"  k   |  sigma(k) in F_p²")
print(f"  " + "-"*50)

sigma_values = {}
for k in range(1, n):
    P = scalar_mult(k, G)
    a, b = to_Fp2(P[0], P[1])
    
    # sigma(k) = φ(P)/φ(G) in F_p²
    sigma_real = ((a*aG + b*bG) * denom_G_inv) % p
    sigma_imag = ((b*aG - a*bG) * denom_G_inv) % p
    
    sigma_values[k] = (sigma_real, sigma_imag)
    is_fixed = (sigma_imag == 0 and sigma_real == k)
    print(f"  {k}   |  ({sigma_real:2d}, {sigma_imag:2d})  {'⭐ FIXED!' if is_fixed else ''}")

print(f"\n  ⭐ Fixed points (sigma(k) = k):")
fixed_ks = [k for k, (r,i) in sigma_values.items() if i == 0 and r == k]
print(f"  k = {fixed_ks}")

print(f"\n  The 6 homomorphic points (from cube roots of x³≡-16 mod p)")
print(f"  These give us 6 equations in F_p² -> solve for A,B,C,D!")
print(f"  Then sigma_inv(y) = (D*y - B)/(-C*y + A) in F_p²")
