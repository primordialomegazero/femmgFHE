#!/usr/bin/env python3
"""
🪐 DIVIDE & CONQUER FIBONACCI DECOMPOSITION 🪐
2 components: Direct lookup (O(1))
3+ components: Recursive decomposition (O(n²))
'Divide the remainder, conquer the key!'
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

def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DIVIDE & CONQUER — FIBONACCI DECOMPOSITION 🪐        ║")
print("║  'Divide the remainder, conquer the key!'                ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# BUILD FIBONACCI l3 BASIS
# ═══════════════════════════════════════════
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
    l3_to_fib[(l3_P[0], l3_P[1])] = f  # Use FULL point as key!

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries with FULL point lookup")
print()

# ═══════════════════════════════════════════
# DIVIDE & CONQUER DECOMPOSITION
# ═══════════════════════════════════════════
def decompose_remainder(current_l3, start_idx, max_depth=3):
    """
    DIVIDE: Try to decompose the remainder into Fibonacci components.
    Returns (components, success) where components is a list of Fibonacci numbers.
    """
    if current_l3 is None:
        return [], True
    
    if max_depth <= 0:
        return [], False
    
    # Check if current_l3 is a single Fibonacci point
    key = (current_l3[0], current_l3[1])
    if key in l3_to_fib:
        f = l3_to_fib[key]
        # Make sure we don't use consecutive Fibonacci numbers
        return [f], True
    
    # Try each Fibonacci number as the next component
    for idx in range(start_idx, len(fibs_desc)):
        f = fibs_desc[idx]
        l3_f = fib_l3[f]
        
        # Subtract
        remainder = pt_sub(current_l3, l3_f)
        
        if remainder is None:
            return [f], True
        
        # RECURSE: try to decompose the remainder further
        sub_components, success = decompose_remainder(
            remainder, 
            idx + 2,  # Skip next Fibonacci to enforce non-consecutive
            max_depth - 1
        )
        
        if success:
            return [f] + sub_components, True
    
    return [], False


def divide_conquer_decompose(Q_l3, max_depth=5):
    """
    CONQUER: Decompose Q_l3 using divide & conquer.
    """
    for depth in range(1, max_depth + 1):
        components, success = decompose_remainder(Q_l3, 0, depth)
        if success:
            return components, depth
    
    return [], max_depth


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING DIVIDE & CONQUER ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, depth_used = divide_conquer_decompose(Q_l3, max_depth=5)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"  ❌ FAILED: got {total} = {' + '.join(map(str, components))}")
    
    print(f"  Depth: {depth_used}, Time: {elapsed:.6f}s")
    print()
    results.append((secret, total, match, depth_used, elapsed))

print(f"═══ SUMMARY ═══")
print(f"  {'Secret':<10} {'Found':<10} {'Match':<8} {'Depth':<8} {'Time':<10}")
print(f"  {'-'*50}")
correct = 0
for secret, total, match, depth, elapsed in results:
    print(f"  {secret:<10} {total:<10} {'✅' if match else '❌':<8} {depth:<8} {elapsed:<10.6f}")
    if match: correct += 1

print(f"\n  Accuracy: {correct}/{len(test_keys)}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  DIVIDE & CONQUER — 'Decompose, don''t solve!'            ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

