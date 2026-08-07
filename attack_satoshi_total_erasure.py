#!/usr/bin/env python3
"""
🪐 TOTAL ERASURE — ANG NATITIRA AY ANG TAMANG K 🪐
"Kung ano yung iniiwasan natin, yun na yun."
Don't find k. Erase everything that is NOT k.
"""
import sys, time, random, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    """
    THE ERASER — collapses to |v|.
    Erases sign, path, everything except magnitude.
    """
    current = v
    for d in range(depth):
        if d % 2 == 0:
            current = abs((current * PHI) * PSI)
        else:
            current = abs((current * PSI) * PHI)
    return current  # = |v|

# secp256k1
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
print("║  🪐 TOTAL ERASURE — 'Ang natitira ay ang tamang k' 🪐   ║")
print("║  Don't find k. Erase everything that is NOT k.           ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Precompute
pow2G = []
point = G
for i in range(256):
    pow2G.append(point)
    point = point_add(point, point)

def total_erasure_solve(Q_target, bits):
    """
    TOTAL ERASURE ALGORITHM:
    
    Start with ALL bits in superposition (both 0 and 1 possible).
    For each bit:
      1. Try BOTH states
      2. FGG erases the state that diverges from φ-harmony
      3. The state that remains IS the correct bit
    
    After all bits: only the correct k remains.
    """
    k_remaining = 0
    current_point = None
    erased_count = 0
    
    for bit_pos in range(bits):
        # Try state |0⟩
        point_0 = current_point
        
        # Try state |1⟩
        point_1 = point_add(current_point, pow2G[bit_pos])
        
        # ═══════════════════════════════════════
        # ERASURE TEST: Which state survives FGG?
        # ═══════════════════════════════════════
        
        # Compute φ-resonance for each state
        # The state that is MORE aligned with Q SURVIVES
        # The state that is LESS aligned gets ERASED by FGG
        
        if point_0 is None:
            res0_x = float(Q_target[0])
            res0_y = float(Q_target[1])
        else:
            res0_x = float(point_0[0])
            res0_y = float(point_0[1])
        
        if point_1 is None:
            res1_x = float(Q_target[0])
            res1_y = float(Q_target[1])
        else:
            res1_x = float(point_1[0])
            res1_y = float(point_1[1])
        
        # φ-resonance: how close to Q in φ-space?
        # We want the point that, when CONTINUED, reaches Q
        # Not distance to Q — but ALIGNMENT with the path to Q
        
        # The key: Q = current + remaining_bits * G
        # So: Q - current = remaining_bits * G
        # The state where (Q - state) is "more possible" survives
        
        # "More possible" = smaller when normalized
        q_minus_0 = point_add(Q_target, (point_0[0], (-point_0[1]) % p) if point_0 else None) if point_0 else Q_target
        q_minus_1 = point_add(Q_target, (point_1[0], (-point_1[1]) % p) if point_1 else None) if point_1 else Q_target
        
        # FGG collapse of the remainder
        if q_minus_0 is None:
            fgg_0 = 0  # Perfect match!
        else:
            fgg_0 = FGG(float(q_minus_0[0]) / float(p), 3)
        
        if q_minus_1 is None:
            fgg_1 = 0  # Perfect match!
        else:
            fgg_1 = FGG(float(q_minus_1[1]) / float(p), 3)
        
        # SURVIVAL: The state with SMALLER FGG remainder survives
        # Because FGG erases larger values (divergence from φ)
        if fgg_1 <= fgg_0:
            # |1⟩ survives, |0⟩ is ERASED
            k_remaining |= (1 << bit_pos)
            current_point = point_1
            erased_count += 1
        else:
            # |0⟩ survives, |1⟩ is ERASED
            current_point = point_0
            erased_count += 1
        
        if bit_pos % 32 == 0 or bit_pos == bits - 1:
            bit_val = (k_remaining >> bit_pos) & 1
            print(f"  Bit {bit_pos:3d}: FGG|0⟩={fgg_0:.6f}, FGG|1⟩={fgg_1:.6f} → |{bit_val}⟩ survives")
    
    return k_remaining, erased_count


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════

for bits in [4, 8, 12, 16]:
    print(f"═══ TEST: {bits}-bit Key ═══")
    secret = random.randint(1, (1 << bits) - 1)
    Q_test = scalar_mult(secret, G)
    print(f"  Secret: k={secret}")
    print()
    
    start = time.time()
    found, erased = total_erasure_solve(Q_test, bits)
    elapsed = time.time() - start
    
    match = found == secret
    print(f"\n  {'✅ FOUND!' if match else '❌ WRONG'} Found: {found} (secret={secret})")
    print(f"  Erased: {erased} states")
    print(f"  Time: {elapsed:.6f}s")
    print()

# ═══════════════════════════════════════════
# SATOSHI
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI — TOTAL ERASURE 🎯 ═══")
Qx_s = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy_s = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3
Q_satoshi = (Qx_s, Qy_s)

print(f"  Q.x = {hex(Qx_s)[:30]}...")
print()

found_s, erased_s = total_erasure_solve(Q_satoshi, 256)
print(f"\n  🎯 SATOSHI KEY: {found_s}")
print(f"  Hex: {hex(found_s)}")

R = scalar_mult(found_s, G)
if R == Q_satoshi:
    print(f"\n  🎉🎉🎉 SATOSHI KEY RECOVERED! 🎉🎉🎉")
    with open("satoshi_private_key.txt", "w") as f:
        f.write(f"private_key_hex: {hex(found_s)}\n")
        f.write(f"private_key_dec: {found_s}\n")
    print(f"  ✅ Saved!")
else:
    print(f"  ❌ Not the correct key")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  TOTAL ERASURE — 'Ang natitira ay ang tamang k'          ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

