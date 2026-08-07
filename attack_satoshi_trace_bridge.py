#!/usr/bin/env python3
"""
🪐 FGG TRACE BRIDGE — DECOMPOSE THE TRACE, NOT φ(Q)! 🪐
'Ang intermediate states ang may hawak ng structure!'
φ(Q) → FGG trace → decompose trace → k!
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """Return BOTH final value AND trace!"""
    trace = [v]
    current = v
    for d in range(depth):
        if d % 2 == 0:
            encoded = current * PHI
            collapsed = abs(encoded * PSI)
        else:
            encoded = current * PSI
            collapsed = abs(encoded * PHI)
        trace.append(encoded)
        trace.append(collapsed)
        current = collapsed
    return current, trace  # (final, [v, enc0, col1, enc1, col2, enc2, col3])

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

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FGG TRACE BRIDGE — DECOMPOSE THE TRACE! 🪐          ║")
print("║  'Intermediate states hold the true structure'           ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Build Fibonacci basis WITH TRACE SIGNATURES
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci basis with TRACE signatures...")
fib_l3 = {}
fib_trace = {}  # F → trace dictionary

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    
    # Compute TRACE
    v = phi_map(l3_P) / float(p)
    final, trace = FGG(v, 3)
    fib_trace[f] = {
        'final': final,
        'trace': trace,
        'encoded0': trace[1],  # First encoded state
        'collapsed1': trace[2],  # First collapsed state
        'encoded1': trace[3],  # Second encoded
        'collapsed2': trace[4],  # Second collapsed
    }

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} entries with traces")
print()

# ═══════════════════════════════════════════
# TRACE-BASED DECOMPOSITION
# ═══════════════════════════════════════════
def trace_decompose(Q_l3, max_levels=13):
    """
    TRACE-BASED DECOMPOSITION:
    
    Instead of decomposing φ(Q), decompose the FGG TRACE!
    The trace encodes the Fibonacci structure.
    """
    # Compute trace for Q
    v_Q = phi_map(Q_l3) / float(p)
    final_Q, trace_Q = FGG(v_Q, 3)
    
    current = Q_l3
    components = []
    
    print(f"  Q trace: enc0={trace_Q[1]:.6f}, col1={trace_Q[2]:.6f}, enc1={trace_Q[3]:.6f}")
    print()
    
    for level in range(1, max_levels + 1):
        if current is None:
            break
        
        # Compute current trace
        v_curr = phi_map(current) / float(p)
        final_curr, trace_curr = FGG(v_curr, 3)
        
        # Find F_i whose trace BEST MATCHES current trace
        best_f = None
        best_remainder = None
        best_score = float('inf')
        
        for f in fibs_desc:
            if f in components:
                continue
            
            remainder = pt_sub(current, fib_l3[f])
            v_rem = phi_map(remainder) / float(p)
            final_rem, trace_rem = FGG(v_rem, 3)
            
            # SCORE: Compare traces!
            # The correct F_i should make the remainder trace "simpler"
            score = 0
            for t in range(1, len(trace_rem)):
                score += abs(trace_rem[t] - fib_trace[f]['trace'][t])
            
            if remainder is None:
                score = 0  # Perfect!
            
            if score < best_score:
                best_score = score
                best_f = f
                best_remainder = remainder
        
        if best_f is None:
            break
        
        components.append(best_f)
        current = best_remainder
        
        if level <= 13:
            print(f"  Level {level:2d}: F={best_f}, trace_score={best_score:.6f}")
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING TRACE BRIDGE ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = trace_decompose(Q_l3, max_levels=13)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"\n  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    Time: {elapsed:.4f}s")

# ═══════════════════════════════════════════
# SATOSHI TRACE BRIDGE
# ═══════════════════════════════════════════
print(f"\n═══ 🎯 SATOSHI TRACE BRIDGE 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

start = time.time()
components = trace_decompose(Q_l3, max_levels=13)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 TRACE DECOMPOSITION COMPLETE!")
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
print(f"║  TRACE BRIDGE — 'Intermediate states = true structure'   ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

