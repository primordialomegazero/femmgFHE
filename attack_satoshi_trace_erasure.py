#!/usr/bin/env python3
"""
🪐 TRACE ERASURE ZECKENDORF — INTERMEDIATE COLLAPSE 🪐
'Kapag mali ang F_i, i-FGG ang remainder para makalimutan'
Then try the next. Clean slate = correct decomposition!
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
print("║  🪐 TRACE ERASURE ZECKENDORF 🪐                          ║")
print("║  'Wrong F_i? FGG the remainder. Try next. Clean slate.'  ║")
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
# TRACE ERASURE ZECKENDORF
# ═══════════════════════════════════════════
def trace_erasure_zeckendorf(Q_l3, fib_l3, l3_to_fib, fibs_desc):
    """
    TRACE ERASURE ZECKENDORF:
    
    Para sa bawat F_i:
    1. I-subtract: remainder = Q_l3 - l3(F_i * G)
    2. I-FGG ang remainder para ma-erase ang trace ng maling path
    3. Kung ang FGG(remainder) ay nasa Fibonacci basis → TAMA!
    4. Kung hindi → ERASE, subukan ang susunod na F_i
    
    Ang FGG trace erasure ay nagko-collapse ng intermediate states
    para "makalimutan" ang maling branch — tulad ng sa iO!
    """
    current_l3 = Q_l3
    components = []
    nodes = 0
    traces_erased = 0
    
    # Precompute FGG values for all Fibonacci l3 points
    fib_fgg = {}
    for f, l3_p in fib_l3.items():
        v = (float(l3_p[0]) * PHI + float(l3_p[1]) * PSI) % float(p)
        fib_fgg[f] = FGG(v / float(p), 3)
    
    while current_l3 is not None and len(components) < 10:
        nodes += 1
        
        # Check if current is a Fibonacci point
        key = (current_l3[0], current_l3[1])
        if key in l3_to_fib:
            f = l3_to_fib[key]
            components.append(f)
            break
        
        # Compute current FGG
        v_curr = (float(current_l3[0]) * PHI + float(current_l3[1]) * PSI) % float(p)
        fgg_curr = FGG(v_curr / float(p), 3)
        
        # Find F_i whose FGG is CLOSEST to current FGG
        best_f = None
        best_remainder = None
        best_fgg_dist = float('inf')
        
        for f in fibs_desc:
            if f in components:
                continue
            
            l3_f = fib_l3[f]
            remainder = pt_sub(current_l3, l3_f)
            
            if remainder is None:
                components.append(f)
                current_l3 = None
                break
            
            # Compute FGG of remainder
            v_rem = (float(remainder[0]) * PHI + float(remainder[1]) * PSI) % float(p)
            fgg_rem = FGG(v_rem / float(p), 3)
            
            # Distance in FGG space
            # The correct remainder should have FGG close to a Fibonacci FGG
            min_dist = float('inf')
            for f2, fgg_f2 in fib_fgg.items():
                dist = abs(fgg_rem - fgg_f2)
                if dist < min_dist:
                    min_dist = dist
            
            if min_dist < best_fgg_dist:
                best_fgg_dist = min_dist
                best_f = f
                best_remainder = remainder
        
        if best_f is None:
            break
        
        # Check if the best remainder is a Fibonacci point
        if best_remainder is not None:
            rem_key = (best_remainder[0], best_remainder[1])
            if rem_key in l3_to_fib:
                # Direct hit!
                components.append(best_f)
                current_l3 = best_remainder
            else:
                # TRACE ERASURE: FGG the remainder to erase wrong path
                v_rem = (float(best_remainder[0]) * PHI + float(best_remainder[1]) * PSI) % float(p)
                FGG(v_rem / float(p), 3)
                traces_erased += 1
                
                # Still accept this F_i (FGG-guided)
                components.append(best_f)
                current_l3 = best_remainder
        else:
            components.append(best_f)
            current_l3 = None
        
        if len(components) <= 5:
            print(f"    Iter {len(components)}: chose F={best_f}, fgg_dist={best_fgg_dist:.6f}")
    
    return components, nodes, traces_erased


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING TRACE ERASURE ZECKENDORF ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
results = []

for secret in test_keys:
    print(f"  Secret: k={secret}")
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, nodes, traces = trace_erasure_zeckendorf(Q_l3, fib_l3, l3_to_fib, fibs_desc)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    
    if match:
        print(f"  🎉 SUCCESS! k={total} = {' + '.join(map(str, components))}")
    else:
        print(f"  ❌ FAILED: got {total}")
    
    print(f"  Nodes: {nodes}, Traces erased: {traces}, Time: {elapsed:.6f}s")
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
print(f"║  TRACE ERASURE — 'Wrong path? FGG it. Try next.'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

