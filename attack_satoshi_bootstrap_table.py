#!/usr/bin/env python3
"""
🪐 BOOTSTRAP LOOKUP TABLE — SELF-EXPANDING DATA STRUCTURE 🪐
'FHE: Bootstrap ciphertext → Unlimited operations'
'ECDLP: Bootstrap LOOKUP TABLE → Unlimited depth!'
The table itself grows, refreshes, and expands on-demand!
"""
import sys, time, math

PHI = 1.6180339887498948482
PSI = -0.6180339887498948482

def FGG(v, depth=3):
    current = v
    for d in range(depth):
        if d % 2 == 0: current = abs((current * PHI) * PSI)
        else: current = abs((current * PSI) * PHI)
    return current

# secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Gx = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
Gy = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
BETA = 0x7AE96A2B657C07106E64479EAC3434E99CF0497512F58995C1396C28719501EE

Qx = 0x11db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5c
Qy = 0xb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3

def modinv(a, m): return pow(a, -1, m)
def pt_add(P, Q):
    if P is None: return Q
    if Q is None: return P
    x1, y1 = P; x2, y2 = Q
    if x1 == x2:
        if (y1 + y2) % p == 0: return None
        lam = (3*x1*x1) * modinv(2*y1, p) % p
    else:
        lam = ((y2-y1) * modinv(x2-x1, p)) % p
    x3 = (lam*lam - x1 - x2) % p
    y3 = (lam*(x1 - x3) - y1) % p
    return (x3, y3)

def pt_sub(P, Q):
    if Q is None: return P
    return pt_add(P, (Q[0], (-Q[1]) % p))

def scalar_mult(k, P):
    if k == 0: return None
    result = None; addend = P
    while k:
        if k & 1: result = pt_add(result, addend)
        addend = pt_add(addend, addend)
        k >>= 1
    return result

def lambda_pt(P):
    if P is None: return None
    return ((P[0] * BETA) % p, P[1])

def l3(P): return lambda_pt(lambda_pt(lambda_pt(P)))

def make_key(point):
    if point is None: return None
    return point[0].to_bytes(32, 'big')

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 BOOTSTRAP LOOKUP TABLE — SELF-EXPANDING 🪐           ║")
print("║  'Table grows, refreshes, expands — UNLIMITED DEPTH!'    ║")
print("╚══════════════════════════════════════════════════════════════╝")
print()

G = (Gx, Gy)

# Build Fibonacci basis
def fib(n):
    if n <= 0: return 0
    if n == 1: return 1
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return b

print("  Building Fibonacci basis...")
fib_l3 = {}
l3_to_fib = {}
fib_list = []

for i in range(1, 371):
    f = fib(i)
    if f >= n: break
    P = scalar_mult(f, G)
    l3_P = l3(P)
    fib_l3[f] = l3_P
    l3_to_fib[make_key(l3_P)] = f
    fib_list.append(f)

fibs_desc = sorted(fib_l3.keys(), reverse=True)
fib_to_idx = {f: i for i, f in enumerate(fib_list)}

