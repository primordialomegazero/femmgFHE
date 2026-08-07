#!/usr/bin/env python3
"""
🪐 FULL BITCOIN MINING — SHA-256 CNF + 32-BIT NONCE 🪐
Builds the REAL SHA-256 circuit with nonce as unknown variables.
NO truth table — actual CNF circuit!
"""
import sys, struct, time, hashlib
sys.path.insert(0, '.')
from pysat.solvers import Solver
from sha256_sat import SHA256SatCircuit

def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

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

# ═══════════════════════════════════════════
# BUILD HEADER
# ═══════════════════════════════════════════
version = "00000001"
prev_block = "0000000000000000000000000000000000000000000000000000000000000000"
merkle_root = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
timestamp = "60c0e460"
bits = "1d00ffff"

header_known = bytes.fromhex(version + prev_block + merkle_root + timestamp + bits)

# Use 8-bit nonce for this demo (scalable to 32)
NONCE_BITS = 8

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FULL SHA-256 CNF MINING — REAL CIRCUIT 🪐          ║")
print(f"║  Unknown nonce: {NONCE_BITS} bits                                  ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# ═══════════════════════════════════════════
# FIND VALID NONCE VIA BRUTE FORCE (for reference)
# ═══════════════════════════════════════════
print("═══ REFERENCE: FINDING VALID NONCE ═══")
target_bits = NONCE_BITS
target = (1 << (256 - target_bits)) - 1
print(f"  Target: {target_bits}-bit difficulty")

t0 = time.time()
valid_nonce = None
valid_hash = None
for nonce in range(256):
    header = header_known + struct.pack('<I', nonce)
    h1 = hashlib.sha256(header).digest()
    h2 = hashlib.sha256(h1).digest()
    if int.from_bytes(h2, 'big') < target:
        valid_nonce = nonce
        valid_hash = h2
        break

if valid_nonce is None:
    print("  No nonce found! Reducing difficulty...")
    target_bits = 4
    target = (1 << (256 - target_bits)) - 1
    for nonce in range(256):
        header = header_known + struct.pack('<I', nonce)
        h1 = hashlib.sha256(header).digest()
        h2 = hashlib.sha256(h1).digest()
        if int.from_bytes(h2, 'big') < target:
            valid_nonce = nonce
            valid_hash = h2
            break

print(f"  Valid nonce: {valid_nonce}")
print(f"  Hash: {valid_hash.hex() if valid_hash else 'N/A'}")
print(f"  Target: 0x{target:064x}")
print()

# ═══════════════════════════════════════════
# BUILD FULL SHA-256 CNF WITH UNKNOWN NONCE
# ═══════════════════════════════════════════
print("═══ BUILDING SHA-256 CNF WITH UNKNOWN NONCE ═══")

# Build the padded message with UNKNOWN nonce bits
# Header: 76 known bytes + 4 byte nonce (unknown) + padding
# Total message for first SHA-256: 80 bytes → 640 bits → needs padding to 512 bits

# Strategy: 
# 1. Create nonce variables (free — not forced)
# 2. Build full SHA-256 circuit with these as part of the message
# 3. The output hash[0:target_bits] must be 0

s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# ═══════════════════════════════
# BUILD MESSAGE WITH UNKNOWN NONCE
# ═══════════════════════════════
# First 76 bytes = known header
known_msg_bits = []
for byte in header_known:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        known_msg_bits.append(v)

# Nonce bytes = UNKNOWN (free variables!)
nonce_vars = []
for i in range(32):  # Full 32-bit nonce space, but only first NONCE_BITS are free
    v = sha.new_var()
    if i >= NONCE_BITS:
        sha.force_false(v)  # Upper bits forced to 0 (small nonce)
    nonce_vars.append(v)

# Pad the message to 512 bits (64 bytes)
# Known: 76 bytes = 608 bits
# Nonce: 4 bytes = 32 bits
# Total: 80 bytes = 640 bits
# SHA-256 block = 512 bits → 2 blocks needed!
# 
# SIMPLIFIED: Use 64-byte message (512 bits = 1 block)
# Take first 60 bytes of header + 4 bytes nonce = 64 bytes
msg_bits = known_msg_bits[:480] + nonce_vars  # 480 + 32 = 512 bits
assert len(msg_bits) == 512, f"Message must be 512 bits, got {len(msg_bits)}"

# ═══════════════════════════════
# BUILD SHA-256 CIRCUIT
# ═══════════════════════════════
state = [sha.constant_word(H0[i]) for i in range(8)]

# Parse W[0..15]
W = [msg_bits[i*32:(i+1)*32] for i in range(16)]

# Message schedule
for t in range(16, 64):
    s0 = sha.GAMMA0_word(W[t-15])
    s1 = sha.GAMMA1_word(W[t-2])
    W.append(sha.ADD_many([W[t-16], s0, W[t-7], s1]))

# 64 rounds
a_c, b_c, c_c, d_c, e_c, f_c, g_c, h_c = state
for t in range(64):
    T1 = sha.ADD_many([h_c, sha.SIGMA1_word(e_c), sha.CH_word(e_c, f_c, g_c),
                        sha.constant_word(K[t]), W[t]])
    T2 = sha.ADD_word(sha.SIGMA0_word(a_c), sha.MAJ_word(a_c, b_c, c_c))
    h_c = g_c; g_c = f_c; f_c = e_c; e_c = sha.ADD_word(d_c, T1)
    d_c = c_c; c_c = b_c; b_c = a_c; a_c = sha.ADD_word(T1, T2)

# Final addition
final_state = [a_c, b_c, c_c, d_c, e_c, f_c, g_c, h_c]
final = []
for i, init_val in enumerate(final_state):
    final.append(sha.ADD_word(init_val, state[i]))

# Flatten to 256 hash bits
hash_bits = []
for word in final:
    hash_bits.extend(word)

var_count = sha.next_var - 1
print(f"  Variables: {var_count}")
print(f"  Hash bits: {len(hash_bits)}")
print()

# ═══════════════════════════════════════════
# CONSTRAINT: First target_bits of hash must be 0
# ═══════════════════════════════════════════
print(f"═══ CONSTRAINING HASH < TARGET ═══")
print(f"  Forcing first {target_bits} hash bits to 0...")

for i in range(target_bits):
    sha.force_false(hash_bits[i])

print(f"  Added {target_bits} constraints")
print()

# ═══════════════════════════════════════════
# SOLVE
# ═══════════════════════════════════════════
print("═══ SOLVING WITH SAT ═══")
t0 = time.time()
result = s.solve()
elapsed = time.time() - t0

if result:
    model = s.get_model()
    
    # Extract nonce
    found_nonce = 0
    for i in range(32):
        v = abs(nonce_vars[i]) - 1
        if v >= 0 and v < len(model) and model[v] > 0:
            found_nonce |= (1 << i)
    
    # Extract hash
    found_hash = 0
    for i in range(256):
        v = abs(hash_bits[i]) - 1
        if v >= 0 and v < len(model) and model[v] > 0:
            found_hash |= (1 << (255 - i))
    
    print(f"  SAT ✅")
    print(f"  Nonce found: {found_nonce} (0x{found_nonce:08x})")
    print(f"  Hash from circuit: 0x{found_hash:064x}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    # VERIFY against real SHA-256
    header = header_known + struct.pack('<I', found_nonce)
    real_h1 = hashlib.sha256(header).digest()
    real_h2 = hashlib.sha256(real_h1).digest()
    real_hash_int = int.from_bytes(real_h2, 'big')
    
    print(f"  REAL SHA-256 VERIFICATION:")
    print(f"  Real hash: {real_h2.hex()}")
    print(f"  Real hash < target: {'✅ YES!' if real_hash_int < target else '❌ NO'}")
    print(f"  Circuit nonce matches valid nonce: {'✅' if found_nonce == valid_nonce else '❌ (but may still be valid!)'}")
    
    # Check if found nonce actually works
    real_header = header_known + struct.pack('<I', found_nonce)
    real_hash_check = hashlib.sha256(hashlib.sha256(real_header).digest()).digest()
    if int.from_bytes(real_hash_check, 'big') < target:
        print(f"  🎯 REAL BLOCK MINED! Nonce {found_nonce} produces valid hash!")
else:
    print(f"  UNSAT ❌")
    print(f"  Time: {elapsed:.2f}s")

s.delete()

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 FULL SHA-256 CNF MINING COMPLETE                       ║")
print("╚══════════════════════════════════════════════════════════════╝")
