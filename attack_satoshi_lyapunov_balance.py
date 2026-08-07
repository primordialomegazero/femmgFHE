#!/usr/bin/env python3
"""
🪐 LYAPUNOV-STABILIZED DECOMPOSITION 🪐
'Lyapunov function V(x) = VOID(x) — must decrease monotonically!'
Stable: V(remainder) < V(current) → ACCEPT
Unstable: V(remainder) >= V(current) → REJECT + COLLAPSE
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0: current = abs((current * PHI) * PSI)
        else: current = abs((current * PSI) * PHI)
    return current

def VOID(v): return FGG(v, 3)

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

# Lyapunov function
def lyapunov_V(point):
    """V(x) = VOID(x) — must decrease toward equilibrium"""
    if point is None: return 0.0
    v = (float(point[0]) * PHI + float(point[1]) * PSI) % float(p)
    return VOID(v / float(p))

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 LYAPUNOV-STABILIZED DECOMPOSITION 🪐                 ║")
print("║  'Stable: V↓ → ACCEPT | Unstable: V↑ → REJECT+COLLAPSE'  ║")
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
# LYAPUNOV-STABILIZED DECOMPOSITION
# ═══════════════════════════════════════════
def lyapunov_decompose(Q_l3, max_depth=50):
    """
    LYAPUNOV-STABILIZED DECOMPOSITION:
    
    V(x) = VOID(x) — Lyapunov function
    Equilibrium: V(x) = 0 (identity)
    
    Sa bawat hakbang:
    1. Kunin ang current Lyapunov value V_current
    2. Subukan ang bawat F_a:
       a. remainder = current - l3(F_a * G)
       b. V_remainder = VOID(remainder)
       c. Kung V_remainder < V_current → STABLE → ACCEPT
       d. Kung V_remainder >= V_current → UNSTABLE → REJECT + COLLAPSE
    3. Kung walang stable F_a → BOOTSTRAP current → subukan muli
    """
    current = Q_l3
    components = []
    used_indices = set()
    
    V_current = lyapunov_V(current)
    lyapunov_history = [V_current]
    bootstraps = 0
    collapses = 0
    
    depth = 0
    while current is not None and depth < max_depth:
        depth += 1
        
        # Check exact match (equilibrium!)
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
        
        # LYAPUNOV SEARCH: Hanapin ang stable F_a
        best_f = None
        best_remainder = None
        best_V = V_current  # Dapat mas mababa!
        found_stable = False
        
        for fa in fibs_desc:
            fa_idx = fib_to_idx[fa]
            if fa_idx in used_indices or fa_idx - 1 in used_indices:
                continue
            
            remainder = pt_sub(current, fib_l3[fa])
            
            if remainder is None:
                # Identity — perfect stability!
                best_f = fa
                best_remainder = None
                best_V = 0.0
                found_stable = True
                break
            
            # LYAPUNOV CHECK: V(remainder) < V(current)?
            V_remainder = lyapunov_V(remainder)
            
            if V_remainder < V_current:
                # STABLE! Papunta sa equilibrium!
                if V_remainder < best_V:
                    best_V = V_remainder
                    best_f = fa
                    best_remainder = remainder
                    found_stable = True
        
        if found_stable and best_f is not None:
            # ACCEPT!
            components.append(best_f)
            used_indices.add(fib_to_idx[best_f])
            used_indices.add(fib_to_idx[best_f] + 1)
            current = best_remainder
            V_current = best_V
            lyapunov_history.append(V_current)
            
            if depth <= 10 or best_V < 0.001:
                print(f"    Depth {depth}: F={best_f} V={V_current:.6f} ↓ (stable) ✅")
        else:
            # UNSTABLE: Walang stable F_a — BOOTSTRAP!
            if depth <= 5:
                print(f"    Depth {depth}: No stable F_a found — BOOTSTRAPPING...")
            
            # Bootstrap: I-FGG ang current para ma-refresh
            v = (float(current[0]) * PHI + float(current[1]) * PSI) % float(p)
            FGG(v / float(p), 3)  # Collapse!
            collapses += 1
            bootstraps += 1
            
            # Recompute V after bootstrap
            V_new = lyapunov_V(current)
            
            if V_new < V_current:
                V_current = V_new
                lyapunov_history.append(V_current)
            else:
                # Kahit bootstrap hindi nakatulong — STOP!
                if depth <= 5:
                    print(f"    Depth {depth}: Bootstrap didn't help — STOPPING")
                break
    
    return components, lyapunov_history, bootstraps, collapses, depth


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING LYAPUNOV-STABILIZED ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, history, bootstraps, collapses, depth = lyapunov_decompose(Q_l3, max_depth=50)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    V trajectory: {[f'{v:.4f}' for v in history[:5]]}...")
    print(f"    Depth={depth}, Bootstraps={bootstraps}, Collapses={collapses}, Time={elapsed:.4f}s")
    print()

# ═══════════════════════════════════════════
# SATOSHI LYAPUNOV
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI LYAPUNOV-STABILIZED 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Running Lyapunov-stabilized decomposition...")
print("  'V(x) must decrease — or COLLAPSE!'")
print()

start = time.time()
components, history, bootstraps, collapses, depth = lyapunov_decompose(Q_l3, max_depth=50)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 LYAPUNOV-STABILIZED COMPLETE!")
    print(f"  Components: {len(components)}")
    print(f"  V trajectory: {[f'{v:.4f}' for v in history[:10]]}...")
    print(f"  Bootstraps: {bootstraps}, Collapses: {collapses}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Lyapunov-Stabilized Decomposition")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Lyapunov-Stabilized\n")
            f.write(f"method: Lyapunov-Stabilized Decomposition\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"lyapunov_trajectory: {history}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ Decomposition failed — depth={depth}, {elapsed:.2f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  LYAPUNOV BALANCE — 'Stability guides, collapse corrects' ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

