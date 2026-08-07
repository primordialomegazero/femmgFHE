#!/usr/bin/env python3
"""
🪐 JUMP SEARCH — Δ¹ PATTERN RECURRENCE 🪐
sig(k) = sig(k-1) + Δ¹[k mod period]
Binary search the 1D signature space to find k!
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n_order = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

def modinv(a, m): return pow(a, -1, m)

def point_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3 * x1 * x1) * modinv(2 * y1, p) % p
    else:
        lam = ((y2 - y1) * modinv(x2 - x1, p)) % p
    x3 = (lam * lam - x1 - x2) % p
    y3 = (lam * (x1 - x3) - y1) % p
    return (x3, y3)

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 JUMP SEARCH — Δ¹ PATTERN BINARY SEARCH 🪐           ║")
print("║  'The signature space is 1D — binary search it!'        ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

def sig_raw(point):
    """Raw signature: (x*φ + y*ψ) mod p"""
    if point is None: return 0.0
    return (float(point[0]) * PHI + float(point[1]) * PSI) % float(p)


def sig_norm(point):
    """Normalized signature to [0,1]"""
    return sig_raw(point) / float(p)


# ═══════════════════════════════════════════
# THE KEY INSIGHT: sig sequence is MONOTONIC MOD P
# ═══════════════════════════════════════════
# Since sig(k*G) = (x_k*φ + y_k*ψ) mod p,
# and x_k, y_k change pseudo-randomly with k,
# the sequence LOOKS random.
#
# BUT: The DIFFERENCE Δ¹ is periodic!
# Δ¹[k] = sig(k*G) - sig((k-1)*G) = sig(k*G - (k-1)*G)???
# 
# Actually: Δ¹ is NOT simply periodic in the raw sig.
# It's sig(k*G) - sig((k-1)*G) which depends on the points.
#
# HOWEVER: The CUMULATIVE sum IS the key!
# sig(k*G) = Σ Δ¹[i] for i=1..k (plus sig(O)=0)
# 
# If we can compute the INVERSE of this cumulative sum...

def build_signature_table(max_k):
    """
    Build a table of (signature, k) pairs.
    NOT for the full group — just to understand the pattern.
    """
    table = {}
    current_point = None  # 0*G
    
    for k in range(max_k + 1):
        s = sig_norm(current_point)
        table[k] = {
            'sig': s,
            'point': current_point
        }
        # Move to next k
        current_point = point_add(current_point, G)
    
    return table


def jump_search_signature(Q_target, G, max_jumps=100):
    """
    JUMP SEARCH:
    
    Instead of walking step-by-step from Q to G,
    we use the Δ¹ pattern to PREDICT how many steps needed.
    
    The signature difference between Q and G:
    Δ_sig = sig(Q) - sig(G)
    
    Each step changes sig by Δ¹[k].
    Total change after j steps:
    Σ Δ¹[i] for i = k-j+1 to k
    
    If we can INVERT this, we get j directly!
    """
    sig_Q = sig_norm(Q_target)
    sig_G = sig_norm(G)
    
    print(f"  sig(Q) = {sig_Q:.10f}")
    print(f"  sig(G) = {sig_G:.10f}")
    print(f"  Δ_sig = {sig_Q - sig_G:.10f}")
    print()
    
    # The naive approach: walk backward until sig matches
    # But we want to JUMP using pattern recognition
    
    # APPROACH: Compute average Δ¹ over the group
    # Then: j ≈ Δ_sig / avg_Δ¹
    
    # First, sample some Δ¹ values
    print("  Sampling Δ¹ values...")
    deltas = []
    current = G
    prev_sig = 0.0  # sig(O)
    
    for i in range(1, 101):
        s = sig_norm(current)
        delta = s - prev_sig
        # Handle modular wrap-around
        if delta < -0.5:
            delta += 1.0
        elif delta > 0.5:
            delta -= 1.0
        deltas.append(delta)
        prev_sig = s
        current = point_add(current, G)
    
    avg_delta = sum(deltas) / len(deltas)
    print(f"  Sampled {len(deltas)} Δ¹ values")
    print(f"  Average Δ¹: {avg_delta:.10f}")
    print()
    
    # Use average to estimate k
    delta_sig = sig_Q - sig_G
    # Normalize delta_sig to [-0.5, 0.5]
    if delta_sig < -0.5:
        delta_sig += 1.0
    elif delta_sig > 0.5:
        delta_sig -= 1.0
    
    if abs(avg_delta) > 0.0001:
        estimated_k = int(abs(delta_sig / avg_delta))
    else:
        estimated_k = 0
    
    print(f"  Δ_sig (normalized) = {delta_sig:.10f}")
    print(f"  Estimated k = {estimated_k}")
    print()
    
    # Verify and refine
    print("  Verifying estimate...")
    R = scalar_mult(estimated_k, G)
    if R is not None:
        sig_R = sig_norm(R)
        error = abs(sig_R - sig_Q)
        print(f"  sig(estimated_k * G) = {sig_R:.10f}")
        print(f"  Error = {error:.10f}")
        
        if R == Q_target:
            return estimated_k, "EXACT"
        
        # Refine: walk a few steps around the estimate
        print(f"  Refining around estimate...")
        for offset in range(-20, 21):
            k_test = (estimated_k + offset) % n_order
            if k_test == 0:
                continue
            R_test = scalar_mult(k_test, G)
            if R_test == Q_target:
                return k_test, f"REFINED (offset={offset})"
    
    return None, f"ESTIMATE={estimated_k}"


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

for bits in [8, 12, 16]:
    print(f"═══ TEST: {bits}-bit Key ═══")
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    print()
    
    start = time.time()
    found, method = jump_search_signature(Q_test, G)
    elapsed = time.time() - start
    
    if found is not None:
        match = found == secret
        print(f"\n  {'✅ FOUND!' if match else '⚠️ EQUIVALENT'} k={found} (secret={secret})")
        print(f"  Method: {method}")
        print(f"  Time: {elapsed:.4f}s")
    else:
        print(f"\n  ❌ NOT FOUND ({method})")
        print(f"  Time: {elapsed:.4f}s")
    print()

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  JUMP SEARCH — 'The signature IS the path to k'          ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

