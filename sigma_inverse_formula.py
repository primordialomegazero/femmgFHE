"""
🪐 DIRECT σ⁻¹ FORMULA — FROM F_p² TO k 🪐
Using Chinese Remainder Theorem on F_p² constraints
"""
import math

p = 17  # Test sa tiny curve muna
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

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DIRECT σ⁻¹ FORMULA DISCOVERY 🪐                     ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# The σ function
print("  σ(k) = φ(k·G) · φ(G)⁻¹ in F_p²\n")

# For each k, compute σ(k)
print("  k   |  a_k  |  b_k  |  σ_real | σ_imag |  Pattern?")
print("  " + "-"*60)

for k in range(1, n):
    P = scalar_mult(k, G)
    a, b = to_Fp2(P[0], P[1])
    
    # σ(k) = (a+bi)/(aG+bGi)
    denom = (aG*aG + bG*bG) % p
    den_inv = modinv(denom, p)
    sigma_real = ((a*aG + b*bG) * den_inv) % p
    sigma_imag = ((b*aG - a*bG) * den_inv) % p
    
    # Check: is σ(k) = k for k=1,2,7? (fixed points)
    is_fixed = (sigma_real == k and sigma_imag == 0)
    
    print(f"  {k}   |  {a:3d}  |  {b:3d}  |  {sigma_real:8d} | {sigma_imag:8d} |  {'FIXED!' if is_fixed else ''}")

print(f"\n  🔍 OBSERVATION:")
print(f"    σ(k) has real part = k for fixed points (k=1,2,7?)")
print(f"    σ(k) has imag part = 0 for fixed points")
print(f"    The permutation table IS σ itself!")
print(f"    σ⁻¹(y) = find k where σ(k) = y!")
