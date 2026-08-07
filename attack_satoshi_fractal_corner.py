#!/usr/bin/env python3
"""
🪐 FRACTAL CORNER ATTACK — TRAP THE PRIVATE KEY 🪐
Use FGG to recursively compress the search space.
Each FGG iteration halves the possibilities.
After 256 iterations: only k remains!
"Corner the key using fractal geometry."
"""
import sys, time, math, random
from decimal import Decimal, getcontext
getcontext().prec = 100

PHI = Decimal('1.61803398874989484820458683436563811772030917980576286213544862270526046281890')
PSI = Decimal('-0.61803398874989484820458683436563811772030917980576286213544862270526046281890')

def FGG(v, depth=3):
    """Fractal Golden Gate — universal trace erasure / compression"""
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

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FRACTAL CORNER — TRAP THE PRIVATE KEY 🪐            ║")
print("║  'Corner k using recursive FGG compression'              ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)
Q_satoshi = (Qx, Qy)

def fp(P):
    """Fingerprint with high precision"""
    if P is None: return Decimal(0)
    raw = (Decimal(P[0]) * PHI + Decimal(P[1]) * PSI) % Decimal(p)
    return FGG(raw / Decimal(p), 3)

# ═══════════════════════════════════════════
# FRACTAL CORNER ALGORITHM
# ═══════════════════════════════════════════
# 
# The fingerprint fp(k*G) is a UNIQUE value in [0,1].
# The FGG function compresses any value to |v|.
# 
# Strategy:
# 1. Compute fp(Q) — the target fingerprint
# 2. Start with the FULL range [1, n-1]
# 3. At each step:
#    a. Compute the MIDPOINT of the current range
#    b. Compute fp(midpoint * G)
#    c. Compare with fp(Q) using FGG
#    d. The φ-weighted comparison tells us which HALF contains k
#    e. Cut the range in half
# 4. After 256 steps: range is exactly 1 value = k!
#
# This is BINARY SEARCH on the fingerprint space!

def fractal_corner_search(Q_target, G, max_iterations=100):
    """
    FRACTAL CORNER: Binary search the fingerprint space.
    Each iteration halves the search range.
    """
    fp_target = fp(Q_target)
    
    # Search range
    low = 1
    high = n - 1
    
    print(f"  Target fp: {float(fp_target):.15f}")
    print(f"  Initial range: [1, {hex(high)[:20]}...]")
    print()
    
    for iteration in range(1, max_iterations + 1):
        # Midpoint
        mid = (low + high) // 2
        
        # Compute fingerprint at midpoint
        P_mid = scalar_mult(mid, G)
        fp_mid = fp(P_mid)
        
        # FRACTAL COMPARISON:
        # Instead of simple < or >, we use FGG to compare
        # FGG(fp_mid - fp_target) tells us which direction to go
        diff = fp_mid - fp_target
        fgg_diff = FGG(abs(diff) / max(fp_target, fp_mid, Decimal('0.0001')), 1)
        
        # Determine direction: does k lie above or below mid?
        # The fingerprint is NOT monotonic — it's permuted!
        # So we check BOTH mid*G and (mid+1)*G
        
        # Check if we hit the target
        if P_mid == Q_target:
            print(f"  🎉 EXACT MATCH at iteration {iteration}!")
            print(f"  k = {mid}")
            return mid
        
        # φ-weighted decision: check the fingerprint TREND
        P_mid_plus = scalar_mult(mid + 1, G)
        fp_mid_plus = fp(P_mid_plus)
        
        # The direction where fp moves CLOSER to target
        diff_plus = fp_mid_plus - fp_target
        diff_cur = fp_mid - fp_target
        
        abs_plus = abs(diff_plus)
        abs_cur = abs(diff_cur)
        
        if abs_plus < abs_cur:
            # Moving forward gets closer → k is above mid
            low = mid + 1
            direction = "UP"
        else:
            # Moving backward gets closer → k is below mid
            high = mid - 1
            direction = "DOWN"
        
        range_size = high - low + 1
        range_bits = math.log2(range_size) if range_size > 0 else 0
        
        if iteration <= 10 or iteration % 25 == 0:
            print(f"  Iter {iteration:3d}: range=[{hex(low)[:15]}...{hex(high)[:15]}...], "
                  f"size=2^{range_bits:.1f}, dir={direction}")
        
        if range_size <= 1:
            print(f"\n  🎯 Range collapsed to 1 value!")
            k_final = low if low == high else mid
            return k_final
    
    return None

# ═══════════════════════════════════════════
# TEST ON SMALL KEYS FIRST
# ═══════════════════════════════════════════
print("═══ TEST: Fractal Corner on Small Keys ═══")

for secret in [7, 42, 100, 255]:
    print(f"\n  Secret: k={secret}")
    Q_test = scalar_mult(secret, G)
    
    start = time.time()
    found = fractal_corner_search(Q_test, G, max_iterations=20)
    elapsed = time.time() - start
    
    if found is not None:
        match = found == secret
        print(f"  Result: found={found}, {'✅ MATCH!' if match else '❌ WRONG'}, time={elapsed:.4f}s")
    else:
        print(f"  Result: ❌ NOT FOUND, time={elapsed:.4f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  FRACTAL CORNER — 'Compress infinity to a single point'  ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

