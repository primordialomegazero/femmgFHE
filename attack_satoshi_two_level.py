#!/usr/bin/env python3
"""
🪐 TWO-LEVEL FIBONACCI LOOKUP 🪐
Level 1: Find largest F_a
Level 2: Decompose remainder into F_b + F_c
Precompute ALL Fibonacci pair sums → O(1) lookup!
"""
import sys, time, math

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
print("║  🪐 TWO-LEVEL FIBONACCI LOOKUP 🪐                        ║")
print("║  'Precompute ALL pair sums → O(1) decomposition!'        ║")
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
print(f"  ✅ {len(fib_l3)} single entries")

# ═══════════════════════════════════════════
# PRECOMPUTE ALL PAIR SUMS
# ═══════════════════════════════════════════
print("  Precomputing Fibonacci pair sums (non-consecutive)...")
pair_to_fibs = {}  # l3 point of sum → (F_a, F_b)

fib_list = sorted(fib_l3.keys())
pair_count = 0
for i, fa in enumerate(fib_list):
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):  # Non-consecutive (Zeckendorf)
        fb = fib_list[j]
        l3_fb = fib_l3[fb]
        l3_sum = pt_add(l3_fa, l3_fb)
        if l3_sum is not None:
            key = (l3_sum[0], l3_sum[1])
            if key not in pair_to_fibs:
                pair_to_fibs[key] = (fa, fb)
                pair_count += 1

print(f"  ✅ {pair_count} pair entries")
print()

# ═══════════════════════════════════════════
# TWO-LEVEL DECOMPOSITION
# ═══════════════════════════════════════════
def two_level_decompose(Q_l3, fib_l3, l3_to_fib, pair_to_fibs, fibs_desc):
    """
    TWO-LEVEL DECOMPOSITION:
    
    Level 1: Try each F_a (largest first)
    Level 2: Check if remainder is:
      a) Single Fibonacci point → DONE! (2 components)
      b) Sum of 2 Fibonacci points → DONE! (3 components)
      c) Neither → skip F_a, try next
    """
    current_l3 = Q_l3
    
    # First, check if it's a single Fibonacci point
    key = (current_l3[0], current_l3[1])
    if key in l3_to_fib:
        return [l3_to_fib[key]]
    
    # Level 1: Try each F_a
    for fa in fibs_desc:
        l3_fa = fib_l3[fa]
        remainder = pt_sub(current_l3, l3_fa)
        
        if remainder is None:
            return [fa]
        
        rem_key = (remainder[0], remainder[1])
        
        # Level 2a: Is remainder a single Fibonacci point?
        if rem_key in l3_to_fib:
            fb = l3_to_fib[rem_key]
            return [fa, fb]
        
        # Level 2b: Is remainder a sum of 2 Fibonacci points?
        if rem_key in pair_to_fibs:
            fb, fc = pair_to_fibs[rem_key]
            return [fa, fb, fc]
    
    return []


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING TWO-LEVEL DECOMPOSITION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = two_level_decompose(Q_l3, fib_l3, l3_to_fib, pair_to_fibs, fibs_desc)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"  ❌ FAILED: got {total} = {' + '.join(map(str, components))}")
    
    print(f"  Time: {elapsed:.6f}s")
    print()
    results.append((secret, total, match))

print(f"═══ SUMMARY ═══")
correct = 0
for secret, total, match in results:
    print(f"  k={secret}: {'✅' if match else '❌'} got {total}")
    if match: correct += 1
print(f"\n  Accuracy: {correct}/{len(test_keys)}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  TWO-LEVEL — 'Precompute all sums → O(1) decomposition'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

