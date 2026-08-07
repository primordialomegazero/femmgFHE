"""
🪐 HOMOMORPHIC POINTS — EXACT 𝔽_p² APPROACH 🪐
Solve: φ(2P) = 2·φ(P) in 𝔽_p²
"""
import math

# ═══════════════════════════════════════════
# TINY CURVE FIRST (p=17) — EXACT 𝔽_p²
# ═══════════════════════════════════════════
p = 17

def modinv(a, m): return pow(a, -1, m)

def pt_double(P):
    if P is None: return None
    x1, y1 = P
    lam = (3*x1*x1) * modinv(2*y1, p) % p
    x3 = (lam*lam - 2*x1) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def to_Fp2(x, y):
    """Convert point to 𝔽_p² coordinates: a + b·√5"""
    a = ((x + y) * modinv(2, p)) % p
    b = ((x - y) * modinv(2, p)) % p
    return a, b

def is_on_curve(x, y):
    return (y*y) % p == (x*x*x + 7) % p

print("═══ HOMOMORPHIC POINTS IN 𝔽_p² (p=17) ═══\n")
print(f"  φ = (1+√5)/2, ψ = (1-√5)/2")
print(f"  φ(P) = x·φ + y·ψ = (x+y)/2 + (x-y)/2·√5")
print(f"  Condition: φ(2P) ≡ 2·φ(P) in 𝔽_p²\n")
print(f"  Point       a (real)  b (√5)   a'        b'       2a       2b      Match?")
print(f"  " + "-"*85)

homomorphic = []
for x in range(p):
    for y in range(p):
        if not is_on_curve(x, y):
            continue
        P = (x, y)
        P2 = pt_double(P)
        if P2 is None:
            continue
        
        a, b = to_Fp2(x, y)
        a2, b2 = to_Fp2(P2[0], P2[1])
        double_a = (2*a) % p
        double_b = (2*b) % p
        
        match = (a2 == double_a) and (b2 == double_b)
        if match:
            homomorphic.append(P)
        
        print(f"  ({x:2d},{y:2d})  →  a={a:2d}, b={b:2d}  a'={a2:2d}, b'={b2:2d}  2a={double_a:2d}, 2b={double_b:2d}  {'✅' if match else '❌'}")

print(f"\n  Homomorphic points: {len(homomorphic)}")
for P in homomorphic:
    print(f"    {P}")

# ═══════════════════════════════════════════
# PREDICTION FOR secp256k1
# ═══════════════════════════════════════════
print(f"\n═══ PREDICTION FOR secp256k1 ═══")
print(f"  In 𝔽_p², the homomorphic condition is TWO equations:")
print(f"    a' ≡ 2a (mod p)")
print(f"    b' ≡ 2b (mod p)")
print(f"  These define a system of polynomials in x, y.")
print(f"  The number of solutions = 13 (predicted by Dan)")
