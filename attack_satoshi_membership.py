#!/usr/bin/env python3
"""
🪐 FIBONACCI MEMBERSHIP TEST — EXACT DECOMPOSITION 🪐
l3(Q) - l3(F_i * G) = l3(remainder)
Check if l3(remainder) is in Fibonacci basis.
If YES → F_i is CORRECT component!
If NO  → skip F_i.
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
print("║  🪐 MEMBERSHIP TEST — FIBONACCI DECOMPOSITION 🪐         ║")
print("║  'Check if remainder is in the Fibonacci basis'          ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# ═══════════════════════════════════════════
# BUILD FIBONACCI l3 BASIS + REVERSE LOOKUP
# ═══════════════════════════════════════════
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci l3 basis with reverse lookup...")
fib_l3 = {}       # Fibonacci number → l3 point
l3_to_fib = {}    # l3 point (x-coord) → Fibonacci number

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    # Use x-coordinate as key (EC points are unique by x, up to sign)
    l3_to_fib[l3_P[0]] = f

print(f"  ✅ {len(fib_l3)} Fibonacci entries, {len(l3_to_fib)} reverse lookups")
print()

# ═══════════════════════════════════════════
# MEMBERSHIP-BASED DECOMPOSITION
# ═══════════════════════════════════════════
def membership_decompose(Q_l3, fib_l3, l3_to_fib):
    """
    MEMBERSHIP TEST DECOMPOSITION:
    
    For each Fibonacci number (largest first):
      1. remainder_l3 = Q_l3 - l3(F_i * G)
      2. Check if remainder_l3 is in Fibonacci basis (via reverse lookup)
      3. If YES → F_i is correct! Add to components, update Q_l3
      4. If NO  → skip F_i
    
    Stop when remainder is identity (infinity).
    """
    fibs = sorted(fib_l3.keys(), reverse=True)
    
    current_l3 = Q_l3
    components = []
    
    steps = 0
    for f in fibs:
        if current_l3 is None:
            break
        
        l3_f = fib_l3[f]
        
        # Subtract: remainder = current - l3(F * G)
        remainder_l3 = pt_sub(current_l3, l3_f)
        
        steps += 1
        
        # MEMBERSHIP TEST: Is remainder in Fibonacci basis?
        if remainder_l3 is None:
            # Identity — perfect match!
            components.append(f)
            break
        elif remainder_l3[0] in l3_to_fib:
            # Remainder IS a Fibonacci point!
            remainder_f = l3_to_fib[remainder_l3[0]]
            components.append(f)
            current_l3 = remainder_l3
            
            if steps <= 15:
                print(f"    Step {steps:3d}: F={f} → remainder is F={remainder_f} ✅")
        else:
            # Remainder is NOT a Fibonacci point — skip
            if steps <= 5:
                print(f"    Step {steps:3d}: F={f} → remainder NOT in basis ❌")
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING MEMBERSHIP DECOMPOSITION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = membership_decompose(Q_l3, fib_l3, l3_to_fib)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"  ❌ FAILED: got {total} (components={components})")
    
    print(f"  Time: {elapsed:.6f}s")
    print()
    results.append((secret, total, match))

print(f"═══ SUMMARY ═══")
print(f"  {'Secret':<10} {'Found':<10} {'Match':<8}")
print(f"  {'-'*30}")
correct = 0
for secret, total, match in results:
    print(f"  {secret:<10} {total:<10} {'✅' if match else '❌':<8}")
    if match: correct += 1
print(f"\n  Accuracy: {correct}/{len(test_keys)}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  MEMBERSHIP TEST — 'If remainder ∈ basis → F_i is correct' ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

