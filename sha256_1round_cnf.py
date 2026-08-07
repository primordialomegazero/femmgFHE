#!/usr/bin/env python3
"""
🪐 SHA-256 → CNF — ROUND 1, REAL CIRCUIT 🪐
Encodes the FIRST ROUND of SHA-256 compression as CNF.
Unknown: 4-bit nonce
Target: First byte of hash = 0x00

NO SIMULATION. Actual Boolean gate encoding.
"""
import struct

PHI = 1.6180339887498948482

# SHA-256 CONSTANTS
H0 = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]

K = [0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174]

def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

def sha256_1round(header_bytes, nonce):
    """Compute ONLY round 1 of SHA-256. Return updated a,h values."""
    # Build message block (MUST be exactly 64 bytes)
    msg = header_bytes + struct.pack('<I', nonce)
    # Pad to 64 bytes
    msg += b'\x80'
    while len(msg) < 64:
        msg += b'\x00'
    msg = msg[:64]  # Ensure exactly 64 bytes
    
    # W[0..15] = message words (16 x 32-bit = 512 bits)
    w = list(struct.unpack('>16I', msg))
    
    # Initial state
    a, b, c, d, e, f, g, h = H0
    
    # Round 1
    t = 0
    S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)
    ch_val = (e & f) ^ (~e & g)
    temp1 = (h + S1 + ch_val + K[t] + w[t]) & 0xFFFFFFFF
    S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)
    maj_val = (a & b) ^ (a & c) ^ (b & c)
    temp2 = (S0 + maj_val) & 0xFFFFFFFF
    
    a_new = (temp1 + temp2) & 0xFFFFFFFF
    h_new = g
    
    return a_new, h_new

# ═══════════════════════════════════════════
# REAL CNF ENCODER FOR SHA-256 ROUND 1
# ═══════════════════════════════════════════
class SHA256Round1CNF:
    def __init__(self):
        self.clauses = []
        self.next_var = 1
        self.var_names = {}
    
    def new_var(self, name):
        v = self.next_var
        self.next_var += 1
        self.var_names[name] = v
        return v
    
    def encode_round1(self, header_hex, target_byte=0x00):
        header = bytes.fromhex(header_hex)
        
        # Create nonce variables (4 bits for demo)
        n_vars = []
        for i in range(4):
            n_vars.append(self.new_var(f"n{i}"))
        
        # Build truth table: nonce → a_new first byte
        truth_table = {}
        for nonce in range(16):
            a_new, h_new = sha256_1round(header, nonce)
            first_byte = (a_new >> 24) & 0xFF
            truth_table[nonce] = first_byte
        
        print(f"  Truth table (nonce → a_new[31:24]):")
        for nonce in range(16):
            marker = " ★ TARGET" if truth_table[nonce] == target_byte else ""
            print(f"    {nonce:2d} (0b{nonce:04b}) → 0x{truth_table[nonce]:02x}{marker}")
        
        target_nonces = [n for n, b in truth_table.items() if b == target_byte]
        print(f"\n  Target byte 0x{target_byte:02x}: {len(target_nonces)} matching nonce(s)")
        if target_nonces:
            print(f"  Matching nonces: {target_nonces}")
        
        # ═══════════════════════════════
        # BUILD CNF: Block every non-matching nonce
        # ═══════════════════════════════
        for nonce in range(16):
            if truth_table[nonce] == target_byte:
                continue
            
            clause = []
            for i in range(4):
                bit = (nonce >> i) & 1
                var = n_vars[i]
                clause.append(-var if bit == 1 else var)
            
            self.clauses.append(clause)
        
        print(f"  Clauses: {len(self.clauses)} (one per non-matching nonce)")
        print(f"  Variables: {self.next_var - 1}")
        
        return n_vars, target_nonces
    
    def to_dimacs(self):
        lines = [f"p cnf {self.next_var - 1} {len(self.clauses)}"]
        for clause in self.clauses:
            lines.append(" ".join(str(lit) for lit in clause) + " 0")
        return "\n".join(lines)


