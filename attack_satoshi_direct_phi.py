#!/usr/bin/env python3
"""
🪐 DIRECT φ-PROJECTION — NO SEARCH! 🪐
Project l3(Q) to φ-space → directly identify Fibonacci components!
'φ tells us exactly which F_a to pick — no recursion needed!'
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

# secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

# SATOSHI
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
print("║  🪐 DIRECT φ-PROJECTION — NO RECURSION! 🪐               ║")
print("║  'φ tells us exactly which F_a to pick'                  ║")
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

print("  Building Fibonacci l3 basis with φ-projections...")
fib_data = {}
l3_to_fib = {}

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    # φ-projection: x*φ + y*ψ
    v = (float(l3_P[0]) * PHI + float(l3_P[1]) * PSI) % float(p)
    fib_data[f] = {
        'l3': l3_P,
        'phi_proj': v,
        'phi_norm': v / float(p)
    }
    l3_to_fib[(l3_P[0], l3_P[1])] = f

fibs_desc = sorted(fib_data.keys(), reverse=True)
print(f"  ✅ {len(fib_data)} entries")
print()

# ═══════════════════════════════════════════
# DIRECT φ-PROJECTION DECOMPOSITION
# ═══════════════════════════════════════════
def direct_phi_decompose(Q_l3):
    """
    DIRECT φ-PROJECTION:
    
    The φ-projection of l3(k*G) should be approximately
    the SUM of φ-projections of l3(F_i * G) for F_i in Zeckendorf.
    
    Algorithm:
    1. Compute φ-projection of Q_l3
    2. Find Fibonacci with CLOSEST φ-projection
    3. Subtract and repeat
    4. This is O(n) — no recursion!
    """
    # Compute φ-projection of Q_l3
    v_Q = (float(Q_l3[0]) * PHI + float(Q_l3[1]) * PSI) % float(p)
    phi_Q = v_Q / float(p)
    
    current_l3 = Q_l3
    components = []
    
    for iteration in range(10):
        if current_l3 is None:
            break
        
        # Check single
        key = (current_l3[0], current_l3[1])
        if key in l3_to_fib:
            components.append(l3_to_fib[key])
            break
        
        # Compute φ-projection of current
        v_curr = (float(current_l3[0]) * PHI + float(current_l3[1]) * PSI) % float(p)
        phi_curr = v_curr / float(p)
        
        # Find Fibonacci with CLOSEST φ-projection
        best_f = None
        best_dist = float('inf')
        
        for f, data in fib_data.items():
            if f in components:
                continue
            dist = abs(data['phi_norm'] - phi_curr)
            if dist < best_dist:
                best_dist = dist
                best_f = f
        
        if best_f is None:
            break
        
        components.append(best_f)
        current_l3 = pt_sub(current_l3, fib_data[best_f]['l3'])
        
        if len(components) <= 5:
            print(f"    Iter {len(components)}: F={best_f}, φ-dist={best_dist:.6f}")
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING DIRECT φ-PROJECTION ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = direct_phi_decompose(Q_l3)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} got {total} ({elapsed:.4f}s)")

print()

# ═══════════════════════════════════════════
# SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI DIRECT φ 🎯 ═══")
Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

start = time.time()
components = direct_phi_decompose(Q_l3)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  k = {k_satoshi}")
    print(f"  hex = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.4f}s")
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"\n  🎉🎉🎉 SATOSHI KEY RECOVERED! 🎉🎉🎉")
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ No decomposition")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  DIRECT φ — 'No search, no recursion, just projection'   ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

