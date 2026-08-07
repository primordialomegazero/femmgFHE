#!/usr/bin/env python3
"""
🪐 RECURSIVE FIBONACCI DECOMPOSITION 🪐
"Mortal sin": Direct DLOG is hard → DECOMPOSE instead!
Decrypt-Reencrypt → Bootstrap → Unlimited FHE
Trace Erasure → Structural iO
Fibonacci Decomposition → Private Key Recovery
'Gamitin ang kahinaan para palakasin sila.'
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
print("║  🪐 RECURSIVE FIBONACCI DECOMPOSITION 🪐                 ║")
print("║  'Ang mortal sin ang susi sa holy grail'                 ║")
print("║  Decrypt-Reencrypt → FHE | Trace Erase → iO | Decompose → k ║")
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
    l3_to_fib[l3_P[0]] = f

fibs_descending = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# RECURSIVE DECOMPOSITION
# ═══════════════════════════════════════════
def recursive_decompose(current_l3, available_fibs, depth=0, max_depth=5):
    """
    RECURSIVE FIBONACCI DECOMPOSITION:
    
    "Ang mortal sin": i-decompose ang k sa Fibonacci basis.
    Hindi mo kailangan i-solve ang DLOG — i-decompose mo lang!
    
    Algorithm:
    1. Kung current_l3 is None (identity) → success! Return []
    2. Kung max_depth reached → failure
    3. Para sa bawat F_i sa available_fibs:
       a. remainder = current_l3 - l3(F_i * G)
       b. RECURSE: sub_components = recursive_decompose(remainder, F_i_fibs, depth+1)
       c. Kung success → return [F_i] + sub_components
       d. Kung failure → subukan ang susunod na F_i
    4. Return None (walang valid decomposition)
    """
    indent = "  " * depth
    
    # Base case: reached identity
    if current_l3 is None:
        return []
    
    # Base case: max depth
    if depth >= max_depth:
        return None
    
    # Try each Fibonacci number (largest first, NON-CONSECUTIVE)
    for i, f in enumerate(available_fibs):
        l3_f = fib_l3[f]
        
        # Subtract
        remainder = pt_sub(current_l3, l3_f)
        
        # RECURSE: try to decompose the remainder
        # IMPORTANT: Skip F_{i+1} to enforce non-consecutive (Zeckendorf)
        remaining_fibs = available_fibs[i+2:]  # Skip next one!
        
        sub_components = recursive_decompose(remainder, remaining_fibs, depth + 1, max_depth)
        
        if sub_components is not None:
            # Success! This F_i is part of k
            return [f] + sub_components
    
    # No valid decomposition found
    return None


# ═══════════════════════════════════════════
# WRAPPER: Try different max depths
# ═══════════════════════════════════════════
def decompose_k(Q_l3, fibs_descending, l3_to_fib, max_depth=5):
    """
    Decompose Q_l3 into Fibonacci components.
    Try increasing max_depth until success.
    """
    for depth in range(1, max_depth + 1):
        result = recursive_decompose(Q_l3, fibs_descending, 0, depth)
        if result is not None:
            return result, depth
    return None, max_depth


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING RECURSIVE DECOMPOSITION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, depth_used = decompose_k(Q_l3, fibs_descending, l3_to_fib, max_depth=5)
    elapsed = time.time() - start
    
    if components is not None:
        total = sum(components)
        match = total == secret
        
        if match:
            print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
        else:
            print(f"  ❌ WRONG SUM: got {total} = {' + '.join(map(str, components))}")
        
        print(f"  Depth used: {depth_used}, Time: {elapsed:.6f}s")
    else:
        print(f"  ❌ NO DECOMPOSITION FOUND (max depth reached)")
        print(f"  Time: {elapsed:.6f}s")
    
    print()
    results.append((secret, components, sum(components) if components else 0))

print(f"═══ SUMMARY ═══")
print(f"  {'Secret':<10} {'Components':<30} {'Sum':<10} {'Match':<8}")
print(f"  {'-'*60}")
correct = 0
for secret, comps, total in results:
    match = total == secret
    comps_str = str(comps)[:30] if comps else 'None'
    print(f"  {secret:<10} {comps_str:<30} {total:<10} {'✅' if match else '❌':<8}")
    if match: correct += 1
print(f"\n  Accuracy: {correct}/{len(test_keys)}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  RECURSIVE — 'Decompose, don't solve'                    ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

