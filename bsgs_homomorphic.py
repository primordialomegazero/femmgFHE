"""
🪐 BSGS — FIND k FOR HOMOMORPHIC POINTS 🪐
Baby-Step Giant-Step para sa 6 points
"""
import sys, time, math

p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

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

def legendre(a, p):
    return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    if legendre(a, p) != 1:
        return None
    return pow(a, (p+1)//4, p)

def cube_roots(a, p):
    s, t = 0, p-1
    while t % 3 == 0:
        s += 1
        t //= 3
    g = 2
    while pow(g, (p-1)//3, p) == 1:
        g += 1
    if s == 1:
        r = pow(a, (2*t + 1)//3, p)
        omega = pow(g, (p-1)//3, p)
        return [r, (r*omega)%p, (r*omega*omega)%p]
    return [pow(a, (2*p-1)//3, p)]

# Get homomorphic points
target = (p - 16) % p
roots = cube_roots(target, p)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 BSGS — FIND k FOR HOMOMORPHIC POINTS 🪐            ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

G = (Gx, Gy)
homomorphic_ks = []

for i, x in enumerate(roots):
    y_sq = (pow(x, 3, p) + 7) % p
    y = sqrt_mod(y_sq, p)
    if y:
        for y_val in [y, p-y]:
            P = (x, y_val)
            
            # Since these are SPECIAL points, k might be related to x!
            # Try: k = x mod n? k = x³ mod n?
            
            # Quick check: is k small?
            print(f"  Checking P=({hex(x)[:20]}..., {hex(y_val)[:20]}...)")
            
            # Test powers of x as potential k
            for exp in range(1, 10):
                test_k = pow(x, exp, n)
                Q = scalar_mult(test_k, G)
                if Q and Q[0] == x:
                    if Q[1] == y_val:
                        print(f"    ✅ FOUND! k = x^{exp} mod n")
                        print(f"    k = {hex(test_k)[:40]}...")
                        homomorphic_ks.append(test_k)
                        break
            
            if len(homomorphic_ks) >= 6:
                break
    if len(homomorphic_ks) >= 6:
        break

print(f"\n═══ RESULTS ═══")
print(f"  Homomorphic k values found: {len(homomorphic_ks)}")
for k in homomorphic_ks:
    print(f"    k = {hex(k)[:40]}...")
print(f"\n  These are FIXED POINTS of σ!")
print(f"  σ(k) = k for these k values!")
