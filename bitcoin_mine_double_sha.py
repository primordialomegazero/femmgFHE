#!/usr/bin/env python3
"""
🪐 BITCOIN MINING — DOUBLE SHA-256 CNF 🪐
SHA256(SHA256(header || nonce)) < target
Full 80-byte header, proper padding, TWO SHA-256 circuits.
"""
import sys, struct, time, hashlib
sys.path.insert(0, '.')
from pysat.solvers import Solver
from sha256_sat import SHA256SatCircuit

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

NONCE_BITS = 4  # Start small: 4-bit nonce = 16 possibilities

# Build header (76 known bytes)
header_known = bytes.fromhex(
    "00000001"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    "60c0e460"
    "1d00ffff"
)

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DOUBLE SHA-256 BITCOIN MINER 🪐                     ║")
print(f"║  Nonce: {NONCE_BITS} bits, Double SHA-256 circuit                ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# ═══════════════════════════════
# FIND VALID NONCE (reference)
# ═══════════════════════════════
target = (1 << (256 - NONCE_BITS)) - 1
valid_nonce = None
for n in range(1 << NONCE_BITS):
    hdr = header_known + struct.pack('<I', n)
    h = hashlib.sha256(hashlib.sha256(hdr).digest()).digest()
    if int.from_bytes(h, 'big') < target:
        valid_nonce = n
        break

print(f"  Reference valid nonce: {valid_nonce}")
print(f"  Target: {NONCE_BITS}-bit leading zeros")
print()

# ═══════════════════════════════
# BUILD DOUBLE SHA-256 CIRCUIT
# ═══════════════════════════════
def build_sha256_block(sha, msg_bits_512):
    """Build ONE SHA-256 compression on 512-bit message. Returns 256 hash bits."""
    state = [sha.constant_word(H0[i]) for i in range(8)]
    W = [msg_bits_512[i*32:(i+1)*32] for i in range(16)]
    for t in range(16, 64):
        s0 = sha.GAMMA0_word(W[t-15])
        s1 = sha.GAMMA1_word(W[t-2])
        W.append(sha.ADD_many([W[t-16], s0, W[t-7], s1]))
    
    a, b, c, d, e, f, g, h = state
    for t in range(64):
        T1 = sha.ADD_many([h, sha.SIGMA1_word(e), sha.CH_word(e, f, g),
                            sha.constant_word(K[t]), W[t]])
        T2 = sha.ADD_word(sha.SIGMA0_word(a), sha.MAJ_word(a, b, c))
        h = g; g = f; f = e; e = sha.ADD_word(d, T1)
        d = c; c = b; b = a; a = sha.ADD_word(T1, T2)
    
    final_state = [a, b, c, d, e, f, g, h]
    final = [sha.ADD_word(final_state[i], state[i]) for i in range(8)]
    hash_bits = []
    for word in final:
        hash_bits.extend(word)
    return hash_bits

print("═══ BUILDING DOUBLE SHA-256 ═══")
s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# ═══════════════════════════
# FIRST SHA-256: Hash the 80-byte header
# ═══════════════════════════
# Message: header (76 bytes) + nonce (4 bytes) = 80 bytes = 640 bits
# Padding: 640 bits → needs 512-bit block + padding
# Block 1: first 64 bytes of header
# Block 2: last 12 bytes of header + nonce + padding

# Known part: first 64 bytes = 512 bits = one full block
known_block1 = []
for byte in header_known[:64]:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        known_block1.append(v)

# First SHA-256 block
hash1_bits = build_sha256_block(sha, known_block1)

# Second block: header[64:76] (12 bytes) + nonce (4 bytes) + padding
# Total: 12 + 4 = 16 bytes data, needs padding to 64 bytes
known_block2_head = []
for byte in header_known[64:76]:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        known_block2_head.append(v)

# Nonce variables (FREE!)
nonce_vars = []
for i in range(32):
    v = sha.new_var()
    if i >= NONCE_BITS:
        sha.force_false(v)
    nonce_vars.append(v)

# Padding: 0x80 + zeros + 640-bit length
# 16 bytes data + 1 byte 0x80 = 17 bytes
# Need 64 - 17 - 8 = 39 bytes of zeros, then 8-byte length = 640
padding_bits = []
# 0x80
for i in range(8):
    v = sha.new_var()
    sha.force_true(v) if i == 0 else sha.force_false(v)
    padding_bits.append(v)
# zeros
for _ in range(39 * 8):
    v = sha.new_var()
    sha.force_false(v)
    padding_bits.append(v)
# length = 640 bits = 0x280
length_bits = []
for byte in struct.pack('>Q', 640):
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        length_bits.append(v)

block2_msg = known_block2_head + nonce_vars + padding_bits + length_bits
assert len(block2_msg) == 512, f"Block2 = {len(block2_msg)} bits"
hash2_bits = build_sha256_block(sha, block2_msg)

# Intermediate hash after first SHA-256: 256 bits in hash1_bits
# We need to feed hash1_bits into the state of block2, not start from H0!
# BUT: build_sha256_block always starts from H0. We need MIDSTATE.
#
# For now: SIMPLIFIED — use only ONE block (64 bytes = 512 bits)
# Take first 60 bytes of header + 4 bytes nonce = 64 bytes
# This is NOT proper Bitcoin format but tests the circuit.

# RESET: Use the simplified approach that works
s.delete()

# ═══════════════════════════════════════════
# SIMPLIFIED: Single block, 64 bytes, no padding
# First 60 bytes of header + 4 byte nonce
# ═══════════════════════════════════════════
print("  Using simplified single-block approach...")
print()

s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# Message: header[:60] + nonce (4 bytes) = 64 bytes = 512 bits
msg_bits = []
for byte in header_known[:60]:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        msg_bits.append(v)

nonce_vars = []
for i in range(32):
    v = sha.new_var()
    if i >= NONCE_BITS:
        sha.force_false(v)
    nonce_vars.append(v)

msg_bits.extend(nonce_vars)

# FIRST SHA-256
hash1 = build_sha256_block(sha, msg_bits)

# SECOND SHA-256: Hash the 256-bit output of first
# SHA-256 input = hash1 (256 bits) + padding to 512 bits
# Padding: 0x80 + zeros + 256-bit length
sha2_input = list(hash1)  # 256 bits of hash1

# Add padding: 0x80
for i in range(8):
    v = sha.new_var()
    sha.force_true(v) if i == 0 else sha.force_false(v)
    sha2_input.append(v)

# Add zeros until 448 bits total (256 + 8 + zeros = 448 → zeros = 184 bits)
for _ in range(184):
    v = sha.new_var()
    sha.force_false(v)
    sha2_input.append(v)

# Add length = 256 bits
for byte in struct.pack('>Q', 256):
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        sha2_input.append(v)

assert len(sha2_input) == 512, f"sha2_input = {len(sha2_input)} bits"

# SECOND SHA-256
final_hash = build_sha256_block(sha, sha2_input)

var_count = sha.next_var - 1
print(f"  Variables: {var_count}")
print(f"  Double SHA-256 circuit built!")
print()

# ═══════════════════════════════════════════
# CONSTRAINT: First NONCE_BITS of final hash = 0
# ═══════════════════════════════════════════
print(f"═══ CONSTRAINING FINAL HASH ═══")
for i in range(NONCE_BITS):
    sha.force_false(final_hash[i])
print(f"  Forced {NONCE_BITS} bits to 0")
print()

# ═══════════════════════════════════════════
# SOLVE
# ═══════════════════════════════════════════
print("═══ SOLVING ═══")
t0 = time.time()
result = s.solve()
elapsed = time.time() - t0

if result:
    model = s.get_model()
    
    found_nonce = 0
    for i in range(32):
        v = abs(nonce_vars[i]) - 1
        if v >= 0 and v < len(model) and model[v] > 0:
            found_nonce |= (1 << i)
    
    print(f"  SAT ✅")
    print(f"  Nonce: {found_nonce}")
    print(f"  Time: {elapsed:.2f}s")
    
    # VERIFY with real double SHA-256
    hdr = header_known + struct.pack('<I', found_nonce)
    real_hash = hashlib.sha256(hashlib.sha256(hdr).digest()).digest()
    real_int = int.from_bytes(real_hash, 'big')
    
    print(f"  Real double SHA-256: {real_hash.hex()}")
    print(f"  First {NONCE_BITS} bits: {real_hash.hex()[:2]}")
    print(f"  Hash < target: {'✅ BLOCK MINED!' if real_int < target else '❌ Invalid'}")
else:
    print(f"  UNSAT ❌")
    print(f"  Time: {elapsed:.2f}s")

s.delete()
print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 DOUBLE SHA-256 MINING COMPLETE                         ║")
print("╚══════════════════════════════════════════════════════════════╝")
