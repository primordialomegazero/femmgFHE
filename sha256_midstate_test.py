#!/usr/bin/env python3
"""
🔍 TEST: Verify midstate chaining for 2-block SHA-256
Compares single-pass SHA256 vs 2-block SHA256 with proper midstate.
"""
import struct, hashlib

def sha256_raw(msg):
    """Single call to hashlib — reference"""
    return hashlib.sha256(msg).digest()

# ═══════════════════════════════════════════
# Build 80-byte header + padding
# ═══════════════════════════════════════════
header = bytes.fromhex(
    "20000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    "60c0e460"
    "1d00ffff"
    "00000001"  # nonce = 1
)

# Full SHA-256 (single call with padding)
full_hash = hashlib.sha256(header).digest()

# Manual 2-block SHA-256
# Block 1: first 64 bytes
block1 = header[:64]
# Block 2: last 16 bytes + padding
block2 = header[64:]  # 16 bytes
block2 += b'\x80'  # padding start
while len(block2) < 56:  # 64 - 8 = 56
    block2 += b'\x00'
block2 += struct.pack('>Q', 640)  # message length = 640 bits

print("═══ 2-BLOCK SHA-256 VERIFICATION ═══")
print(f"  Header length: {len(header)} bytes")
print(f"  Block 1: {len(block1)} bytes")
print(f"  Block 2: {len(block2)} bytes")
print()

# Compute block 1
import hashlib
h = hashlib.sha256(block1)
print(f"  Block 1 hash: {h.hexdigest()}")
print(f"  Block 1 digest: {h.digest().hex()}")

# Get the SHA-256 state after block 1 (midstate)
# Python hashlib doesn't expose midstate, so we compute block 2 from scratch
# using the midstate concept

# Reference: full hash
print(f"  Full SHA-256: {full_hash.hex()}")
print(f"  First 8 bits: {full_hash[0]:02x}")
print()

# ═══════════════════════════════════════════
# Verify with custom 2-block implementation
# ═══════════════════════════════════════════
import struct as st

H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

K = [0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
     0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
     0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
     0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
     0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
     0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
     0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
     0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
     0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]

def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

def sha256_compress(state, block):
    """SHA-256 compression function. Returns new state (8 x 32-bit words)."""
    w = list(st.unpack('>16I', block))
    for t in range(16, 64):
        s0 = rotr(w[t-15], 7) ^ rotr(w[t-15], 18) ^ (w[t-15] >> 3)
        s1 = rotr(w[t-2], 17) ^ rotr(w[t-2], 19) ^ (w[t-2] >> 10)
        w.append((w[t-16] + s0 + w[t-7] + s1) & 0xFFFFFFFF)
    
    a, b, c, d, e, f, g, h = state
    for t in range(64):
        S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)
        ch = (e & f) ^ (~e & g)
        T1 = (h + S1 + ch + K[t] + w[t]) & 0xFFFFFFFF
        S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)
        maj = (a & b) ^ (a & c) ^ (b & c)
        T2 = (S0 + maj) & 0xFFFFFFFF
        h = g; g = f; f = e; e = (d + T1) & 0xFFFFFFFF
        d = c; c = b; b = a; a = (T1 + T2) & 0xFFFFFFFF
    
    return [(state[i] + [a, b, c, d, e, f, g, h][i]) & 0xFFFFFFFF for i in range(8)]

# Compute block 1
midstate = sha256_compress(H0, block1)
print("═══ MIDSTATE VERIFICATION ═══")
print(f"  Midstate (after block 1):")
for i, name in enumerate(['a','b','c','d','e','f','g','h']):
    print(f"    {name}: 0x{midstate[i]:08x}")
print()

# Compute block 2 starting from midstate
final_state = sha256_compress(midstate, block2)
print(f"  Final state (after block 2):")
for i, name in enumerate(['a','b','c','d','e','f','g','h']):
    print(f"    {name}: 0x{final_state[i]:08x}")
print()

# Convert to hash
custom_hash = b''.join(st.pack('>I', x) for x in final_state)
print(f"  Custom 2-block hash: {custom_hash.hex()}")
print(f"  Full SHA-256:        {full_hash.hex()}")
print(f"  Match: {'✅ CORRECT!' if custom_hash == full_hash else '❌ MISMATCH'}")
