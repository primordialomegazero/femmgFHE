#!/usr/bin/env python3
"""Debug: Find why SHA-256 circuit is UNSAT"""
import sys, os, time, struct

# Copy the constants and functions from sha256_sat.py
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

def rotr(x, n): return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

# Import sha256_sat
sys.path.insert(0, '.')
from sha256_sat import SHA256SatCircuit

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🔍 DEBUG SHA-256 CIRCUIT 🔍                               ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# Test: Just 1 round, no constraints — is it SAT?
try:
    from pysat.solvers import Solver
    s = Solver(name='minisat22')
    sha = SHA256SatCircuit(s)
    
    # Build message: 512 bits of 'abc' properly padded
    msg = b'abc'
    msg_len = len(msg) * 8
    padded = msg + b'\x80'
    while len(padded) * 8 % 512 != 448:
        padded += b'\x00'
    padded += struct.pack('>Q', msg_len)
    
    print(f"  Message: {msg}")
    print(f"  Padded length: {len(padded)} bytes = {len(padded)*8} bits")
    print()
    
    # Convert to hex
    msg_hex = padded.hex()
    print(f"  Padded hex: {msg_hex[:40]}...")
    print()
    
    # Try: just build the circuit, no constraints
    print("  Building circuit (no constraints)...")
    hash_bits = sha.hash_constant_message(msg_hex[:128])  # First 64 bytes = 512 bits
    
    print(f"  Variables: {sha.next_var - 1}")
    print(f"  Hash bits: {len(hash_bits)}")
    print()
    
    # Solve WITHOUT forcing hash bits
    print("  Solving (no output constraints)...")
    result = s.solve()
    print(f"  Result: {'SAT ✅' if result else 'UNSAT ❌'}")
    
    if result:
        # Get the hash bits from the model
        model = s.get_model()
        print(f"  Model size: {len(model)}")
        
        # Extract first 32 hash bits
        hash_val = 0
        for i in range(min(32, len(hash_bits))):
            v = hash_bits[i]
            if v > 0 and v <= len(model):
                if model[v-1] > 0:
                    hash_val |= (1 << (31 - i))
        print(f"  First 32 hash bits: 0x{hash_val:08x}")
        
        # Expected
        import hashlib
        expected = hashlib.sha256(b'abc').hexdigest()
        print(f"  Expected hash: {expected}")
        
    s.delete()
    
except ImportError:
    print("  PySAT not available")
except Exception as e:
    print(f"  Error: {e}")
    import traceback
    traceback.print_exc()
