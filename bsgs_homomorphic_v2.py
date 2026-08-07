"""
🪐 BSGS v2 — FIND k FOR 6 HOMOMORPHIC POINTS 🪐
Optimized: search small range muna, then expand!
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

target = (p - 16) % p
roots = cube_roots(target, p)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 BSGS v2 — HOMOMORPHIC DISCRETE LOGS 🪐             ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

homomorphic_points = []
for x in roots:
    y_sq = (pow(x, 3, p) + 7) % p
    y = sqrt_mod(y_sq, p)
    if y:
        homomorphic_points.append((x, y))
        homomorphic_points.append((x, p-y))

print(f"  Total homomorphic points: {len(homomorphic_points)}")
print(f"  These are FIXED POINTS of σ → σ(k) = (k,0)\n")

G = (Gx, Gy)
found_ks = {}

# BSGS parameters
BABY_STEPS = 1000000  # 1 million baby steps
GIANT_STEPS = 1000    # 1000 giant steps

for idx, (px, py) in enumerate(homomorphic_points):
    print(f"  Point {idx+1}: x={hex(px)[:30]}...")
    
    # Step 1: Pre-compute baby steps (k = 0..BABY_STEPS-1)
    print(f"    Computing {BABY_STEPS} baby steps...")
    baby_steps = {}
    current = None  # O (point at infinity) for k=0
    
    for k in range(BABY_STEPS):
        if k == 0:
            continue  # Skip k=0
        current = pt_add(current, G) if current else G
        baby_steps[current[0]] = k  # Store x-coordinate → k
        
        if k % 200000 == 0:
            print(f"      Baby step {k}...")
    
    # Step 2: Giant steps
    print(f"    Computing {GIANT_STEPS} giant steps...")
    # Compute -BABY_STEPS * G
    neg_baby = scalar_mult(BABY_STEPS, G)
    if neg_baby:
        neg_baby = (neg_baby[0], (-neg_baby[1]) % p)  # Negate
    else:
        continue
    
    P = (px, py)
    for j in range(GIANT_STEPS):
        # Check if P is in baby steps
        if P and P[0] in baby_steps:
            k = baby_steps[P[0]]
            # Verify y-coordinate
            Q = scalar_mult(k, G)
            if Q and Q[0] == px and Q[1] == py:
                print(f"    ✅ FOUND! k = {k}")
                found_ks[(px, py)] = k
                break
        
        # P = P - BABY_STEPS * G
        if P and neg_baby:
            P = pt_add(P, neg_baby)
        
        if j % 200 == 0:
            print(f"      Giant step {j}...")
    
    if (px, py) not in found_ks:
        print(f"    ❌ Not found in range 0..{BABY_STEPS * GIANT_STEPS}")

print(f"\n═══ RESULTS ═══")
print(f"  Found {len(found_ks)}/6 discrete logs!")
for (px, py), k in found_ks.items():
    print(f"    P=({hex(px)[:20]}...): k = {k}")
    print(f"      k (hex) = {hex(k)[:40]}...")