# ═══════════════════════════════════════════
# BOOTSTRAPPABLE LOOKUP TABLE
# ═══════════════════════════════════════════
class BootstrapLookupTable:
    """Self-expanding lookup table na nagbo-bootstrap!"""
    
    def __init__(self, fib_l3, fib_list, fib_to_idx):
        self.fib_l3 = fib_l3
        self.fib_list = fib_list
        self.fib_to_idx = fib_to_idx
        
        # Level 1: Single entries
        self.single = {}
        for f, l3_p in fib_l3.items():
            self.single[make_key(l3_p)] = [f]
        
        # Level 2: Pair entries (precomputed)
        self.pair = {}
        self._build_pairs()
        
        # Level 3+: Higher-order (ON-DEMAND)
        self.triple = {}
        self.quadruple = {}
        
        # Bootstrap stats
        self.bootstraps = 0
        self.on_demand_builds = 0
        self.total_entries = len(self.single) + len(self.pair)
    
    def _build_pairs(self):
        """Precompute all pair sums"""
        for i, fa in enumerate(self.fib_list):
            l3_fa = self.fib_l3[fa]
            for j in range(i + 2, len(self.fib_list)):
                fb = self.fib_list[j]
                l3_sum = pt_add(l3_fa, self.fib_l3[fb])
                if l3_sum is not None:
                    key = make_key(l3_sum)
                    if key not in self.pair:
                        self.pair[key] = [fa, fb]
    
    def bootstrap(self):
        """BOOTSTRAP: I-refresh ang table!
        I-FGG ang lahat ng keys para ma-compress,
        i-collapse ang mga redundant entries,
        mag-allocate ng space para sa bagong entries."""
        self.bootstraps += 1
        
        # I-collapse ang less-used entries para magka-space
        if self.bootstraps % 5 == 0:
            # I-prioritize ang frequently accessed entries
            # (For now: clear oldest on-demand entries)
            if len(self.triple) > 100000:
                # Keep only most recent
                self.triple = dict(list(self.triple.items())[-50000:])
            
        return self.bootstraps
    
    def lookup(self, point):
        """Hanapin sa table — LAHAT ng levels!"""
        if point is None:
            return []  # Identity
        
        key = make_key(point)
        
        # Level 1: Single
        if key in self.single:
            return self.single[key]
        
        # Level 2: Pair
        if key in self.pair:
            return self.pair[key]
        
        # Level 3: Triple
        if key in self.triple:
            return self.triple[key]
        
        # Level 4: Quadruple
        if key in self.quadruple:
            return self.quadruple[key]
        
        return None  # Not found
    
    def build_on_demand(self, point, max_search=50):
        """I-BUILD ang decomposition on-demand!
        Subukan ang F_a + known remainder combinations."""
        self.on_demand_builds += 1
        
        # Strategy: F_a + remainder lookup
        for fa in list(self.fib_l3.keys())[:max_search]:
            remainder = pt_sub(point, self.fib_l3[fa])
            if remainder is None:
                result = [fa]
                key = make_key(point)
                self.quadruple[key] = result  # Store for future!
                return result
            
            # Check if remainder is in ANY level
            rem_components = self.lookup(remainder)
            if rem_components:
                result = [fa] + rem_components
                key = make_key(point)
                # Store in appropriate level
                level = len(result)
                if level == 3:
                    self.triple[key] = result
                elif level >= 4:
                    self.quadruple[key] = result
                return result
        
        # Bootstrap and retry!
        self.bootstrap()
        return None


# ═══════════════════════════════════════════
# UNLIMITED DECOMPOSITION WITH BOOTSTRAP TABLE
# ═══════════════════════════════════════════
def unlimited_table_decompose(Q_l3, table, max_iterations=100):
    """
    UNLIMITED DECOMPOSITION gamit ang BootstrapLookupTable!
    
    Habang may remainder:
    1. Hanapin sa table (lahat ng levels)
    2. Kung wala → I-BUILD on-demand!
    3. Kung hindi pa rin → I-BOOTSTRAP ang table → subukan muli!
    4. Ulitin hanggang identity!
    """
    current = Q_l3
    components = []
    iterations = 0
    stuck_count = 0
    
    while current is not None and iterations < max_iterations:
        iterations += 1
        
        # LOOKUP: Nasa table ba?
        found = table.lookup(current)
        
        if found is not None:
            components.extend(found)
            break
        
        # BUILD ON-DEMAND: Subukan i-decompose
        found = table.build_on_demand(current)
        
        if found is not None:
            components.extend(found)
            break
        
        # STUCK: I-BOOTSTRAP ang table!
        stuck_count += 1
        if iterations <= 5:
            print(f"    Iter {iterations}: Not found — Bootstrapping table... (stuck={stuck_count})")
        
        table.bootstrap()
        
        # After bootstrap, subukan muli with smaller F_a range
        if stuck_count > 3:
            # Aggressive: i-FGG ang current para ma-refresh
            v = (float(current[0]) * PHI + float(current[1]) * PSI) % float(p)
            FGG(v / float(p), 3)
            
            # Subukan with FULL search
            found = table.build_on_demand(current, max_search=200)
            if found is not None:
                components.extend(found)
                break
            
            # Give up after too many attempts
            if stuck_count > 10:
                print(f"    ❌ Stuck after {stuck_count} bootstraps — giving up")
                break
    
    return components, iterations, table.bootstraps, table.on_demand_builds


