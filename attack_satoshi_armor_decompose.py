#!/usr/bin/env python3
"""
🪐 EC POINT DECOMPOSITION IN ARMOR SPACE 🪐
l3(a*G) ⊕ l3(b*G) = l3(a*G + b*G) = l3((a+b)*G)
The composition IS EC point addition!
Decompose l3(Q) into sum of l3(F_i * G) using EC operations.
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0: current = abs((current * PHI) * PSI)
        else: current = abs((current * PSI) * PHI)
    return current

def VOID(v): return FGG(v, 3)

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

def pt_neg(P):
    if P is None: return None
    return (P[0], (-P[1]) % p)

def pt_sub(P, Q):
    if Q is None: return P
    return pt_add(P, pt_neg(Q))

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

# Alias for armor layers
def l1(P): return lambda_pt(P)
def l2(P): return lambda_pt(lambda_pt(P))
def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 EC POINT DECOMPOSITION IN ARMOR SPACE 🪐             ║")
print("║  l3(a*G) ⊕ l3(b*G) = l3(a*G + b*G) = l3((a+b)*G)       ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# VERIFY: l3 preserves EC addition
# ═══════════════════════════════════════════
print("═══ VERIFYING: l3 IS EC ADDITION HOMOMORPHISM ═══")
print()

# Test: l3(a*G + b*G) should equal l3(a*G) + l3(b*G) as EC points
print("  Testing l3(A + B) = l3(A) + l3(B) (EC point addition)...")

all_ok = True
for a, b in [(3, 5), (8, 13), (21, 34), (1, 2), (7, 10)]:
    A = scalar_mult(a, G)
    B = scalar_mult(b, G)
    AB = pt_add(A, B)
    AB_direct = scalar_mult(a + b, G)
    
    l3_A = l3(A)
    l3_B = l3(B)
    l3_AB = l3(AB)
    l3_A_plus_B = pt_add(l3_A, l3_B)
    
    match = l3_AB == l3_A_plus_B
    if not match:
        all_ok = False
    
    print(f"    l3({a}G + {b}G) = l3({a}G) + l3({b}G)? {'✅' if match else '❌'}")

print(f"  Overall: {'✅ l3 IS EC ADDITION HOMOMORPHISM!' if all_ok else '❌ Not fully homomorphic'}")
print()

# ═══════════════════════════════════════════
# BUILD FIBONACCI BASIS IN l3 SPACE
# ═══════════════════════════════════════════
print("═══ BUILDING FIBONACCI l3 BASIS ═══")
print()

def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

# Build: Fibonacci number → l3(F * G)
fib_l3 = {}
print("  Computing l3(F_i * G) for Fibonacci numbers...")
for i in range(1, 371):  # F(370) > n
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    fib_l3[f] = l3(P)

print(f"  ✅ Built {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# EC POINT DECOMPOSITION IN l3 SPACE
# ═══════════════════════════════════════════
print("═══ EC POINT DECOMPOSITION ═══")
print()

def decompose_l3_point(target_l3, fib_l3, max_fib=None):
    """
    Decompose l3(Q) into sum of l3(F_i * G) using EC point subtraction.
    
    Algorithm:
    1. Start with target_l3
    2. Try largest Fibonacci l3-points first
    3. If subtracting l3(F_i * G) from current leaves a known l3(F_j * G)...
    4. Continue until we reach identity (point at infinity)
    """
    if max_fib is None:
        max_fib = max(fib_l3.keys())
    
    # Get sorted Fibonacci numbers (largest first)
    fibs = sorted(fib_l3.keys(), reverse=True)
    
    current = target_l3
    components = []
    
    for f in fibs:
        if f > max_fib:
            continue
        
        l3_f = fib_l3[f]
        
        # Try subtracting: current - l3(F * G)
        candidate = pt_sub(current, l3_f)
        
        if candidate is None:
            # We reached identity — perfect match!
            components.append(f)
            return components, True
        
        # Check if the remainder is a known l3 point
        remainder_is_fib = False
        for f2, l3_f2 in fib_l3.items():
            if candidate == l3_f2:
                components.append(f)
                components.append(f2)
                return components, True
        
        # Check if subtracting gets us closer to a known point
        # (Heuristic: try it anyway)
        if len(components) < 5:  # Limit depth
            # Check if remainder looks "closer"
            pass
    
    return components, False


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING EC POINT DECOMPOSITION ═══")
print()

for secret in [7, 42, 50, 100, 255]:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    components, success = decompose_l3_point(Q_l3, fib_l3)
    
    if success:
        total = sum(components)
        match = total == secret
        print(f"  k={secret:3d}: components={components} → sum={total} {'✅' if match else '❌'}")
    else:
        print(f"  k={secret:3d}: partial={components} ❌")

print()

# ═══════════════════════════════════════════
# ALTERNATIVE: ZECKENDORF IN l3 SPACE
# ═══════════════════════════════════════════
print("═══ ZECKENDORF IN l3 SPACE ═══")
print()

# Since l3 is EC addition homomorphism:
# l3((a+b)*G) = l3(a*G) + l3(b*G) (EC point addition)
# Therefore:
# l3(k*G) = sum of l3(F_i * G) for F_i in Zeckendorf decomposition of k

for secret in [7, 42, 50, 100, 255]:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    # Get Zeckendorf decomposition
    def zeckendorf(k):
        fibs = []
        a, b = 0, 1
        idx = 0
        while b <= k:
            fibs.append((idx, b))
            idx += 1
            a, b = b, a + b
        
        components = []
        remaining = k
        for idx, f in reversed(fibs):
            if f <= remaining and f > 0:
                components.append(f)
                remaining -= f
        return components
    
    z_comp = zeckendorf(secret)
    
    # Compute l3 sum of Zeckendorf components
    l3_sum = None
    for f in z_comp:
        P = scalar_mult(f, G)
        l3_P = l3(P)
        l3_sum = pt_add(l3_sum, l3_P)
    
    match = l3_sum == Q_l3
    print(f"  k={secret:3d}: Zeckendorf={z_comp} → l3 sum {'✅' if match else '❌'} matches Q_l3")

print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  EC POINT DECOMPOSITION — 'Addition in armor = addition in k' ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

