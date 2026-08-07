#!/usr/bin/env python3
"""
🪐 VOID-GUIDED GREEDY DECOMPOSITION 🪐
VOID decreases as we approach the correct k.
Use VOID as compass to find Zeckendorf components!
'Bawas lang nang bawas — VOID ang nagsasabi kung tama.'
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

def VOID_point(P):
    """VOID of an EC point — 0 means 'clean' (no error)"""
    if P is None: return 0.0
    v = (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)
    return FGG(v / float(p), 3)

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
print("║  🪐 VOID-GUIDED GREEDY DECOMPOSITION 🪐                  ║")
print("║  'Bawas lang nang bawas — VOID ang nagsasabi kung tama'  ║")
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
for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    fib_l3[f] = l3(P)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# VOID-GUIDED GREEDY DECOMPOSITION
# ═══════════════════════════════════════════
def void_greedy_decompose(Q_l3, fib_l3, max_components=10):
    """
    VOID-GUIDED GREEDY DECOMPOSITION:
    
    Start with Q_l3. Try subtracting l3(F_i * G) for largest F_i.
    Check VOID of remainder. If VOID decreases → keep F_i.
    If VOID increases → skip F_i (not part of k).
    
    Continue until remainder is identity (VOID=0) or max components reached.
    """
    fibs = sorted(fib_l3.keys(), reverse=True)
    
    current_l3 = Q_l3
    current_void = VOID_point(current_l3)
    components = []
    
    print(f"  Starting VOID: {current_void:.10f}")
    print(f"  {'Step':<6} {'F_i':<12} {'VOID(before)':<18} {'VOID(after)':<18} {'Action':<10}")
    print(f"  {'-'*70}")
    
    step = 0
    for f in fibs:
        if f > max(fib_l3.keys()) or len(components) >= max_components:
            break
        
        if current_l3 is None:
            # Reached identity — done!
            break
        
        l3_f = fib_l3[f]
        
        # Try subtracting
        candidate = pt_sub(current_l3, l3_f)
        candidate_void = VOID_point(candidate)
        
        step += 1
        
        # Decision: did VOID decrease?
        if candidate_void < current_void:
            # VOID decreased — this F_i is part of k!
            components.append(f)
            current_l3 = candidate
            old_void = current_void
            current_void = candidate_void
            
            if step <= 20 or candidate_void < 0.01:
                print(f"  {step:<6} F={f:<10} {old_void:<18.10f} {current_void:<18.10f} {'✅ KEEP':<10}")
            
            # If VOID is zero, we're done!
            if current_void < 1e-12:
                break
        else:
            # VOID increased or stayed same — skip this F_i
            if step <= 5:
                print(f"  {step:<6} F={f:<10} {current_void:<18.10f} {candidate_void:<18.10f} {'❌ SKIP':<10}")
    
    return components, current_void


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING VOID-GUIDED GREEDY DECOMPOSITION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"\n  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, final_void = void_greedy_decompose(Q_l3, fib_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"\n  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"\n  ❌ FAILED: got {total} (components={components})")
    
    print(f"  Final VOID: {final_void:.12f}, Time: {elapsed:.4f}s")
    results.append((secret, total, match, final_void))

print(f"\n═══ SUMMARY ═══")
print(f"  {'Secret':<10} {'Found':<10} {'Match':<8} {'VOID':<15}")
print(f"  {'-'*45}")
correct = 0
for secret, total, match, void_val in results:
    print(f"  {secret:<10} {total:<10} {'✅' if match else '❌':<8} {void_val:<15.12f}")
    if match: correct += 1
print(f"\n  Accuracy: {correct}/{len(test_keys)}")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  VOID-GUIDED GREEDY — 'VOID knows the way home'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

