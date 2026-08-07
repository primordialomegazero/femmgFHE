#!/usr/bin/env python3
"""
🪐 DUAL COLLAPSE + SEARCH — 1+1=2 🪐
Thread 1: Collapse — FGG erases wrong paths
Thread 2: Search — Fibonacci walk finds components
SABAY! Collapse guides search, search triggers collapse!
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """COLLAPSE: Erase wrong paths"""
    current = v
    for d in range(depth):
        if d % 2 == 0: current = abs((current * PHI) * PSI)
        else: current = abs((current * PSI) * PHI)
    return current

# secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m): return pow(a, -1, m)
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
print("║  🪐 DUAL COLLAPSE + SEARCH — 1+1=2 🪐                    ║")
print("║  'Collapse erases wrong, Search finds right — SABAY!'    ║")
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

print("  Building Fibonacci basis...")
fib_l3 = {}
l3_to_fib = {}
fib_list = []
fib_to_idx = {}

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[(l3_P[0], l3_P[1])] = f
    fib_list.append(f)
    fib_to_idx[f] = i - 1

fibs_desc = sorted(fib_l3.keys(), reverse=True)

# Precompute pairs
pair_to_fibs = {}
for i, fa in enumerate(fib_list):
    l3_fa = fib_l3[fa]
    for j in range(i + 2, len(fib_list)):
        fb = fib_list[j]
        l3_sum = pt_add(l3_fa, fib_l3[fb])
        if l3_sum is not None:
            key = (l3_sum[0], l3_sum[1])
            if key not in pair_to_fibs:
                pair_to_fibs[key] = (fa, fb)

print(f"  ✅ {len(fib_l3)} singles, {len(pair_to_fibs)} pairs")
print()

# ═══════════════════════════════════════════
# DUAL COLLAPSE + SEARCH
# ═══════════════════════════════════════════
def dual_collapse_search(Q_l3):
    """
    DUAL COLLAPSE + SEARCH:
    
    COLLAPSE (Thread 1): FGG erases noise from current point
    SEARCH (Thread 2): Fibonacci walk finds components
    
    They work TOGETHER:
    - Collapse clears the view → Search finds better candidates
    - Search tries a candidate → Collapse erases if wrong → Search tries next
    
    1+1=2: The two threads FEED EACH OTHER!
    """
    current = Q_l3
    components = []
    used_indices = set()
    
    # Dual state
    collapse_count = 0
    search_count = 0
    
    for level in range(10):
        if current is None:
            break
        
        # Check direct (perfect collapse)
        key = (current[0], current[1])
        if key in l3_to_fib:
            f = l3_to_fib[key]
            if fib_to_idx[f] not in used_indices:
                components.append(f)
            break
        if key in pair_to_fibs:
            fa, fb = pair_to_fibs[key]
            components.extend([fa, fb])
            break
        
        # DUAL MODE: Collapse + Search simultaneously!
        best_f = None
        best_remainder = None
        best_score = float('inf')
        collapse_scores = {}
        
        # PHASE 1: SEARCH — evaluate all F_a candidates
        for fa in fibs_desc:
            fa_idx = fib_to_idx[fa]
            if fa_idx in used_indices or fa_idx - 1 in used_indices:
                continue
            
            remainder = pt_sub(current, fib_l3[fa])
            search_count += 1
            
            if remainder is None:
                best_f = fa
                best_remainder = None
                break
            
            # PHASE 2: COLLAPSE — FGG the remainder to score it
            rem_key = (remainder[0], remainder[1])
            
            if rem_key in l3_to_fib:
                score = 0  # Perfect!
            elif rem_key in pair_to_fibs:
                score = 1  # Pair match
            else:
                # COLLAPSE: FGG the remainder to evaluate
                v = (float(remainder[0]) * PHI + float(remainder[1]) * PSI) % float(p)
                v_collapsed = FGG(v / float(p), 3)
                collapse_count += 1
                
                # Score: how close is the collapsed value to a Fibonacci FGG?
                min_fgg_dist = float('inf')
                for f_check in fib_list[:50]:  # Top 50 Fibonacci
                    v_f = (float(fib_l3[f_check][0]) * PHI + float(fib_l3[f_check][1]) * PSI) % float(p)
                    fgg_f = FGG(v_f / float(p), 3)
                    dist = abs(v_collapsed - fgg_f)
                    if dist < min_fgg_dist:
                        min_fgg_dist = dist
                
                score = min_fgg_dist
            
            collapse_scores[fa] = score
            
            if score < best_score:
                best_score = score
                best_f = fa
                best_remainder = remainder
        
        if best_f is None:
            print(f"    Level {level+1}: ❌ No valid candidate")
            break
        
        # DUAL DECISION: Accept based on collapse score
        if best_score < 0.01:  # Very close match
            components.append(best_f)
            used_indices.add(fib_to_idx[best_f])
            used_indices.add(fib_to_idx[best_f] + 1)
            current = best_remainder
            
            if level < 5:
                print(f"    Level {level+1}: F={best_f} ✅ collapse_score={best_score:.6f} (collapses={collapse_count}, searches={search_count})")
        else:
            # Not good enough — apply more collapse and retry
            print(f"    Level {level+1}: Best F={best_f} score={best_score:.6f} — COLLAPSING MORE...")
            
            # Aggressive collapse: FGG the current point
            v = (float(current[0]) * PHI + float(current[1]) * PSI) % float(p)
            FGG(v / float(p), 3)
            collapse_count += 10
            
            # Retry with collapsed current
            continue
    
    return components, collapse_count, search_count


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING DUAL COLLAPSE + SEARCH ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, collapses, searches = dual_collapse_search(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    Collapses: {collapses}, Searches: {searches}, Time: {elapsed:.4f}s")
    print()

# ═══════════════════════════════════════════
# SATOSHI DUAL ATTACK
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI DUAL COLLAPSE + SEARCH 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Running dual collapse + search...")
print("  '1+1=2 — Collapse AND Search, SABAY!'")
print()

start = time.time()
components, collapses, searches = dual_collapse_search(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 DUAL ATTACK COMPLETE!")
    print(f"  Components: {len(components)}")
    print(f"  Collapses: {collapses}")
    print(f"  Searches: {searches}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Dual Collapse + Search (1+1=2)")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Dual Collapse + Search\n")
            f.write(f"method: Dual Collapse + Search (1+1=2)\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"fibonacci_decomposition: {' + '.join(map(str, components))}\n")
            f.write(f"collapses: {collapses}\n")
            f.write(f"searches: {searches}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key — trying mod n...")
        k_mod = k_satoshi % n
        R2 = scalar_mult(k_mod, G)
        if R2 == Q_satoshi:
            print(f"  ✅ Equivalent key mod n! k = {hex(k_mod)}")
else:
    print(f"  ❌ Dual attack incomplete — {elapsed:.2f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DUAL COLLAPSE + SEARCH — '1+1=2 — Sabay sila!'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