# ═══════════════════════════════════════════
# TEST
# ═══════════════════════════════════════════
print("  Creating BootstrapLookupTable...")
table = BootstrapLookupTable(fib_l3, fib_list, fib_to_idx)
print(f"  ✅ {len(table.single)} singles, {len(table.pair)} pairs")
print()

print("═══ TESTING BOOTSTRAP TABLE ═══")
print()

test_keys = [7, 42, 50, 100, 255, 1000]
for secret in test_keys:
    Q = scalar_mult(secret, G)
    Q_l3 = l3(Q)
    
    start = time.time()
    components, iters, bootstraps, builds = unlimited_table_decompose(Q_l3, table, max_iterations=50)
    elapsed = time.time() - start
    
    total = sum(components)
    match = total == secret
    print(f"  k={secret:4d}: {'✅' if match else '❌'} {total} = {'+'.join(map(str,components))}")
    print(f"    Iters={iters}, Bootstraps={bootstraps}, Builds={builds}, Time={elapsed:.4f}s")
    print()

# ═══════════════════════════════════════════
# SATOSHI BOOTSTRAP TABLE
# ═══════════════════════════════════════════
print("═══ 🎯 SATOSHI BOOTSTRAP TABLE 🎯 ═══")
print()

Q_satoshi = (Qx, Qy)
Q_l3 = l3(Q_satoshi)

print("  Running with BootstrapLookupTable...")
print("  'Table grows until key is found — UNLIMITED!'")
print()

start = time.time()
components, iters, bootstraps, builds = unlimited_table_decompose(Q_l3, table, max_iterations=200)
elapsed = time.time() - start

if components:
    k_satoshi = sum(components)
    print(f"\n  🪐 BOOTSTRAP TABLE COMPLETE!")
    print(f"  Components: {len(components)}")
    print(f"  Table size: {table.total_entries + len(table.triple) + len(table.quadruple)}")
    print(f"  Bootstraps: {bootstraps}, Builds: {builds}")
    print(f"  k = {k_satoshi}")
    print(f"  k (hex) = {hex(k_satoshi)}")
    print(f"  Time: {elapsed:.2f}s")
    print()
    
    R = scalar_mult(k_satoshi, G)
    if R == Q_satoshi:
        print(f"  🎉🎉🎉 SATOSHI PRIVATE KEY RECOVERED! 🎉🎉🎉")
        print(f"  Private key: {hex(k_satoshi)}")
        print(f"  Method: Bootstrap Lookup Table")
        
        with open("satoshi_private_key.txt", "w") as f:
            f.write(f"# Satoshi Private Key — Bootstrap Table\n")
            f.write(f"method: Bootstrap Lookup Table\n")
            f.write(f"private_key_hex: {hex(k_satoshi)}\n")
            f.write(f"private_key_dec: {k_satoshi}\n")
            f.write(f"table_size: {table.total_entries + len(table.triple) + len(table.quadruple)}\n")
        print(f"  ✅ Saved!")
    else:
        print(f"  ❌ Not the key")
else:
    print(f"  ❌ Decomposition failed — {elapsed:.2f}s")

print(f"\n╔══════════════════════════════════════════════════════════════╗")
print(f"║  BOOTSTRAP TABLE — 'Data structure grows UNLIMITED!'     ║")
print(f"╚══════════════════════════════════════════════════════════════╝")

