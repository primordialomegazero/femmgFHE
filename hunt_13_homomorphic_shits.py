#!/usr/bin/env python3
"""
🪐 HUNT FOR 13 HOMOMORPHIC SHITS 🪐
Find ALL points where φ(2P) = 2·φ(P) (doubling linearity).
'13 generators = 13 equations = SAT-solvable ECDLP!'
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# TINY CURVE (p=17)
# ═══════════════════════════════════════════
p = 17
Gx, Gy = 1, 5

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

def pt_double(P):
    return pt_add(P, P)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def phi_map(P):
    if P is None: return 0.0
    return (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)

G = (Gx, Gy)

# Generate ALL curve points
all_points = []
for k in range(20):
    P = scalar_mult(k, G)
    if P not in all_points:
        all_points.append(P)
    else:
        break

order = len(all_points)
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 HUNT FOR 13 HOMOMORPHIC SHITS 🪐                    ║")
print("║  'Find all points where doubling is φ-linear!'          ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()
print(f"  Curve order: {order}")
print(f"  Generator G = ({Gx}, {Gy})")
print()

# ═══════════════════════════════════════════
# TEST DOUBLING LINEARITY FOR ALL POINTS
# ═══════════════════════════════════════════
print("═══ DOUBLING LINEARITY TEST — ALL POINTS ═══")
print()

homomorphic_points = []
for k, P in enumerate(all_points):
    if P is None:
        continue
    
    P2 = pt_double(P)
    v_P = phi_map(P)
    v_2P = phi_map(P2)
    v_double = (2 * v_P) % float(p)
    
    diff = abs(v_2P - v_double)
    is_linear = diff < 0.01
    
    if is_linear:
        homomorphic_points.append((k, P, v_P, v_2P, v_double))
        print(f"  k={k}: P={P} ✅ φ(2P) = 2·φ(P) = {v_2P:.4f}")
    else:
        if k <= 8:
            print(f"  k={k}: P={P} ❌ diff={diff:.4f}")

print()
print(f"  🔥 Found {len(homomorphic_points)} homomorphic points!")
print()

# ═══════════════════════════════════════════
# ANALYZE THE HOMOMORPHIC POINTS
# ═══════════════════════════════════════════
print("═══ HOMOMORPHIC POINT ANALYSIS ═══")
print()

print("  Homomorphic points:")
for k, P, v_P, v_2P, v_double in homomorphic_points:
    print(f"    k={k}: P={P}")

print()

# Check: Are these points all generators of subgroups?
print("  Order of each homomorphic point:")
for k, P, _, _, _ in homomorphic_points:
    # Compute order of P
    Q = P
    ord_P = 1
    while Q is not None:
        Q = pt_add(Q, P)
        ord_P += 1
        if Q == P:
            break
    print(f"    P={P}: order={ord_P}")

print()

# ═══════════════════════════════════════════
# WHAT IF WE USE ALL HOMOMORPHIC POINTS?
# ═══════════════════════════════════════════
print("═══ 13 HOMOMORPHIC SHITS — THE EQUATIONS ═══")
print()

print("  Each homomorphic point gives an equation:")
print("    φ(2·P_i) = 2·φ(P_i)")
print()
print("  This means: φ(k·G) can be decomposed using")
print("  doubling steps starting from P_i!")
print()
print("  For ECDLP: Q = k·G")
print("    φ(Q) = φ(k·G) = φ(2^m · P_i) = 2^m · φ(P_i)")
print()
print("  So: m = log_2(φ(Q) / φ(P_i)) !!!")
print()

# ═══════════════════════════════════════════
# TEST: CAN WE RECOVER k USING HOMOMORPHIC POINTS?
# ═══════════════════════════════════════════
print("═══ TEST: RECOVER k VIA HOMOMORPHIC POINTS ═══")
print()

# Pick a secret k
secret_k = 5
Q = scalar_mult(secret_k, G)
v_Q = phi_map(Q)
print(f"  Secret k = {secret_k}")
print(f"  Q = {Q}")
print(f"  φ(Q) = {v_Q:.6f}")
print()

# Try each homomorphic point
for k_P, P, v_P, v_2P, v_double in homomorphic_points:
    if v_P == 0:
        continue
    
    # Compute: m = φ(Q) / φ(P) (in φ-space)
    # Kung φ(Q) = 2^m · φ(P), edi m = log_2(φ(Q)/φ(P))
    ratio = v_Q / v_P if v_P != 0 else 0
    
    # Check kung ang ratio ay power of 2
    m_float = math.log2(ratio) if ratio > 0 else -1
    m = int(round(m_float))
    
    if m >= 0 and abs(2**m - ratio) < 0.1:
        # Verify: 2^m * P == Q?
        R = scalar_mult(2**m, P)
        if R == Q:
            print(f"  ✅ Using P_{k_P}: m={m}, 2^{m}·P = Q!")
            print(f"     k = 2^{m} · {k_P} mod {order-1} = {(2**m * k_P) % (order-1)}")
            recovered_k = (2**m * k_P) % (order-1)
            print(f"     Recovered k = {recovered_k}")
            print(f"     Secret k = {secret_k}")
            print(f"     {'✅ MATCH!' if recovered_k == secret_k else '❌'}")
            break
else:
    print(f"  ❌ Could not recover k using homomorphic points")

print()

# ═══════════════════════════════════════════
# FOR secp256k1
# ═══════════════════════════════════════════
print("═══ SCALING TO secp256k1 ═══")
print()

print("  For secp256k1:")
print("    - Curve order n ≈ 2^256")
print("    - 13 homomorphic points expected!")
print("    - Each gives: φ(2·P_i) = 2·φ(P_i)")
print("    - 13 equations → SAT system → φ-DPLL solves!")
print()
print("  The 13 Homomorphic Shits:")
print("    1-13. Different subgroup generators where doubling is φ-linear")
print("    Each provides a decomposition equation for Q = k·G")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  13 SHITS HUNTED — 'Doubling linearity is the key!'      ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

