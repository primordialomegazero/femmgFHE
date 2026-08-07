#!/usr/bin/env python3
"""
🔍 DEBUG: Force message bits to 'abc', check if hash output matches.
This tests whether the circuit computes SHA-256 correctly.
"""
import sys, os, struct, time

sys.path.insert(0, '.')
from pysat.solvers import Solver
from sha256_sat import SHA256SatCircuit

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🔍 FORCE MESSAGE, CHECK HASH 🔍                           ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# Build properly padded 'abc' (512 bits = 64 bytes)
msg = b'abc'
msg_len = len(msg) * 8
padded = msg + b'\x80'
while len(padded) * 8 % 512 != 448:
    padded += b'\x00'
padded += struct.pack('>Q', msg_len)

msg_hex = padded.hex()
print(f"  Padded message: {msg_hex[:40]}...")
print(f"  Message length: {len(padded)} bytes = 512 bits")
print()

# Expected hash
import hashlib
expected = hashlib.sha256(b'abc').digest()
print(f"  Expected hash: {expected.hex()}")
print()

# Build circuit
s = Solver(name='minisat22')
sha = SHA256SatCircuit(s)

# ═══════════════════════════════════════════
# THE KEY: hash_constant_message forces message bits as constants
# AND returns the hash variable IDs
# ═══════════════════════════════════════════
print("  Building circuit with message='abc'...")
t0 = time.time()
hash_bits = sha.hash_constant_message(msg_hex)
elapsed = time.time() - t0
print(f"  Built in {elapsed:.1f}s, {sha.next_var - 1} variables")
print()

# Solve
print("  Solving...")
t0 = time.time()
result = s.solve()
elapsed = time.time() - t0
print(f"  Result: {'SAT ✅' if result else 'UNSAT ❌'} ({elapsed:.1f}s)")
print()

if result:
    model = s.get_model()
    
    # Extract ALL 256 hash bits from the model
    hash_bytes = []
    for byte_idx in range(32):
        byte_val = 0
        for bit_idx in range(8):
            bit_pos = byte_idx * 8 + bit_idx
            v = hash_bits[bit_pos]
            var_idx = abs(v) - 1
            if var_idx < len(model) and model[var_idx] > 0:
                byte_val |= (1 << (7 - bit_idx))
        hash_bytes.append(byte_val)
    
    circuit_hash = bytes(hash_bytes)
    print(f"  Circuit hash: {circuit_hash.hex()}")
    print(f"  Expected:     {expected.hex()}")
    print(f"  Match: {'✅ YES!' if circuit_hash == expected else '❌ NO'}")
    
    if circuit_hash != expected:
        # Show first differing bytes
        for i in range(32):
            if circuit_hash[i] != expected[i]:
                print(f"\n  First diff at byte {i}:")
                print(f"    Circuit:  {circuit_hash[i]:02x} ({circuit_hash[i]:08b})")
                print(f"    Expected: {expected[i]:02x} ({expected[i]:08b})")
                break
else:
    print("  UNSAT — circuit has contradictions even with forced message!")

s.delete()
