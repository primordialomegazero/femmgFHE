#!/usr/bin/env python3
"""
🪐 SAT MINER v2 — PROPER MIDSTATE CHAINING 🪐
Block 1 (known) → midstate (known constants)
Block 2 (has nonce) → starts from midstate → final hash
Only Block 2 needs CNF circuit!
"""
import sys, struct, time, hashlib
sys.path.insert(0, '.')
from pysat.solvers import Solver
from sha256_sat import SHA256SatCircuit

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

NONCE_BITS = 8

# ═══════════════════════════════════════════
# BUILD HEADER & COMPUTE MIDSTATE OFFLINE
# ═══════════════════════════════════════════
header_known = bytes.fromhex(
    "20000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    + format(int(time.time()), '08x')
    + "1f00ffff"
)

def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

def sha256_compress(state, block):
    w = list(struct.unpack('>16I', block))
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

H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

# Compute midstate from Block 1 (purely known data)
block1 = header_known[:64]
midstate = sha256_compress(H0, block1)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SAT MINER v2 — MIDSTATE CHAINING 🪐                ║")
print(f"║  Nonce: {NONCE_BITS} bits, Block 2 only CNF                    ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()
print(f"  Midstate computed offline:")
for i, name in enumerate(['a','b','c','d','e','f','g','h']):
    print(f"    {name}: 0x{midstate[i]:08x}")
print()

# ═══════════════════════════════════════════
# BUILD BLOCK 2 DATA (has nonce)
# ═══════════════════════════════════════════
# Block 2 = header[64:76] (12 bytes) + nonce (4 bytes) + padding
block2_head = header_known[64:76]  # 12 known bytes

# Build the full block 2 for reference
# Padding: data (12+4=16 bytes) + 0x80 + zeros + length=640
target = (1 << (256 - NONCE_BITS)) - 1

# Find valid nonces (brute force reference)
valid_nonces = []
for n in range(256):
    block2_data = block2_head + struct.pack('<I', n)
    block2 = block2_data + b'\x80'
    while len(block2) < 56:
        block2 += b'\x00'
    block2 += struct.pack('>Q', 640)
    final_state = sha256_compress(midstate, block2)
    h = b''.join(struct.pack('>I', x) for x in final_state)
    if h[0] == 0x00:  # First 8 bits = 0
        valid_nonces.append(n)

print(f"  Valid nonces (first byte=0x00): {valid_nonces}")
print()

# ═══════════════════════════════════════════
# BUILD CNF FOR BLOCK 2 ONLY
# ═══════════════════════════════════════════
print("═══ BUILDING CNF (BLOCK 2 ONLY) ═══")

t0 = time.time()
s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# Midstate as constants
midstate_vars = [sha.constant_word(midstate[i]) for i in range(8)]

# Block 2 message bits
block2_bits = []

# Known part: header[64:76] = 12 bytes
for byte in block2_head:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        block2_bits.append(v)

# Nonce (unknown!)
nonce_vars = []
for i in range(32):
    v = sha.new_var()
    if i >= NONCE_BITS:
        sha.force_false(v)
    nonce_vars.append(v)
block2_bits.extend(nonce_vars)

# Padding: 0x80
for i in range(8):
    v = sha.new_var()
    sha.force_true(v) if i == 0 else sha.force_false(v)
    block2_bits.append(v)

# Zeros (56 - 17 = 39 bytes = 312 bits)
for _ in range(39 * 8):
    v = sha.new_var()
    sha.force_false(v)
    block2_bits.append(v)

# Length = 640 bits
for byte in struct.pack('>Q', 640):
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        block2_bits.append(v)

assert len(block2_bits) == 512, f"Block2 = {len(block2_bits)} bits"

# SHA-256 compress starting from midstate
W = [block2_bits[i*32:(i+1)*32] for i in range(16)]
for t in range(16, 64):
    s0 = sha.GAMMA0_word(W[t-15])
    s1 = sha.GAMMA1_word(W[t-2])
    W.append(sha.ADD_many([W[t-16], s0, W[t-7], s1]))

a, b, c, d, e, f, g, h = midstate_vars
for t in range(64):
    T1 = sha.ADD_many([h, sha.SIGMA1_word(e), sha.CH_word(e, f, g),
                        sha.constant_word(K[t]), W[t]])
    T2 = sha.ADD_word(sha.SIGMA0_word(a), sha.MAJ_word(a, b, c))
    h = g; g = f; f = e; e = sha.ADD_word(d, T1)
    d = c; c = b; b = a; a = sha.ADD_word(T1, T2)

final_state = [a, b, c, d, e, f, g, h]
final = [sha.ADD_word(final_state[i], midstate_vars[i]) for i in range(8)]
hash_bits = []
for word in final:
    hash_bits.extend(word)

# First SHA-256 complete! Now second SHA-256...
# Hash the 256-bit output with padding
sha2_input = list(hash_bits)
# Padding for 256-bit message
sha2_input.append(sha.new_var()); sha.force_true(sha2_input[-1])
for i in range(7):
    sha2_input.append(sha.new_var()); sha.force_false(sha2_input[-1])
for _ in range(184):
    sha2_input.append(sha.new_var()); sha.force_false(sha2_input[-1])
for byte in struct.pack('>Q', 256):
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        sha2_input.append(v)

assert len(sha2_input) == 512

# Second SHA-256 (starts from H0, not midstate!)
state2 = [sha.constant_word(H0[i]) for i in range(8)]
W2 = [sha2_input[i*32:(i+1)*32] for i in range(16)]
for t in range(16, 64):
    s0 = sha.GAMMA0_word(W2[t-15])
    s1 = sha.GAMMA1_word(W2[t-2])
    W2.append(sha.ADD_many([W2[t-16], s0, W2[t-7], s1]))

a2, b2, c2, d2, e2, f2, g2, h2 = state2
for t in range(64):
    T1 = sha.ADD_many([h2, sha.SIGMA1_word(e2), sha.CH_word(e2, f2, g2),
                        sha.constant_word(K[t]), W2[t]])
    T2 = sha.ADD_word(sha.SIGMA0_word(a2), sha.MAJ_word(a2, b2, c2))
    h2 = g2; g2 = f2; f2 = e2; e2 = sha.ADD_word(d2, T1)
    d2 = c2; c2 = b2; b2 = a2; a2 = sha.ADD_word(T1, T2)

final2 = [sha.ADD_word([a2,b2,c2,d2,e2,f2,g2,h2][i], state2[i]) for i in range(8)]
final_hash_bits = []
for word in final2:
    final_hash_bits.extend(word)

build_time = time.time() - t0
print(f"  Variables: {sha.next_var - 1:,}")
print(f"  Build time: {build_time:.2f}s")
print()

# ═══════════════════════════════════════════
# CONSTRAIN + SOLVE
# ═══════════════════════════════════════════
print("═══ SAT SOLVING ═══")
for i in range(8):  # First byte = 0x00
    sha.force_false(final_hash_bits[i])

t0 = time.time()
result = s.solve()
solve_time = time.time() - t0

if result:
    model = s.get_model()
    found_nonce = 0
    for i in range(32):
        v = abs(nonce_vars[i]) - 1
        if v >= 0 and v < len(model) and model[v] > 0:
            found_nonce |= (1 << i)
    
    print(f"  ✅ SAT SOLVED! Nonce: {found_nonce}")
    print(f"  Solve time: {solve_time:.2f}s")
    
    # VERIFY
    block2_data = block2_head + struct.pack('<I', found_nonce)
    block2 = block2_data + b'\x80'
    while len(block2) < 56:
        block2 += b'\x00'
    block2 += struct.pack('>Q', 640)
    
    hash1_state = sha256_compress(midstate, block2)
    hash1 = b''.join(struct.pack('>I', x) for x in hash1_state)
    hash2 = hashlib.sha256(hash1).digest()
    
    print(f"  Real hash: {hash2.hex()}")
    print(f"  First byte: 0x{hash2[0]:02x}")
    print(f"  Hash < target: {'✅ BLOCK MINED!' if hash2[0] == 0x00 else '❌'}")
    print(f"  Nonce valid: {'✅' if found_nonce in valid_nonces else '❌'}")
else:
    print(f"  ❌ UNSAT")
    print(f"  Valid nonces exist: {valid_nonces}")

s.delete()
print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SAT MINER v2 — MIDSTATE CHAINING COMPLETE              ║")
print("╚══════════════════════════════════════════════════════════════╝")