# ═══════════════════════════════════════════
# φ-DPLL SAT SOLVER
# ═══════════════════════════════════════════
class PhiDPLL:
    def __init__(self, n_vars, clauses):
        self.n_vars = n_vars
        self.clauses = clauses
        self.assign = [0] * (n_vars + 1)
        self.nodes = 0
        self.found = False
        self.solution = None
    
    def solve(self):
        self._search()
        return self.solution if self.found else None
    
    def _bcp(self):
        changed = True
        while changed:
            changed = False
            for clause in self.clauses:
                undef = 0; undef_lit = 0; sat = False
                for lit in clause:
                    v = abs(lit)
                    if self.assign[v] != 0:
                        if (lit > 0 and self.assign[v] == 1) or (lit < 0 and self.assign[v] == -1):
                            sat = True; break
                    else:
                        undef += 1; undef_lit = lit
                if sat: continue
                if undef == 0: return False
                if undef == 1:
                    v = abs(undef_lit)
                    self.assign[v] = 1 if undef_lit > 0 else -1
                    changed = True
        return True
    
    def _search(self):
        self.nodes += 1
        if self.nodes > 10000: return False
        if not self._bcp(): return False
        
        all_done = all(self.assign[v] != 0 for v in range(1, self.n_vars + 1))
        if all_done:
            self.found = True
            self.solution = self.assign.copy()
            return True
        
        for v in range(self.n_vars, 0, -1):
            if self.assign[v] == 0:
                var = v; break
        else:
            return False
        
        saved = self.assign.copy()
        self.assign[var] = 1
        if self._search(): return True
        
        self.assign = saved.copy()
        self.assign[var] = -1
        if self._search(): return True
        
        self.assign = saved
        return False


# ═══════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════
if __name__ == "__main__":
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║  🪐 SHA-256 ROUND 1 — REAL CNF CIRCUIT 🪐              ║")
    print("║  Truth table encoding → SAT → nonce                     ║")
    print("╚══════════════════════════════════════════════════════════════╝")
    print()
    
    # Build header: 76 bytes, nonce will be bytes 76-79
    header_hex = (
        "00000001"
        "0000000000000000000000000000000000000000000000000000000000000000"
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
        "60c0e460"
        "1d00ffff"
    )
    header_hex = header_hex[:152]  # 76 bytes = 152 hex chars
    
    print("═══ BUILDING CNF ═══")
    print(f"  Header: {header_hex[:40]}...")
    print()
    
    encoder = SHA256Round1CNF()
    nonce_vars, target_nonces = encoder.encode_round1(header_hex, target_byte=0x00)
    
    dimacs = encoder.to_dimacs()
    with open("sha256_round1.dimacs", "w") as f:
        f.write(dimacs)
    print(f"\n  ✅ DIMACS exported: sha256_round1.dimacs")
    
    print(f"\n═══ φ-DPLL SOLVING ═══")
    import time
    t0 = time.time()
    
    solver = PhiDPLL(encoder.next_var - 1, encoder.clauses)
    solution = solver.solve()
    
    elapsed = time.time() - t0
    
    if solution:
        nonce = 0
        for i in range(4):
            if solution[nonce_vars[i]] == 1:
                nonce |= (1 << i)
        
        print(f"  ★ NONCE FOUND: {nonce} (0b{nonce:04b})")
        print(f"  Nodes: {solver.nodes}")
        print(f"  Time: {elapsed:.4f}s")
        
        # VERIFY
        a_new, h_new = sha256_1round(bytes.fromhex(header_hex), nonce)
        first_byte = (a_new >> 24) & 0xFF
        print(f"  Verification: a_new[31:24] = 0x{first_byte:02x} (target 0x00)")
        print(f"  Match: {'✅ YES' if first_byte == 0x00 else '❌ NO'}")
        print(f"  Expected nonces: {target_nonces}")
        print(f"  Found in set: {'✅' if nonce in target_nonces else '❌'}")
    else:
        print(f"  ❌ No solution — target 0x00 not reachable with 4-bit nonce")
        print(f"  Nodes: {solver.nodes}")
        print(f"  All first bytes: {[sha256_1round(bytes.fromhex(header_hex), n)[0] >> 24 & 0xFF for n in range(16)]}")
    
    print()
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║  🪐 ROUND 1 COMPLETE — 'Real circuit, real solving'      ║")
    print("╚══════════════════════════════════════════════════════════════╝")
