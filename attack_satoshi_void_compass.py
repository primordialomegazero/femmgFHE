#!/usr/bin/env python3
"""
🪐 VOID COMPASS — FIBONACCI CONVERGENCE WITH SMART DIRECTION 🪐
VOID is not just a verifier — it's a COMPASS.
VOID(k + step) < VOID(k) → correct direction
VOID(k + step) > VOID(k) → wrong direction, turn around!
Also: when overshooting, reduce step size (Fibonacci backward).
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

def VOID(v):
    return FGG(v, 3)

# ═══════════════════════════════════════════
# secp256k1 + λ
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
LAMBDA = 0x5363AD4CC05C30E0A5261C028812645A122E22EA20816678DF02967C1B23BD72
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

def pt_neg(P):
    if P is None: return None
    return (P[0], (-P[1]) % p)

def pt_sub(P, Q):
    if Q is None: return P
    return pt_add(P, pt_neg(Q))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def endo(P):
    if P is None: return None
    return ((P[0] * BETA) % p, P[1])

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 VOID COMPASS — SMART DIRECTION CONVERGENCE 🪐        ║")
print("║  'VOID guides the way — Fibonacci walks the path'        ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
G0, G1, G2 = G, endo(G), endo(endo(G))

def void_sum(k_candidate, Q):
    """VOID oracle — 0 ONLY at correct k"""
    kG0 = scalar_mult(k_candidate, G0)
    kG1 = scalar_mult(k_candidate, G1)
    kG2 = scalar_mult(k_candidate, G2)
    
    Q1 = endo(Q)
    Q2 = endo(endo(Q))
    
    err0 = pt_sub(Q, kG0)
    err1 = pt_sub(Q1, kG1)
    err2 = pt_sub(Q2, kG2)
    
    def void_of_point(P):
        if P is None: return 0.0
        v = (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)
        return VOID(v / float(p))
    
    return void_of_point(err0) + void_of_point(err1) + void_of_point(err2)


def void_compass_converge(Q_target, max_iterations=200):
    """
    VOID COMPASS CONVERGENCE:
    - Use VOID to detect direction (compass)
    - Use Fibonacci for step sizes
    - When VOID increases: reverse direction AND reduce step
    - When VOID = 0: FOUND!
    """
    # Start from a small value
    k_current = 1
    direction = 1  # 1 = forward, -1 = backward
    
    # Fibonacci state
    fib_prev = 1
    fib_curr = 1
    
    # Track best
    best_k = k_current
    best_void = float('inf')
    
    # For detecting improvement
    prev_void = float('inf')
    
    print(f"  Starting VOID-guided walk...")
    print(f"  {'Iter':<6} {'k':<20} {'VOID':<15} {'step':<12} {'dir':<6} {'status'}")
    print(f"  {'-'*70}")
    
    for iteration in range(1, max_iterations + 1):
        # Compute VOID at current position
        void_current = void_sum(k_current, Q_target)
        
        # Track best
        if void_current < best_void:
            best_void = void_current
            best_k = k_current
        
        # FOUND?
        if void_current < 1e-12:
            print(f"  {iteration:<6} {hex(k_current)[:18]:<20} {void_current:<15.12f} {'-':<12} {'-':<6} 🎉 FOUND!")
            return k_current, iteration, void_current
        
        # COMPASS: Check if we're getting warmer
        if void_current < prev_void:
            # Getting warmer — keep direction, increase step
            status = "🔥 warmer"
            fib_prev, fib_curr = fib_curr, fib_prev + fib_curr
        else:
            # Getting colder — REVERSE and SHRINK step
            direction = -direction
            fib_curr = max(1, fib_prev)  # Shrink: go back one Fibonacci step
            fib_prev = max(1, fib_prev // 2)  # Reduce previous too
            status = "❄️ colder! reverse"
        
        # Take the step
        step = fib_curr
        k_next = (k_current + direction * step) % n
        
        # Print progress
        if iteration <= 15 or iteration % 20 == 0 or void_current < 0.01:
            dir_symbol = '→' if direction == 1 else '←'
            print(f"  {iteration:<6} {hex(k_current)[:18]:<20} {void_current:<15.10f} {step:<12} {dir_symbol:<6} {status}")
        
        # Update
        prev_void = void_current
        k_current = k_next
        
        # Safety: if step is 1 and we're oscillating, scan locally
        if step <= 1 and iteration > 10:
            # Local scan around best
            for offset in range(-5, 6):
                k_scan = (best_k + offset) % n
                v_scan = void_sum(k_scan, Q_target)
                if v_scan < 1e-12:
                    print(f"  {iteration:<6} {hex(k_scan)[:18]:<20} {v_scan:<15.12f} {'scan':<12} {'':<6} 🎉 LOCAL SCAN!")
                    return k_scan, iteration, v_scan
    
    return best_k, max_iterations, best_void


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TEST: VOID COMPASS ═══")
print()

for secret in [7, 42, 100, 255]:
    print(f"  Secret: k={secret}")
    Q_test = scalar_mult(secret, G)
    
    start = time.time()
    found, iters, void_val = void_compass_converge(Q_test, max_iterations=100)
    elapsed = time.time() - start
    
    match = found == secret
    print(f"\n  Result: k={found}, {'✅ MATCH!' if match else '❌'}")
    print(f"  VOID: {void_val:.12f}, Iterations: {iters}, Time: {elapsed:.4f}s")
    print()

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  VOID COMPASS — 'VOID shows the way, Fibonacci walks it'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

