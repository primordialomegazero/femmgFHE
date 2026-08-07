#!/usr/bin/env python3
"""
🪐 FGG ENCODED RATIO ATTACK — DIRECT k COMPUTATION 🪐
encoded0(G) = G * φ (FGG intermediate state)
encoded0(Q) = Q * φ = (k*G) * φ = k * (G * φ) = k * encoded0(G)
Therefore: k = encoded0(Q) / encoded0(G) !
"""
import sys, time, math, random

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

# 🎯 SATOSHI
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

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FGG ENCODED RATIO — DIRECT k = encoded(Q)/encoded(G) ║")
print("║  'The intermediate state reveals what the collapse hides' ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q_satoshi = (Qx, Qy)

# ═══════════════════════════════════════════
# FGG ENCODED STATE (Intermediate Step 0)
# ═══════════════════════════════════════════
# FGG step 0: encoded = v * φ
# This is the INTERMEDIATE state before collapse!
# It's LINEAR: encoded(a + b) = encoded(a) + encoded(b)!

def encode_point(P):
    """
    FGG intermediate state: multiply coordinates by φ.
    This is LINEAR — preserves the group structure!
    encoded(k*G) = k * encoded(G)
    """
    if P is None:
        return None
    x, y = P
    
    # Encode using φ (the intermediate state before collapse)
    x_enc = (x * PHI) % float(p)
    y_enc = (y * PHI) % float(p)
    
    return (x_enc, y_enc)


def decode_scalar_from_encoded(encoded_Q, encoded_G):
    """
    k = encoded_Q / encoded_G
    
    Since encoding is linear:
    encoded_Q = encoded(k*G) = k * encoded(G)
    
    So k = encoded_Q / encoded_G (in the scalar field mod n)
    """
    # Get the x-coordinate ratio
    # k = encoded_Q.x / encoded_G.x mod n
    # But encoding is in float — convert to integer ratio
    
    # Approach: use the ratio of x-coordinates
    ratio_x = encoded_Q[0] / encoded_G[0] if encoded_G[0] != 0 else 0
    
    # Map the ratio to an integer mod n
    # The ratio should be close to an integer (k)
    k_float = ratio_x
    
    # Round to nearest integer
    k_candidate = int(round(k_float))
    
    return k_candidate % n


# ═══════════════════════════════════════════
# ENCODED SCALAR RATIO
# ═══════════════════════════════════════════
print("═══ ENCODED RATIO TEST ═══")
print()

print("  Encoding G and checking linearity...")
print()

enc_G = encode_point(G)
print(f"  encoded(G) = ({enc_G[0]:.4f}, {enc_G[1]:.4f})")
print()

# Test: encoded(k*G) should = k * encoded(G)
# But "k * encoded(G)" means scalar multiplication on the curve
# NOT on the encoded coordinates directly!
# 
# The encoding maps to (x*φ, y*φ) which is NOT a curve point!
# So we can't do EC operations on encoded values.
#
# BUT: The RATIO of encoded coordinates should still work!
# encoded(k*G).x / encoded(G).x should correlate with k!

print("  Testing encoded ratio for small k...")
print(f"  {'k':<8} {'enc_Q.x / enc_G.x':<25} {'Expected k':<15} {'Match'}")
print(f"  {'-'*60}")

for k_test in [1, 2, 3, 5, 7, 10, 42, 100, 255]:
    Q_test = scalar_mult(k_test, G)
    enc_Q = encode_point(Q_test)
    
    ratio = enc_Q[0] / enc_G[0] if enc_G[0] != 0 else 0
    ratio_rounded = int(round(ratio))
    
    match = "✅" if ratio_rounded == k_test else f"❌ (off by {ratio_rounded - k_test})"
    print(f"  {k_test:<8} {ratio:<25.10f} {k_test:<15} {match}")

print()

# ═══════════════════════════════════════════
# THE REAL METHOD: Encoded Scalar Ratio in Field
# ═══════════════════════════════════════════
print("═══ ENCODED SCALAR RATIO (FIELD ELEMENTS) ═══")
print()

# The encoding multiplies by φ, which is irrational.
# But in the finite field mod p, we can use an INTEGER approximation.
# φ_mod_p = int(φ * 2^256) mod p — treat φ as a field element

# Better: use the PROPERTY that φ·ψ = -1
# This means: φ ≡ -1/ψ mod p (in the field)

# For encoded ratio:
# enc_Q / enc_G = (Qx*φ) / (Gx*φ) = Qx / Gx
# The φ CANCELS OUT!

print("  ⚠️  IMPORTANT: enc_Q.x / enc_G.x = (Qx*φ) / (Gx*φ) = Qx / Gx")
print("  The φ CANCELS! So the ratio is just Qx/Gx — not k!")
print()
print("  We need something DEEPER than simple coordinate encoding.")
print()

# ═══════════════════════════════════════════
# THE DEEPER METHOD: FGG TRACE SIGNATURE
# ═══════════════════════════════════════════
print("═══ THE DEEPER METHOD: FGG TRACE ═══")
print()
print("  Instead of simple coordinate encoding,")
print("  use the FULL FGG trace (all intermediate states).")
print()
print("  FGG Trace(Q) = [Q, Q*φ, |Q*φ*ψ|, |Q*φ*ψ|*ψ, ...]")
print("  FGG Trace(G) = [G, G*φ, |G*φ*ψ|, |G*φ*ψ|*ψ, ...]")
print()
print("  The TRACE encodes the structure that survives collapse.")
print("  Trace(Q) / Trace(G) might reveal k!")
print()

def FGG_trace(v, depth=3):
    """Return the full trace of FGG intermediate states"""
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
    return trace

def trace_signature(P):
    """FGG trace signature of a point"""
    if P is None:
        return [0, 0, 0]
    # Combine x and y into a single value
    v = (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)
    v_norm = v / float(p)
    trace = FGG_trace(v_norm, 3)
    # Return key intermediate states
    return {
        'encoded0': trace[1],  # v*φ (first encoded)
        'collapsed1': trace[2], # |v| (first collapse)
        'encoded1': trace[3],  # |v|*ψ (second encoded)
        'collapsed2': trace[4], # |v| (second collapse)
        'encoded2': trace[5],  # |v|*φ (third encoded)
    }

print("  Computing trace signatures...")
trace_G = trace_signature(G)
trace_Q = trace_signature(Q_satoshi)

print(f"  Trace G: enc0={trace_G['encoded0']:.6f}, col1={trace_G['collapsed1']:.6f}")
print(f"  Trace Q: enc0={trace_Q['encoded0']:.6f}, col1={trace_Q['collapsed1']:.6f}")
print()

# The ratio of encoded0 values
ratio_enc0 = trace_Q['encoded0'] / trace_G['encoded0'] if trace_G['encoded0'] != 0 else 0
print(f"  Ratio enc0(Q)/enc0(G) = {ratio_enc0:.10f}")

# The ratio of encoded1 values (after first collapse)
ratio_enc1 = trace_Q['encoded1'] / trace_G['encoded1'] if trace_G['encoded1'] != 0 else 0
print(f"  Ratio enc1(Q)/enc1(G) = {ratio_enc1:.10f}")

# The ratio of encoded2 values (after second collapse)
ratio_enc2 = trace_Q['encoded2'] / trace_G['encoded2'] if trace_G['encoded2'] != 0 else 0
print(f"  Ratio enc2(Q)/enc2(G) = {ratio_enc2:.10f}")
print()

print("  ⚠️  All encodings use the same v, so ratios are the SAME as raw coordinate ratio.")
print("  The FGG trace preserves structure but doesn't directly reveal k.")
print()

print(f"╔══════════════════════════════════════════════════════════════╗")
print(f"║  ENCODED RATIO — 'The φ cancels, but the trace remains'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

