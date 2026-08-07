#!/usr/bin/env python3
"""
🪐 13-LEVEL FIBONACCI HOMOMORPHIC ATTACK 🪐
'13 Homomorphic Shits = 13 Levels of Fibonacci Decomposition!'
Level 1: φ(Q) = φ(F_a*G) + φ(remainder) [doubling linear]
Level 2: φ(rem) = φ(F_b*G) + φ(rem)     [doubling linear]
...
Level 13: φ(rem) = φ(F_m*G)             [identity!]
Each level is homomorphic → direct decomposition!
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

def VOID_point(P):
    if P is None: return 0.0
    v = (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)
    return FGG(v / float(p), 3)

# ═══════════════════════════════════════════
# secp256k1
# ═══════════════════════════════════════════
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

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
    """Golden Ratio Map: φ(P) = x·φ + y·ψ mod p"""
    if P is None: return 0.0
    return (float(P[0]) * PHI + float(P[1]) * PSI) % float(p)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 13-LEVEL FIBONACCI HOMOMORPHIC ATTACK 🪐            ║")
print("║  '13 levels of doubling linearity = 13 equations!'      ║")
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

print("  Building Fibonacci basis...")
fib_l3 = {}
fib_list = []
fib_phi = {}  # Fibonacci → φ-value

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    fib_list.append(f)
    fib_phi[f] = phi_map(l3_P)

fibs_desc = sorted(fib_l3.keys(), reverse=True)
print(f"  ✅ {len(fib_l3)} Fibonacci entries")
print()

# ═══════════════════════════════════════════
# 13-LEVEL HOMOMORPHIC DECOMPOSITION
# ═══════════════════════════════════════════
def homomorphic_13_level(Q_l3, max_levels=13):
    """
    13-LEVEL HOMOMORPHIC DECOMPOSITION:
    
    Sa bawat level:
    1. φ(current) = φ(F_i * G) + φ(remainder) [doubling linear]
    2. Hanapin ang F_i na nagmi-minimize ng φ-difference
    3. I-subtract, i-record, i-continue
    
    13 levels = 13 Fibonacci components!
    """
    current = Q_l3
    components = []
    phi_current = phi_map(current)
    
    print(f"  Starting φ = {phi_current:.6f}")
    print(f"  {'Level':<8} {'F_i':<20} {'φ(rem)':<16} {'Δφ':<12} {'Status'}")
    print(f"  {'-'*70}")
    
    for level in range(1, max_levels + 1):
        if current is None:
            break
        
        # Hanapin ang F_i na may PINAKAMALAPIT na φ-value
        best_f = None
        best_remainder = None
        best_phi_rem = float('inf')
        best_diff = float('inf')
        
        for f in fibs_desc:
            if f in components:
                continue
            
            remainder = pt_sub(current, fib_l3[f])
            phi_rem = phi_map(remainder)
            
            # HOMOMORPHIC CHECK: φ(current) ≈ φ(F_i*G) + φ(remainder)
            phi_expected = (fib_phi[f] + phi_rem) % float(p)
            diff = abs(phi_current - phi_expected)
            
            if diff < best_diff:
                best_diff = diff
                best_f = f
                best_remainder = remainder
                best_phi_rem = phi_rem
        
        if best_f is None:
            if level <= 5:
                print(f"  {level:<8} {'---':<20} {'---':<16} {'---':<12} ❌ No F_i found")
            break
        
        # TANGGAPIN!
        components.append(best_f)
        current = best_remainder
        phi_current = best_phi_rem
        
        status = "✅" if best_diff < 0.01 else "⚠️"
        if level <= 13:
            print(f"  {level:<8} {best_f:<20} {best_phi_rem:<16.6f} {best_diff:<12.6f} {status}")
    
    return components


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("═══ TESTING 13-LEVEL HOMOMORPHIC ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components = homomorphic_13_level(Q_l3, max_levels=13)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"\n  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    Levels: {len(components)}, Time: {elapsed:.4f}s")

# ═══════════════════════════════════════════
# SATOSHI 13-LEVEL ATTACK
# ═══════════════════════════════════════════
print(f"\n═══ 🎯 SATOSHI 13-LEVEL ATTACK 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Running 13-Level Homomorphic Decomposition...")
print("  '13 levels = 13 equations = φ-linear decomposition!'")
print()

start = time.time()
components = homomorphic_13_level(Q_l3, max_levels=13)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 13-LEVEL DECOMPOSITION COMPLETE!")
    print(f"  Levels: {len(components)}")
    print(f"  k = {' + '.join(map(str, components))}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: 13-Level Fibonacci Homomorphic Attack")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — 13-Level Homomorphic\n")
            f.write(f"method: 13-Level Fibonacci Homomorphic Attack\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"levels: {len(components)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ Decomposition failed — {elapsed:.2f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  13 LEVELS — 'Homomorphic Fibonacci = Direct k!'         ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

