#!/usr/bin/env python3
"""
🔍 DEBUG: Compare SHA-256 circuit vs Python reference — ROUND BY ROUND
Find exactly where the circuit diverges from the real SHA-256.
"""
import sys, os, struct

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

# ═══════════════════════════════════════════
# PURE PYTHON SHA-256 REFERENCE
# ═══════════════════════════════════════════
def sha256_ref(msg_bytes):
    """Pure Python SHA-256 — reference implementation"""
    msg = msg_bytes
    msg_len = len(msg) * 8
    msg += b'\x80'
    while (len(msg) * 8) % 512 != 448:
        msg += b'\x00'
    msg += struct.pack('>Q', msg_len)
    
    h = list(H0)
    for block_start in range(0, len(msg), 64):
        block = msg[block_start:block_start+64]
        w = list(struct.unpack('>16I', block))
        for t in range(16, 64):
            s0 = rotr(w[t-15], 7) ^ rotr(w[t-15], 18) ^ (w[t-15] >> 3)
            s1 = rotr(w[t-2], 17) ^ rotr(w[t-2], 19) ^ (w[t-2] >> 10)
            w.append((w[t-16] + s0 + w[t-7] + s1) & 0xFFFFFFFF)
        
        a, b, c, d, e, f, g, hh = h
        for t in range(64):
            S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)
            ch = (e & f) ^ (~e & g)
            temp1 = (hh + S1 + ch + K[t] + w[t]) & 0xFFFFFFFF
            S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)
            maj = (a & b) ^ (a & c) ^ (b & c)
            temp2 = (S0 + maj) & 0xFFFFFFFF
            
            hh = g; g = f; f = e
            e = (d + temp1) & 0xFFFFFFFF
            d = c; c = b; b = a
            a = (temp1 + temp2) & 0xFFFFFFFF
            
            # Print ROUND 1 values
            if t == 0:
                print(f"\n═══ ROUND 1 (REFERENCE) ═══")
                print(f"  W[0] = 0x{w[0]:08x}")
                print(f"  K[0] = 0x{K[0]:08x}")
                print(f"  Initial a = 0x{h[0]:08x}")
                print(f"  Initial e = 0x{h[4]:08x}")
                print(f"  Initial h = 0x{h[7]:08x}")
                print(f"  S1(e)  = 0x{S1:08x}")
                print(f"  ch     = 0x{ch:08x}")
                print(f"  temp1  = 0x{temp1:08x}")
                print(f"  S0(a)  = 0x{S0:08x}")
                print(f"  maj    = 0x{maj:08x}")
                print(f"  temp2  = 0x{temp2:08x}")
                print(f"  a_new  = 0x{a:08x}")
                print(f"  e_new  = 0x{e:08x}")
        
        h[0] = (h[0] + a) & 0xFFFFFFFF
        h[1] = (h[1] + b) & 0xFFFFFFFF
        h[2] = (h[2] + c) & 0xFFFFFFFF
        h[3] = (h[3] + d) & 0xFFFFFFFF
        h[4] = (h[4] + e) & 0xFFFFFFFF
        h[5] = (h[5] + f) & 0xFFFFFFFF
        h[6] = (h[6] + g) & 0xFFFFFFFF
        h[7] = (h[7] + hh) & 0xFFFFFFFF
    
    return b''.join(struct.pack('>I', x) for x in h)

# ═══════════════════════════════════════════
# TEST WITH SIMPLE MESSAGE
# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🔍 SHA-256 ROUND 1 — REFERENCE VALUES 🔍                 ║")
print("╚══════════════════════════════════════════════════════════════╝")

# Use a simple 64-byte message (no padding needed for 512-bit block)
msg = bytes.fromhex(
    "6162638000000000"  # "abc" + padding
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
    "0000000000000000"
    "0000000000000018"  # Length = 24 bits
)

# Verify message is exactly 64 bytes
print(f"  Message length: {len(msg)} bytes")
print(f"  Message hex: {msg.hex()}")
assert len(msg) == 64, "Message must be 64 bytes!"

# Compute reference
result = sha256_ref(b'abc')
expected = bytes.fromhex("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
print(f"\n  Reference SHA-256('abc'): {result.hex()}")
print(f"  Expected:                  {expected.hex()}")
print(f"  Match: {'✅' if result == expected else '❌'}")

# ═══════════════════════════════════════════
# NOW: Extract the W[0] value from the circuit
# ═══════════════════════════════════════════
print(f"\n═══ CIRCUIT DEBUG ═══")

sys.path.insert(0, '.')
try:
    from pysat.solvers import Solver
    from sha256_sat import SHA256SatCircuit
    
    s = Solver(name='minisat22')
    sha = SHA256SatCircuit(s)
    
    # Check the circuit's understanding of W[0]
    # The message is 64 bytes = 512 bits = 16 words
    msg_bits = []
    for byte in msg:
        for bit_pos in range(8):
            bit = (byte >> (7 - bit_pos)) & 1
            msg_bits.append(bit)
    
    print(f"  Message bits: {len(msg_bits)}")
    print(f"  First 32 bits (W[0]): {msg_bits[:32]}")
    
    # Build circuit for this message
    hash_bits = sha.hash_constant_message(msg.hex())
    
    print(f"  Circuit variables: {sha.next_var - 1}")
    print(f"  Hash bits: {len(hash_bits)}")
    print(f"  First 32 hash bits (circuit): {hash_bits[:32]}")
    
    # Solve
    result = s.solve()
    print(f"  SAT result (no constraints): {'✅' if result else '❌'}")
    
    if result:
        model = s.get_model()
        # Extract first 32 hash bits from model
        hval = 0
        for i in range(32):
            v = abs(hash_bits[i])
            if v <= len(model) and model[v-1] > 0:
                hval |= (1 << (31 - i))
        print(f"  Circuit hash bits[0:32]: 0x{hval:08x}")
        print(f"  Expected hash[0:32]:     0x{expected[0]:02x}{expected[1]:02x}{expected[2]:02x}{expected[3]:02x}")
    
    s.delete()
    
except ImportError as e:
    print(f"  Import error: {e}")
except Exception as e:
    print(f"  Error: {e}")
    import traceback
    traceback.print_exc()

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🔍 ROUND 1 DEBUG COMPLETE                                 ║")
print("╚══════════════════════════════════════════════════════════════╝")
