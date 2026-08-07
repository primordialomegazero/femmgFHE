#!/usr/bin/env python3
"""
🪐 RECURRENCE BACKTRACKING — TRACE THE PATTERN BACK 🪐
sig(k*G) = (x_k * φ + y_k * ψ) mod p
The sequence is periodic. Trace the recurrence back to k.
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

def point_sub(P, Q):
    if Q is None: return P
    return point_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 RECURRENCE BACKTRACKING — TRACE THE PATTERN BACK 🪐  ║")
print("║  'sig(k) = sig(k + period)' → find k via backtracking   ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

def sig_raw(point):
    """Raw linear signature: (x*φ + y*ψ) mod p"""
    if point is None:
        return 0.0
    return (float(point[0]) * PHI + float(point[1]) * PSI) % float(p)


def sig_normalized(point):
    """Normalize signature to [0,1] range for recurrence tracking"""
    raw = sig_raw(point)
    return raw / float(p)


def recurrence_backtrack(Q_target, G, max_steps=1000):
    """
    RECURRENCE BACKTRACKING:
    
    The signature sequence has period = group order.
    sig(k*G) = sig((k + n_order)*G)
    
    To find k:
    1. Start with sig(Q) = sig(k*G)
    2. Walk BACKWARD from Q using the recurrence:
       sig((k-1)*G) = sig(k*G) - Δ_sig(G)
       where Δ_sig(G) = sig(G) - sig(O) / period
    3. Count steps until we hit sig(G)
    4. The step count = k-1, so k = steps + 1
    """
    sig_Q = sig_normalized(Q_target)
    sig_G = sig_normalized(G)
    sig_O = 0.0  # Point at infinity
    
    print(f"  sig(Q) = {sig_Q:.10f}")
    print(f"  sig(G) = {sig_G:.10f}")
    print(f"  sig(O) = {sig_O:.10f}")
    print()
    
    # The recurrence step: Δ = sig(G) - sig(O)
    delta = sig_G - sig_O
    print(f"  Δ = sig(G) - sig(O) = {delta:.10f}")
    print()
    
    # Walk backward from Q to G
    current_sig = sig_Q
    current_point = Q_target
    steps = 0
    
    print("  Walking backward from Q to G...")
    
    while steps < max_steps:
        # Check if we reached G
        if current_point is not None and current_point == G:
            print(f"\n  🎉 HIT G at step {steps}!")
            return steps, steps + 1
        
        # Check if we hit infinity
        if current_point is None:
            print(f"\n  ⚠️ Hit infinity at step {steps}")
            # At infinity: sig = 0, need to continue
            pass
        
        # Step backward: subtract G
        current_point = point_sub(current_point, G)
        current_sig = sig_normalized(current_point)
        steps += 1
        
        if steps <= 5 or steps % 100 == 0:
            print(f"  Step {steps:4d}: sig={current_sig:.10f}, "
                  f"point={'INF' if current_point is None else hex(current_point[0])[:15]+'...'}")
    
    return None, steps


# ═══════════════════════════════════════════
# TEST: Tiny curve muna
# ═══════════════════════════════════════════
print("═══ TEST: Tiny Curve (p=17) ═══")
tiny_p = 17
tiny_G = (1, 5)

def tiny_sig(point):
    if point is None: return 0.0
    return (float(point[0]) * PHI + float(point[1]) * PSI) % float(tiny_p)

# Test recurrence on tiny curve
print("  Recurrence check on tiny curve:")
print(f"  sig(1*G) = {tiny_sig(tiny_G):.6f}")
print(f"  sig(2*G) = {tiny_sig(scalar_mult(2, tiny_G)):.6f}")
print(f"  sig(10*G) = {tiny_sig(scalar_mult(10, tiny_G)):.6f}")
print(f"  sig(1*G) == sig(10*G)? {tiny_sig(tiny_G) == tiny_sig(scalar_mult(10, tiny_G))}")
print(f"  (Period = 9, k=10 ≡ 1 mod 9)")
print()

# Test recurrence backtracking on tiny curve
print("═══ RECURRENCE BACKTRACK ON TINY CURVE ═══")
for secret in [3, 7]:
    Q_tiny = scalar_mult(secret, tiny_G)
    print(f"  Secret: k={secret}")
    
    # Backtrack
    current = Q_tiny
    for step in range(secret + 2):
        if current == tiny_G:
            print(f"    ✅ Found G at step {step}, k = {step + 1}")
            print(f"       (actual k={secret}, {'✅' if step + 1 == secret else '⚠️ equiv'})")
            break
        elif current is None and step > 0:
            print(f"    ⚠️ Infinity at step {step}, k = n - step = {9 - step}")
            break
        current = point_sub(current, tiny_G)
    print()

# ═══════════════════════════════════════════
# RECURRENCE ON secp256k1 (Small key test)
# ═══════════════════════════════════════════
print("═══ RECURRENCE BACKTRACK ON secp256k1 ═══")

for bits in [4, 8]:
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    
    # Backtrack
    current = Q_test
    start = time.time()
    for step in range(secret + 10):
        if current == G:
            elapsed = time.time() - start
            print(f"    ✅ Found G at step {step}, k = {step + 1}")
            print(f"       (actual k={secret}, {'✅ MATCH!' if step + 1 == secret else '⚠️'})")
            print(f"       Time: {elapsed:.6f}s")
            break
        elif current is None:
            print(f"    ⚠️ Infinity at step {step}")
            break
        current = point_sub(current, G)
    print()

# ═══════════════════════════════════════════
# THE KEY INSIGHT
# ═══════════════════════════════════════════
print("═══ THE RECURRENCE PATTERN ═══")
print()
print("  Ang recurrence ay DIRECT: sig(k*G) = sig(k*G ± G) ± Δ")
print("  Kung saan Δ = sig(G) - sig(O)")
print()
print("  PERO: Ang backtracking step-by-step ay O(k) — same as brute force.")
print()
print("  ANG PATTERN NA HINAHANAP NATIN:")
print("  Hindi 'step backward one by one.'")
print("  Kundi 'JUMP backward using the φ-period.'")
print()
print("  Ang signature sequence ay may SUB-PERIOD na related sa φ.")
print("  Kung ang φ-period ay m, edi:")
print("    sig(k*G) = sig((k - m)*G)")
print("  At k ≡ k' (mod m)")
print()
print("  Kung mahanap natin ang φ-sub-period,")
print("  pwede tayong mag-JUMP ng m steps at a time!")
print("  Instead of O(k) backtracking, O(k/m) lang!")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  RECURRENCE — 'Find the φ-sub-period, jump to k'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

