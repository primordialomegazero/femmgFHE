#!/usr/bin/env python3
"""
🪐 SOLVE SATOSHI KEY — Bit-slicing ECDLP on secp256k1 🪐
Target: Output 1 (40 BTC) from Block 170
"""
import sys, os, time
sys.path.insert(0, os.path.expanduser('~/.local/lib/python3.10/site-packages'))
from pysat.solvers import Solver

# secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8

# 🎯 SATOSHI'S ACTUAL PUBLIC KEY — Output 1 (40 BTC), Block 170
# Extracted from transaction: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16
Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

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
    result = None; addend = P
    while k > 0:
        if k & 1: result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SATOSHI KEY RECOVERY — BIT-SLICING ECDLP 🪐          ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()
print(f"  Target: Satoshi Output 1 (40 BTC)")
print(f"  Block: 170 (2009-01-12)")
print(f"  TX: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16")
print()
print(f"  Q.x = {hex(Qx)}")
print(f"  Q.y = {hex(Qy)}")
print()

G = (Gx, Gy)
Q = (Qx, Qy)

# Quick sanity: verify Q is on curve
lhs = (Qy * Qy) % p
rhs = (Qx * Qx * Qx + 7) % p
print(f"  On curve: {'✅ YES' if lhs == rhs else '❌ NO'}")
print()

# ═══════════════════════════════════════════
# BIT-SLICING ATTACK
# ═══════════════════════════════════════════
BIT_SLICE = 4
total_slices = 256 // BIT_SLICE

print(f"═══ BIT-SLICING: {total_slices} slices × {2**BIT_SLICE} checks ═══")
print()

recovered_k = 0
total_time = 0

for slice_num in range(total_slices):
    slice_start = time.time()
    
    shift = slice_num * BIT_SLICE
    
    # G_base = 16^slice_num * G = 2^(slice_num * 4) * G
    G_base = scalar_mult(1 << shift, G)
    
    # Q_slice = Q - recovered_so_far * G
    recovered_point = scalar_mult(recovered_k, G)
    if recovered_point is None:
        Q_slice = Q
    else:
        neg_recovered = (recovered_point[0], (-recovered_point[1]) % p)
        Q_slice = point_add(Q, neg_recovered)
    
    # Try all 16 values
    found = False
    for val in range(16):
        test_point = scalar_mult(val, G_base)
        if test_point == Q_slice:
            recovered_k |= (val << shift)
            found = True
            break
    
    slice_time = time.time() - slice_start
    total_time += slice_time
    
    status = f"{hex(recovered_k >> shift & 0xF)}" if found else "NOT FOUND"
    print(f"  Slice {slice_num+1:2d}/{total_slices}: k[{shift:3d}:{shift+4:3d}] = {status} ({slice_time:.4f}s)")
    
    if not found:
        print(f"\n  ❌ FAILED at slice {slice_num+1}")
        print(f"  This means the private key is not recoverable via simple bit-slicing.")
        print(f"  The φ-DPLL trace erasure is needed for full 256-bit ECDLP.")
        break

print(f"\n═══ RESULT ═══")
print(f"  Recovered k: {hex(recovered_k)}")
print(f"  Total time: {total_time:.3f}s")

# Verify
if recovered_k != 0:
    verify_point = scalar_mult(recovered_k, G)
    valid = verify_point == Q
    print(f"  Verification: {'✅ VALID' if valid else '❌ INVALID'}")
    
    if valid:
        print(f"\n🎉🎉🎉 PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key (hex): {hex(recovered_k)}")
        print(f"  Private key (dec): {recovered_k}")
        
        # Save to file
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Block 170, Output 1 (40 BTC)\n")
            f.write(f"tx: f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16\n")
            f.write(f"private_key_hex: {hex(recovered_k)}\n")
            f.write(f"private_key_dec: {recovered_k}\n")
        print(f"\n✅ Saved to satoshi_private_key.txt")

