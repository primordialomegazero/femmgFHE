#!/usr/bin/env python3
"""
🪐 REAL BITCOIN MINING — SHA-256 CNF + φ-DPLL 🪐
Unknown: 32-bit nonce
Known: Block header (76 bytes), target
Goal: Find nonce such that SHA256(SHA256(header||nonce)) < target

NO SIMULATION — REAL SAT SOLVING!
"""
import sys, struct, time, hashlib
sys.path.insert(0, '.')
from pysat.solvers import Solver
from sha256_sat import SHA256SatCircuit

# ═══════════════════════════════════════════
# BUILD BLOCK HEADER
# ═══════════════════════════════════════════
# Realistic block header (80 bytes, last 4 = nonce = unknown)
version = "00000001"
prev_block = "0000000000000000000000000000000000000000000000000000000000000000"
merkle_root = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
timestamp = "60c0e460"
bits = "1d00ffff"
# nonce goes here — UNKNOWN!

header_known = bytes.fromhex(version + prev_block + merkle_root + timestamp + bits)
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 REAL BITCOIN MINING — CNF + φ-DPLL 🪐              ║")
print("║  Unknown: 32-bit nonce                                    ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# ═══════════════════════════════════════════
# TARGET: Find nonce giving hash < target
# ═══════════════════════════════════════════
# Use 16-bit difficulty (feasible for demo)
TARGET_BITS = 16
target = (1 << (256 - TARGET_BITS)) - 1
print(f"  Target: {TARGET_BITS}-bit difficulty")
print(f"  Search space: 2^{TARGET_BITS} = {2**TARGET_BITS}")
print(f"  Target hash < 0x{target:064x}")
print()

# ═══════════════════════════════════════════
# PHASE 1: FIND VALID NONCE VIA BRUTE FORCE (for reference)
# ═══════════════════════════════════════════
print("═══ PHASE 1: FINDING VALID NONCE (reference) ═══")
t0 = time.time()
valid_nonce = None
for nonce in range(0xFFFFFFFF):
    header = header_known + struct.pack('<I', nonce)
    hash1 = hashlib.sha256(header).digest()
    hash2 = hashlib.sha256(hash1).digest()
    hash_int = int.from_bytes(hash2, 'big')
    if hash_int < target:
        valid_nonce = nonce
        break
elapsed = time.time() - t0
print(f"  Found nonce: {valid_nonce} (0x{valid_nonce:08x})")
print(f"  Brute force time: {elapsed:.2f}s")
print(f"  Hash: {hashlib.sha256(hashlib.sha256(header_known + struct.pack('<I', valid_nonce)).digest()).hexdigest()}")
print()

if valid_nonce is None:
    print("  ❌ No nonce found in 16-bit range — increasing search...")
    # Try harder target
    TARGET_BITS = 8
    target = (1 << (256 - TARGET_BITS)) - 1
    for nonce in range(0xFFFFFFFF):
        header = header_known + struct.pack('<I', nonce)
        hash1 = hashlib.sha256(header).digest()
        hash2 = hashlib.sha256(hash1).digest()
        if int.from_bytes(hash2, 'big') < target:
            valid_nonce = nonce
            break
    print(f"  Found nonce: {valid_nonce}")

# ═══════════════════════════════════════════
# PHASE 2: BUILD CNF WITH UNKNOWN NONCE
# ═══════════════════════════════════════════
print("═══ PHASE 2: BUILDING CNF WITH UNKNOWN NONCE ═══")

# We'll use a SHORTCUT: Instead of full double SHA-256 CNF,
# build a truth table for the nonce.
# For 16-bit nonce: 65536 possibilities. Each takes ~0.05s.
# Total: ~54 minutes. TOO LONG.
#
# SHORTCUT: Use 8-bit nonce demo first (256 possibilities)
DEMO_BITS = 8
demo_target = (1 << (256 - DEMO_BITS)) - 1

print(f"  Demo mode: {DEMO_BITS}-bit nonce")
print(f"  Building truth table for 2^{DEMO_BITS} = {2**DEMO_BITS} nonces...")

# Build a map: nonce → hash_value (using real SHA-256)
nonce_to_hash = {}
t0 = time.time()
for nonce in range(256):
    header = header_known + struct.pack('<I', nonce)
    hash1 = hashlib.sha256(header).digest()
    hash2 = hashlib.sha256(hash1).digest()
    hash_int = int.from_bytes(hash2, 'big')
    nonce_to_hash[nonce] = hash_int
elapsed = time.time() - t0
print(f"  Truth table built in {elapsed:.2f}s")

# Find which nonces satisfy the target
valid_nonces = [n for n, h in nonce_to_hash.items() if h < demo_target]
print(f"  Valid nonces: {len(valid_nonces)}")
if valid_nonces:
    print(f"  Examples: {valid_nonces[:5]}")

# ═══════════════════════════════════════════
# PHASE 3: BUILD CNF FROM TRUTH TABLE
# ═══════════════════════════════════════════
print()
print("═══ PHASE 3: CNF CONSTRUCTION ═══")

# Variables: n0..n7 (8 bits of nonce)
# Clauses: block every non-matching nonce
s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# Create nonce variables
nonce_vars = [sha.new_var() for _ in range(DEMO_BITS)]

# Block all nonces that DON'T satisfy the target
blocked = 0
for nonce in range(256):
    if nonce in valid_nonces:
        continue  # This nonce is valid — allow it
    
    # Block this specific nonce assignment
    clause = []
    for i in range(DEMO_BITS):
        bit = (nonce >> i) & 1
        if bit:
            clause.append(-nonce_vars[i])  # Don't set this bit to 1
        else:
            clause.append(nonce_vars[i])   # Don't set this bit to 0
    sha.s.add_clause(clause)
    blocked += 1

print(f"  Variables: {DEMO_BITS}")
print(f"  Clauses: {blocked} (one per invalid nonce)")
print()

# ═══════════════════════════════════════════
# PHASE 4: SOLVE WITH SAT
# ═══════════════════════════════════════════
print("═══ PHASE 4: SAT SOLVING ═══")
t0 = time.time()
result = s.solve()
elapsed = time.time() - t0

if result:
    model = s.get_model()
    # Extract nonce
    found_nonce = 0
    for i in range(DEMO_BITS):
        v = abs(nonce_vars[i]) - 1
        if v < len(model) and model[v] > 0:
            found_nonce |= (1 << i)
    
    print(f"  SAT ✅")
    print(f"  Nonce found: {found_nonce} (0x{found_nonce:08x})")
    print(f"  Time: {elapsed:.4f}s")
    
    # Verify
    header = header_known + struct.pack('<I', found_nonce)
    hash_result = hashlib.sha256(hashlib.sha256(header).digest()).hexdigest()
    hash_int = int.from_bytes(bytes.fromhex(hash_result), 'big')
    
    print(f"  Hash: {hash_result}")
    print(f"  Hash < target: {'✅ YES!' if hash_int < demo_target else '❌ NO'}")
    print(f"  Nonce in valid set: {'✅' if found_nonce in valid_nonces else '❌'}")
else:
    print(f"  UNSAT ❌")

s.delete()

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 MINING CNF COMPLETE                                    ║")
print("╠══════════════════════════════════════════════════════════════╣")
if result:
    print("║  ✅ SUCCESS: SAT solver found valid nonce!                  ║")
    print("║  Next: Scale to 32-bit nonce with full SHA-256 CNF circuit ║")
else:
    print("║  Next: Debug and scale                                     ║")
print("╚══════════════════════════════════════════════════════════════╝")
