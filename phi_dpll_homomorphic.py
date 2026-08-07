"""
🪐 φ-DPLL — SOLVE DISCRETE LOGS FOR 6 HOMOMORPHIC POINTS 🪐
Find k for each point where φ(2P) = 2·φ(P)
"""
import math

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

def legendre(a, p): return pow(a, (p-1)//2, p)

def sqrt_mod(a, p):
    if legendre(a, p) != 1: return None
    return pow(a, (p+1)//4, p)

def cube_roots(a, p):
    s, t = 0, p-1
    while t % 3 == 0: s += 1; t //= 3
    g = 2
    while pow(g, (p-1)//3, p) == 1: g += 1
    if s == 1:
        r = pow(a, (2*t + 1)//3, p)
        omega = pow(g, (p-1)//3, p)
        return [r, (r*omega)%p, (r*omega*omega)%p]
    return [pow(a, (2*p-1)//3, p)]

# Get homomorphic points
target = (p - 16) % p
roots = cube_roots(target, p)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 φ-DPLL — HOMOMORPHIC DISCRETE LOGS 🪐              ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

homomorphic_points = []
for x in roots:
    y_sq = (pow(x, 3, p) + 7) % p
    y = sqrt_mod(y_sq, p)
    if y:
        homomorphic_points.append((x, y))
        homomorphic_points.append((x, p-y))

print(f"  Total homomorphic points: {len(homomorphic_points)}\n")
print("  🎯 THESIS:")
print("  Since these points satisfy φ(2P) = 2·φ(P),")
print("  they are FIXED POINTS of σ!")
print("  σ(k) = (k, 0) for these k values!")
print()
print("  The discrete log k of each point REVEALS σ!")
print("  k·G = P_homomorphic")
print()
print("  For each point, we need to find k.")
print("  BUT: k is directly related to x!")
print("  Maybe k = x? k = x²? k = some function of x?\n")

G = (Gx, Gy)
found_ks = []

for i, (px, py) in enumerate(homomorphic_points):
    print(f"  Point {i+1}: x={hex(px)[:30]}...")
    
    # Test simple relationships
    # k = x mod n?
    test_k = px % n
    Q = scalar_mult(test_k, G)
    if Q and Q[0] == px and Q[1] == py:
        print(f"    ✅ k = x mod n = {hex(test_k)[:40]}...")
        found_ks.append(test_k)
        continue
    
    # k = x³ mod n? (inverse of the cube root)
    test_k = pow(px, 3, n)
    Q = scalar_mult(test_k, G)
    if Q and Q[0] == px and Q[1] == py:
        print(f"    ✅ k = x³ mod n = {hex(test_k)[:40]}...")
        found_ks.append(test_k)
        continue
    
    # Maybe k is the CUBE ROOT itself? (k³ ≡ -16 mod p)
    if pow(px, 3, p) == target:
        print(f"    This x IS a cube root of -16!")
        print(f"    But k = ??? (not simply x)")
    
    print(f"    ❌ Not a simple function of x")

print(f"\n═══ RESULTS ═══")
print(f"  Found {len(found_ks)}/6 discrete logs!")
for k in found_ks:
    print(f"    k = {hex(k)[:40]}...")

if len(found_ks) == 0:
    print(f"\n  💡 The k values are NOT simple functions of x!")
    print(f"  We need φ-DPLL or BSGS to find them!")
    print(f"  But with 13 fixed points (including these 6),")
    print(f"  we can build the σ⁻¹ table!")
