#!/usr/bin/env python3
"""
🔐 SHA-256 SAT CIRCUIT v2 — FIXED Wire Order
"""
import sys, os, time, struct

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

class SHA256SatCircuit:
    def __init__(self, solver):
        self.s = solver; self.bits = 32
        self.next_var = 1
    
    def new_var(self): v = self.next_var; self.next_var += 1; return v
    def new_vars(self, n): return [self.new_var() for _ in range(n)]
    
    def XOR(self, a, b):
        c = self.new_var()
        self.s.add_clause([-a, -b, -c]); self.s.add_clause([-a, b, c])
        self.s.add_clause([a, -b, c]); self.s.add_clause([a, b, -c])
        return c
    def AND(self, a, b):
        c = self.new_var()
        self.s.add_clause([-a, -b, c]); self.s.add_clause([a, -c]); self.s.add_clause([b, -c])
        return c
    def OR(self, a, b):
        c = self.new_var()
        self.s.add_clause([a, b, -c]); self.s.add_clause([-a, c]); self.s.add_clause([-b, c])
        return c
    def NOT(self, a):
        c = self.new_var()
        self.s.add_clause([a, c]); self.s.add_clause([-a, -c])
        return c
    def force_true(self, v): self.s.add_clause([v])
    def force_false(self, v): self.s.add_clause([-v])
    
    def constant_word(self, value):
        w = self.new_vars(self.bits)
        for i in range(self.bits):
            if (value >> (self.bits-1-i)) & 1: self.force_true(w[i])
            else: self.force_false(w[i])
        return w
    
    def XOR_word(self, A, B):
        return [self.XOR(A[i], B[i]) for i in range(self.bits)]
    def AND_word(self, A, B):
        return [self.AND(A[i], B[i]) for i in range(self.bits)]
    def NOT_word(self, A):
        return [self.NOT(A[i]) for i in range(self.bits)]
    def CH_word(self, X, Y, Z):
        return self.XOR_word(self.AND_word(X, Y), self.AND_word(self.NOT_word(X), Z))
    def MAJ_word(self, X, Y, Z):
        xy = self.AND_word(X, Y); xz = self.AND_word(X, Z); yz = self.AND_word(Y, Z)
        return self.XOR_word(self.XOR_word(xy, xz), yz)
    def ROTR_word(self, X, n):
        result = self.new_vars(self.bits)
        for i in range(self.bits):
            src_idx = (i - n) % self.bits
            self.force_false(self.XOR(result[i], X[src_idx]))
        return result
    def SHR_word(self, X, n):
        result = self.new_vars(self.bits)
        for i in range(self.bits):
            if i + n < self.bits: self.force_false(self.XOR(result[i], X[i + n]))
            else: self.force_false(result[i])
        return result
    
    def SIGMA0_word(self, X):
        r2 = self.ROTR_word(X, 2); r13 = self.ROTR_word(X, 13); r22 = self.ROTR_word(X, 22)
        return self.XOR_word(self.XOR_word(r2, r13), r22)
    def SIGMA1_word(self, X):
        r6 = self.ROTR_word(X, 6); r11 = self.ROTR_word(X, 11); r25 = self.ROTR_word(X, 25)
        return self.XOR_word(self.XOR_word(r6, r11), r25)
    def GAMMA0_word(self, X):
        r7 = self.ROTR_word(X, 7); r18 = self.ROTR_word(X, 18); s3 = self.SHR_word(X, 3)
        return self.XOR_word(self.XOR_word(r7, r18), s3)
    def GAMMA1_word(self, X):
        r17 = self.ROTR_word(X, 17); r19 = self.ROTR_word(X, 19); s10 = self.SHR_word(X, 10)
        return self.XOR_word(self.XOR_word(r17, r19), s10)
    
    def ADD_word(self, A, B):
        S = self.new_vars(self.bits)
        carry = self.new_var(); self.force_false(carry)
        for i in range(self.bits-1, -1, -1):
            axb = self.XOR(A[i], B[i]); s = self.XOR(axb, carry)
            ab = self.AND(A[i], B[i]); xc = self.AND(axb, carry)
            carry = self.OR(ab, xc); S[i] = s
        return S
    def ADD_many(self, words):
        result = words[0]
        for w in words[1:]: result = self.ADD_word(result, w)
        return result
    
    def compress(self, msg_bits, rounds=64):
        """FIXED: Correct wire order for state updates"""
        initial_state = [self.constant_word(H0[i]) for i in range(8)]
        
        # Parse message
        W = []
        for i in range(16):
            W.append(msg_bits[i*32:(i+1)*32])
        
        # Message schedule
        for t in range(16, 64):
            g0 = self.GAMMA0_word(W[t-15])
            g1 = self.GAMMA1_word(W[t-2])
            new_w = self.ADD_many([W[t-16], g0, W[t-7], g1])
            W.append(new_w)
        
        # Working variables
        a, b, c, d, e, f, g, h = initial_state
        
        # 64 rounds
        for t in range(rounds):
            T1 = self.ADD_many([h, self.SIGMA1_word(e), self.CH_word(e, f, g),
                                self.constant_word(K[t]), W[t]])
            T2 = self.ADD_word(self.SIGMA0_word(a), self.MAJ_word(a, b, c))
            
            # FIXED: compute all NEW values first, then assign all at once
            new_a = self.ADD_word(T1, T2)
            new_e = self.ADD_word(d, T1)
            new_b = a
            new_c = b
            new_d = c
            new_f = e
            new_g = f
            new_h = g
            
            # Assign all at once
            a, b, c, d, e, f, g, h = new_a, new_b, new_c, new_d, new_e, new_f, new_g, new_h
        
        # Final addition with initial state
        final = []
        final_state = [a, b, c, d, e, f, g, h]
        for i in range(8):
            final.append(self.ADD_word(final_state[i], initial_state[i]))
        
        # Flatten
        hash_bits = []
        for word in final:
            hash_bits.extend(word)
        
        return hash_bits

# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
if __name__ == '__main__':
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║  🔐 SHA-256 SAT CIRCUIT v2 — FIXED Wire Order               ║")
    print("╚══════════════════════════════════════════════════════════════╝\n")
    
    try:
        from pysat.solvers import Solver
        
        print("═══ Test: SHA-256('abc') — Full 64 Rounds ═══")
        s = Solver(name='minisat22')
        sha = SHA256SatCircuit(s)
        
        # Build padded message for "abc"
        msg = b"abc"
        msg_len = len(msg) * 8
        padded = msg + b'\x80'
        while (len(padded) * 8) % 512 != 448:
            padded += b'\x00'
        padded += struct.pack('>Q', msg_len)
        
        msg_bits = []
        for byte in padded:
            for i in range(8):
                v = sha.new_var()
                if (byte >> (7 - i)) & 1: sha.force_true(v)
                else: sha.force_false(v)
                msg_bits.append(v)
        
        hash_bits = sha.compress(msg_bits, rounds=64)
        
        expected = bytes.fromhex("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
        
        # Force first 64 bits
        for word_idx in range(2):
            word_val = int.from_bytes(expected[word_idx*4:(word_idx+1)*4], 'big')
            for i in range(32):
                bit_pos = word_idx * 32 + i
                if (word_val >> (31 - i)) & 1:
                    s.add_clause([hash_bits[bit_pos]])
                else:
                    s.add_clause([-hash_bits[bit_pos]])
        
        print(f"  Variables: {sha.next_var - 1}")
        print(f"  Vars/round: {(sha.next_var - 1) / 64:.0f}")
        print(f"  Solving...")
        
        start = time.time()
        result = s.solve()
        elapsed = time.time() - start
        
        if result:
            print(f"  SAT ✅ | Time: {elapsed:.3f}s")
            print(f"  🎉 SHA-256 CIRCUIT WORKS!")
        else:
            print(f"  UNSAT ❌ | Time: {elapsed:.3f}s")
        
        s.delete()
        
    except ImportError:
        print("  PySAT not available. Install: pip install python-sat")
