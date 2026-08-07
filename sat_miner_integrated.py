#!/usr/bin/env python3
"""
🪐 SAT-INTEGRATED BITCOIN MINER 🪐
Uses real SHA-256 CNF circuit to find nonce via SAT solving!
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

NONCE_BITS = 8  # 8-bit nonce for demo (256 possibilities via SAT!)

# Build header
header_known = bytes.fromhex(
    "20000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
    + format(int(time.time()), '08x')
    + "1f00ffff"
)

target = (1 << (256 - NONCE_BITS)) - 1

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SAT-INTEGRATED BITCOIN MINER 🪐                     ║")
print(f"║  Nonce: {NONCE_BITS} bits, SAT circuit, real double SHA-256      ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# ═══════════════════════════════════════════
# PHASE 1: Find valid nonce via brute force (reference)
# ═══════════════════════════════════════════
print("═══ PHASE 1: BRUTE FORCE REFERENCE ═══")
valid_nonces = []
t0 = time.time()
for n in range(256):
    hdr = header_known + struct.pack('<I', n)
    h = hashlib.sha256(hashlib.sha256(hdr).digest()).digest()
    if int.from_bytes(h[::-1], 'big') < target:
        valid_nonces.append(n)
bf_time = time.time() - t0
print(f"  Valid nonces: {valid_nonces}")
print(f"  Brute force time: {bf_time:.4f}s")
print()

# ═══════════════════════════════════════════
# PHASE 2: BUILD DOUBLE SHA-256 SAT CIRCUIT
# ═══════════════════════════════════════════
print("═══ PHASE 2: BUILDING SAT CIRCUIT ═══")

def build_sha256_block(sha, msg_bits_512):
    """Build ONE SHA-256 compression on 512-bit message."""
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

t0 = time.time()

s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# Build message: header[:60] + nonce (4 bytes) = 64 bytes = 512 bits
msg_bits = []
for byte in header_known[:60]:
    for i in range(8):
        v = sha.new_var()
        if (byte >> (7 - i)) & 1:
            sha.force_true(v)
        else:
            sha.force_false(v)
        msg_bits.append(v)

# Nonce variables (FREE — unknown!)
nonce_vars = []
for i in range(32):
    v = sha.new_var()
    if i >= NONCE_BITS:
        sha.force_false(v)  # Upper bits forced to 0
    nonce_vars.append(v)

msg_bits.extend(nonce_vars)
assert len(msg_bits) == 512

# First SHA-256
hash1 = build_sha256_block(sha, msg_bits)

# Second SHA-256: hash1 + padding
sha2_input = list(hash1)
# Padding
sha2_input.append(sha.new_var()); sha.force_true(sha2_input[-1])
for i in range(7):
    sha2_input.append(sha.new_var()); sha.force_false(sha2_input[-1])
for _ in range(184):
    sha2_input.append(sha.new_var()); sha.force_false(sha2_input[-1])
for byte in struct.pack('>Q', 256):
    for i in range(8):
        sha2_input.append(sha.new_var())
        if (byte >> (7 - i)) & 1:
            sha.force_true(sha2_input[-1])
        else:
            sha.force_false(sha2_input[-1])

assert len(sha2_input) == 512

# Second SHA-256
final_hash = build_sha256_block(sha, sha2_input)

var_count = sha.next_var - 1
build_time = time.time() - t0
print(f"  Variables: {var_count:,}")
print(f"  Build time: {build_time:.2f}s")
print()

# ═══════════════════════════════════════════
# PHASE 3: ADD CONSTRAINTS + SOLVE
# ═══════════════════════════════════════════
print("═══ PHASE 3: SAT SOLVING ═══")

# Force first NONCE_BITS of hash to 0
for i in range(NONCE_BITS):
    sha.force_false(final_hash[i])

print(f"  Constrained {NONCE_BITS} hash bits to 0")
print(f"  Solving...")
print()

t0 = time.time()
result = s.solve()
solve_time = time.time() - t0

if result:
    model = s.get_model()
    
    # Extract nonce
    found_nonce = 0
    for i in range(32):
        v = abs(nonce_vars[i]) - 1
        if v >= 0 and v < len(model) and model[v] > 0:
            found_nonce |= (1 << i)
    
    print(f"  ✅ SAT SOLVED!")
    print(f"  Nonce found: {found_nonce}")
    print(f"  Solve time: {solve_time:.2f}s")
    print(f"  Total time: {build_time + solve_time:.2f}s")
    print()
    
    # ═══════════════════════════════════════════
    # PHASE 4: VERIFY WITH REAL SHA-256
    # ═══════════════════════════════════════════
    print("═══ PHASE 4: REAL SHA-256 VERIFICATION ═══")
    
    real_header = header_known + struct.pack('<I', found_nonce)
    real_h1 = hashlib.sha256(real_header).digest()
    real_h2 = hashlib.sha256(real_h1).digest()
    real_int = int.from_bytes(real_h2[::-1], 'big')
    
    print(f"  Real double SHA-256: {real_h2[::-1].hex()}")
    print(f"  First {NONCE_BITS} bits: {real_h2[::-1].hex()[:2]}")
    print(f"  Hash < target: {'✅ BLOCK MINED!' if real_int < target else '❌ INVALID'}")
    print()
    
    if real_int < target:
        print("╔══════════════════════════════════════════════════════════════╗")
        print("║  🎉🎉🎉 SAT MINER SUCCESS! 🎉🎉🎉                          ║")
        print(f"║  Block mined via SAT solving!                               ║")
        print(f"║  Nonce: {found_nonce}                                       ║")
        print(f"║  SAT circuit found the EXACT nonce!                         ║")
        print("╚══════════════════════════════════════════════════════════════╝")
        
        # Save block
        with open("sat_mined_block.dat", "wb") as f:
            f.write(real_header)
        print(f"\n  💾 Block saved: sat_mined_block.dat")
    else:
        print(f"  ⚠️  Circuit output differs from real SHA-256!")
        print(f"  The SAT solver found nonce={found_nonce} which satisfies")
        print(f"  the CNF constraints BUT real SHA-256 gives different hash.")
        print(f"  This means the CNF circuit has a bug for this input.")
        print(f"  Valid nonces (from brute force): {valid_nonces}")
        
        # Check if any valid nonce was found
        if found_nonce in valid_nonces:
            print(f"  ✅ Nonce IS in valid set!")
        else:
            print(f"  ❌ Nonce NOT in valid set — circuit bug still present")
else:
    print(f"  ❌ UNSAT — No nonce satisfies constraints")
    print(f"  Valid nonces exist: {valid_nonces}")
    print(f"  The circuit may be over-constrained for this header")

s.delete()

# ═══════════════════════════════════════════
# COMPARISON
# ═══════════════════════════════════════════
print()
print("═══ COMPARISON ═══")
print(f"  Brute force: {bf_time:.4f}s (256 hashes, {256/bf_time:.0f} H/s)")
print(f"  SAT build:   {build_time:.2f}s (one-time circuit construction)")
print(f"  SAT solve:   {solve_time:.2f}s")
print(f"  SAT total:   {build_time + solve_time:.2f}s")
print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 SAT-INTEGRATED MINING COMPLETE                          ║")
print("╚══════════════════════════════════════════════════════════════╝")
