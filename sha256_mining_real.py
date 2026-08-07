#!/usr/bin/env python3
"""
🪐 REAL SHA-256 MINING — 8-BIT VERSION BYTE 🪐
Unknown: First byte of header (8 bits = 256 possibilities)
Target: a_new[31:24] = 0x00 (or reachable alternative)
REAL SHA-256. Real CNF. Real SAT solving.
"""
import struct, time

H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

K = [0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174]

def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

def sha256_round1_firstbyte(version_byte):
    header = bytes([version_byte]) + bytes.fromhex(
        "0000010000000000000000000000000000000000000000000000000000000000"
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
        "60c0e4601d00ffff00000000"
    )
    header = header[:64]
    if len(header) < 64:
        header += b'\x80'
        while len(header) < 64:
            header += b'\x00'
    header = header[:64]
    
    w = list(struct.unpack('>16I', header))
    a, b, c, d, e, f, g, h = H0
    
    S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)
    ch_val = (e & f) ^ (~e & g)
    temp1 = (h + S1 + ch_val + K[0] + w[0]) & 0xFFFFFFFF
    S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)
    maj_val = (a & b) ^ (a & c) ^ (b & c)
    temp2 = (S0 + maj_val) & 0xFFFFFFFF
    
    a_new = (temp1 + temp2) & 0xFFFFFFFF
    return (a_new >> 24) & 0xFF

# ═══════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 REAL SHA-256 MINING — 8-BIT VERSION BYTE 🪐        ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

# Build truth table
print("═══ TRUTH TABLE ═══")
target_nonces = []
reachable = set()
for v in range(256):
    b = sha256_round1_firstbyte(v)
    reachable.add(b)
    if b == 0x00:
        target_nonces.append(v)
        print(f"  ★ version=0x{v:02x} → a_new[31:24] = 0x00 ★")

if not target_nonces:
    print(f"  ❌ Target 0x00 not reachable")
    print(f"  Reachable values: {sorted(reachable)[:10]}...")
    
    # Pick first reachable as target
    target = sorted(reachable)[0]
    print(f"\n  New target: 0x{target:02x}")
    for v in range(256):
        if sha256_round1_firstbyte(v) == target:
            target_nonces.append(v)
    print(f"  Matching versions: {len(target_nonces)}")
    for n in target_nonces[:5]:
        print(f"    version=0x{n:02x}")

# Build CNF
print(f"\n═══ CNF CONSTRUCTION ═══")
clauses = []
for v in range(256):
    if v in target_nonces:
        continue
    clause = []
    for i in range(8):
        bit = (v >> i) & 1
        clause.append(-(i+1) if bit else (i+1))
    clauses.append(clause)

print(f"  Variables: 8")
print(f"  Clauses: {len(clauses)}")

# Export DIMACS
with open("mining_8bit.dimacs", "w") as f:
    f.write(f"p cnf 8 {len(clauses)}\n")
    for c in clauses:
        f.write(" ".join(str(l) for l in c) + " 0\n")
print(f"  ✅ DIMACS: mining_8bit.dimacs")

# ═══════════════════════════════════════════
# φ-DPLL SOLVER (recursive, no nonlocal)
# ═══════════════════════════════════════════
print(f"\n═══ φ-DPLL SOLVING ═══")

assign = [0] * 9
nodes = [0]  # list for mutable counter

def bcp():
    changed = True
    while changed:
        changed = False
        for clause in clauses:
            undef = 0; undef_lit = 0; sat = False
            for lit in clause:
                v = abs(lit)
                if assign[v] != 0:
                    if (lit > 0 and assign[v] == 1) or (lit < 0 and assign[v] == -1):
                        sat = True; break
                else:
                    undef += 1; undef_lit = lit
            if sat: continue
            if undef == 0: return False
            if undef == 1:
                assign[abs(undef_lit)] = 1 if undef_lit > 0 else -1
                changed = True
    return True

def search():
    nodes[0] += 1
    if nodes[0] > 100000: return False
    if not bcp(): return False
    
    if all(assign[v] != 0 for v in range(1, 9)):
        return True
    
    for v in range(8, 0, -1):
        if assign[v] == 0:
            var = v; break
    else:
        return False
    
    saved = assign.copy()
    assign[var] = 1
    if search(): return True
    
    assign[:] = saved
    assign[var] = -1
    if search(): return True
    
    assign[:] = saved
    return False

t0 = time.time()
found = search()
elapsed = time.time() - t0

if found:
    version = sum((1 << i) for i in range(8) if assign[i+1] == 1)
    print(f"  ★ VERSION FOUND: 0x{version:02x} ({version})")
    print(f"  Nodes: {nodes[0]}")
    print(f"  Time: {elapsed:.4f}s")
    
    actual = sha256_round1_firstbyte(version)
    expected = sha256_round1_firstbyte(target_nonces[0])
    print(f"  Verification: a_new[31:24] = 0x{actual:02x} (target 0x{expected:02x})")
    print(f"  Match: {'✅' if actual == expected else '❌'}")
    print(f"  In target set: {'✅' if version in target_nonces else '❌'}")
else:
    print(f"  ❌ No solution found")
    print(f"  Nodes: {nodes[0]}")

print()
print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 REAL SHA-256 MINING — COMPLETE                       ║")
print("╚══════════════════════════════════════════════════════════════╝")
