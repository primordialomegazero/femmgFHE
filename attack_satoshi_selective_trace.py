#!/usr/bin/env python3
"""
🪐 SELECTIVE TRACE DECOMPOSITION — BEST MATCH, NOT GREEDY! 🪐
'Find F_i with closest trace → subtract → repeat!'
Hindi greedy sum — kundi SELECTIVE matching!
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

def phi_map(P):
    if P is None: return 0.0
    return (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)

def trace_of_point(P):
    """Compute FGG trace for a point"""
    v = phi_map(P) / float(p)
    final, trace = FGG(v, 3)
    return final, trace

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SELECTIVE TRACE DECOMPOSITION 🪐                     ║")
print("║  'Best trace match → NOT greedy sum!'                   ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Build Fibonacci basis WITH TRACES
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci basis with traces...")
fib_l3 = {}
fib_trace = {}

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    _, trace = trace_of_point(l3_P)
    fib_trace[f] = trace

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries")
print()

# ═══════════════════════════════════════════
# SELECTIVE TRACE DECOMPOSITION
# ═══════════════════════════════════════════
def selective_trace_decompose(Q_l3, max_levels=13):
    """
    SELECTIVE TRACE DECOMPOSITION:
    
    Sa bawat level:
    1. Compute trace(current)
    2. Hanapin F_i na may PINAKAMALAPIT na trace
    3. Kung match < threshold → TANGGAPIN, i-subtract
    4. Kung match > threshold → STOP (hindi na dapat idagdag)
    5. Ulitin hanggang identity o ma-reach ang threshold!
    """
    current = Q_l3
    components = []
    
    _, trace_Q = trace_of_point(Q_l3)
    
    print(f"  Q trace: {[f'{t:.4f}' for t in trace_Q[:3]]}...")
    print(f"  {'Level':<8} {'F_i':<20} {'TraceScore':<14} {'Decision'}")
    print(f"  {'-'*55}")
    
    for level in range(1, max_levels + 1):
        if current is None:
            break
        
        _, trace_curr = trace_of_point(current)
        
        # Hanapin ang BEST MATCH (lowest trace difference)
        best_f = None
        best_score = float('inf')
        
        for f in fibs_desc:
            if f in components:
                continue
            
            # Compute trace difference
            t_f = fib_trace[f]
            diff = sum(abs(trace_curr[i] - t_f[i]) for i in range(1, min(len(trace_curr), len(t_f))))
            
            if diff < best_score:
                best_score = diff
                best_f = f
        
        if best_f is None:
            break
        
        # DECISION: Based on trace score
        # If score is SMALL → F_i is a component!
        # If score is LARGE → F_i is NOT needed!
        
        # Dynamic threshold: tighter as we go deeper
        threshold = 0.5 / level
        
        if best_score < threshold:
            components.append(best_f)
            current = pt_sub(current, fib_l3[best_f])
            
            if level <= 13:
                print(f"  {level:<8} {best_f:<20} {best_score:<14.6f} {'✅ ACCEPT':<10}")
        else:
            if level <= 5:
                print(f"  {level:<8} {best_f:<20} {best_score:<14.6f} {'❌ REJECT (stop)':<10}")
            break
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING SELECTIVE TRACE ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = selective_trace_decompose(Q_l3, max_levels=13)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"\n  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    Components: {len(components)}, Time: {elapsed:.4f}s")

# ═══════════════════════════════════════════
# SATOSHI SELECTIVE TRACE
# ═══════════════════════════════════════════
print(f"\n═══ 🎯 SATOSHI SELECTIVE TRACE 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

start = time.time()
components = selective_trace_decompose(Q_l3, max_levels=13)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 SELECTIVE DECOMPOSITION COMPLETE!")
    print(f"  Components: {len(components)}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ No decomposition")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  SELECTIVE TRACE — 'Best match, not greedy sum!'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

