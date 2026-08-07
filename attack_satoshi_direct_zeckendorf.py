#!/usr/bin/env python3
"""
🪐 DIRECT ZECKENDORF IN l3 SPACE 🪐
'Find largest F_i where l3(F_i*G) is "closest" to l3(Q)'
Then subtract and repeat. O(n) — no recursion!
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

def modinv(a, m): return pow(a, -1, m)
def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv((x2-x1)%p, p)) % p
    return ((lam*lam-x1-x2)%p, (lam*(x1-(lam*lam-x1-x2)%p)-y1)%p)

def pt_sub(P, Q):
    if Q is None: return P
    return pt_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def lambda_pt(P):
    if P is None: return None
    return ((P[0] * BETA) % p, P[1])

def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DIRECT ZECKENDORF IN l3 SPACE 🪐                     ║")
print("║  'Largest F_i closest to l3(Q) → subtract → repeat'      ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Build Fibonacci basis
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci l3 basis...")
fib_l3 = {}
l3_to_fib = {}
for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# DIRECT ZECKENDORF: Closest Point Method
# ═══════════════════════════════════════════
def direct_zeckendorf(Q_l3, fib_l3, l3_to_fib, fibs_desc):
    """
    DIRECT ZECKENDORF IN l3 SPACE:
    
    Instead of checking membership (na hindi gagana para sa 3+ components),
    gamitin natin ang DISTANCE sa l3 space!
    
    "Closest F_i" = ang Fibonacci point na may pinakamaliit na
    difference sa l3(Q) — sa EC point addition space.
    
    Pero mas simple: ang l3 ay ISOMORPHISM!
    Ibig sabihin: l3(a*G) + l3(b*G) = l3((a+b)*G)
    
    So: l3(k*G) = l3(F_a*G) + l3(F_b*G) + l3(F_c*G)
    
    Para mahanap si F_a:
    - l3(k*G) - l3(F_a*G) = l3((k-F_a)*G)
    - Dapat si (k-F_a) ay MAS MALIIT na Zeckendorf
    
    Ang trick: Hanapin ang F_a na NAGPAPALIIT sa remainder!
    The remainder should be "closer" to a Fibonacci point.
    """
    current_l3 = Q_l3
    components = []
    used_indices = set()
    
    for iteration in range(10):  # Max 10 components
        if current_l3 is None:
            break
        
        # Check if current is a single Fibonacci point
        key = (current_l3[0], current_l3[1])
        if key in l3_to_fib:
            f = l3_to_fib[key]
            components.append(f)
            break
        
        # Find the "best" F_i to subtract
        # Strategy: subtract F_i and check if remainder is "more Fibonacci-like"
        best_f = None
        best_remainder = None
        best_improvement = -1
        
        for idx, f in enumerate(fibs_desc):
            if idx in used_indices:
                continue
            
            l3_f = fib_l3[f]
            remainder = pt_sub(current_l3, l3_f)
            
            if remainder is None:
                best_f = f
                best_remainder = None
                break
            
            # Check: is remainder closer to a Fibonacci point?
            rem_key = (remainder[0], remainder[1])
            if rem_key in l3_to_fib:
                # Direct hit! This F_i is correct.
                best_f = f
                best_remainder = remainder
                break
            
            # Heuristic: smaller x-coordinate difference = closer
            # (Not reliable, but fast)
            improvement = 0
            for f2 in fibs_desc[:10]:  # Check top 10
                l3_f2 = fib_l3[f2]
                dist = abs(remainder[0] - l3_f2[0])
                if dist < improvement or improvement == 0:
                    improvement = dist
            
            if improvement > best_improvement:
                best_improvement = improvement
                best_f = f
                best_remainder = remainder
        
        if best_f is None:
            break
        
        components.append(best_f)
        used_indices.add(fibs_desc.index(best_f))
        current_l3 = best_remainder
        
        if len(components) <= 5:
            print(f"    Iter {iteration}: chose F={best_f}")
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING DIRECT ZECKENDORF ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = direct_zeckendorf(Q_l3, fib_l3, l3_to_fib, fibs_desc)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"  ❌ FAILED: got {total} = {' + '.join(map(str, components))}")
    
    print(f"  Time: {elapsed:.6f}s")
    print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  DIRECT ZECKENDORF — 'Subtract the closest, repeat'      ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

