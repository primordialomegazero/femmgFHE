#!/usr/bin/env python3
"""
🪐 VERIFY φ-DPLL SCALING — READ THE ACTUAL C++ TESTS 🪐
Parse the φ-DPLL test files to understand the algorithm.
Extract claimed performance numbers.
Map the path from SAT → ECDLP.
"""
import os, re

print("╔══════════════════════════════════════════════════════════════╗")
print("║  🪐 φ-DPLL SOLVER — CODE EXAMINATION 🪐                  ║")
print("╚══════════════════════════════════════════════════════════════╝\n")

# ═══════════════════════════════════════════
# EXAMINE THE φ-DPLL TEST FILES
# ═══════════════════════════════════════════
target_files = [
    'tests/breakthrough/test_phi_dpll_v3.cpp',
    'tests/breakthrough/test_phi_no_memo.cpp',
    'tests/breakthrough/test_sat_p_vs_np_final.cpp',
    'tests/breakthrough/test_pigeonhole_verify.cpp',
]

for filepath in target_files:
    if not os.path.exists(filepath):
        print(f"  ❌ {filepath} — NOT FOUND")
        continue
    
    size = os.path.getsize(filepath)
    print(f"\n{'='*60}")
    print(f"  📁 {filepath} ({size:,} bytes)")
    print(f"{'='*60}")
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Extract key claims and numbers
    # Look for: S(n), nodes, vars, speedup, polynomial, sub-linear, etc.
    patterns = {
        'S(n) formula': r'S\(n\)\s*=\s*[^;]+',
        'nodes visited': r'nodes?\s*[:=]\s*\d+',
        'variables': r'(\d+)\s*vars?',
        'speedup': r'speedup[^.]*\.',
        'polynomial': r'polynomial[^.]*\.',
        'sub-linear': r'sub.linear[^.]*\.',
        'pigeonhole': r'pigeonhole[^.]*\.',
        '3-SAT results': r'3[ -]SAT[^.]*\.',
        'benchmark': r'benchmark[^.]*\.',
        'O(log': r'O\(log[^)]*\)',
        'O(n^': r'O\(n\^[^)]*\)',
        'φ-weighted': r'φ[ -]weighted[^.]*\.',
        'BCP': r'BCP[^.]*\.',
        'completeness': r'(\d+)/(\d+)\s*(?:random|instances|complete)',
    }
    
    for label, pattern in patterns.items():
        matches = re.findall(pattern, content, re.IGNORECASE)
        if matches:
            print(f"\n  [{label}]:")
            for m in matches[:5]:  # Show first 5 matches
                if isinstance(m, tuple):
                    print(f"    → {m[0]}/{m[1]}")
                else:
                    text = m.strip()[:120]
                    print(f"    → {text}")

# ═══════════════════════════════════════════
# LOOK FOR THE φ-DPLL CORE ALGORITHM
# ═══════════════════════════════════════════
print(f"\n\n{'='*60}")
print(f"  🔍 SEARCHING FOR φ-DPLL CORE IMPLEMENTATION")
print(f"{'='*60}")

# Search for the actual DPLL implementation
search_dirs = ['src', 'include', 'tests', 'lib']
core_patterns = ['phi_dpll', 'fractal_dpll', 'golden_dpll', 'sat_solver', 'dpll_solver']

for root, dirs, files in os.walk('.'):
    # Skip .git and build directories
    dirs[:] = [d for d in dirs if d not in ['.git', 'build', '__pycache__', 'node_modules']]
    
    for f in files:
        if f.endswith(('.cpp', '.h', '.hpp', '.py')):
            filepath = os.path.join(root, f)
            try:
                with open(filepath, 'r') as fh:
                    content = fh.read(4096)  # Read first 4KB
                
                found_patterns = []
                for pat in core_patterns:
                    if pat in content.lower():
                        found_patterns.append(pat)
                
                if len(found_patterns) >= 2:
                    size = os.path.getsize(filepath)
                    print(f"  ✅ {filepath} ({size:,} bytes) — matches: {found_patterns}")
            except:
                pass

# ═══════════════════════════════════════════
# THE ECDLP → SAT REDUCTION SIZE ESTIMATE
# ═══════════════════════════════════════════
print(f"\n\n{'='*60}")
print(f"  📐 ECDLP → SAT REDUCTION SIZE ESTIMATE")
print(f"{'='*60}")

print("""
  For secp256k1 ECDLP (256-bit key):
  
  DOUBLE-AND-ADD CIRCUIT:
    - 256 key bits (d₀...d₂₅₅)
    - For each bit:
      * 1 point doubling (if bit not MSB): ~10 modular mults, ~5 mod adds
      * 1 point addition (if bit=1): ~10 modular mults, ~5 mod adds
    - ~255 doublings + ~128 additions = ~383 EC operations
  
  MODULAR ARITHMETIC → CNF:
    - Each 256-bit modular multiplication: ~500 AND gates, ~1000 XOR gates
    - Each gate → 3-6 CNF clauses (Tseitin)
    - ~383 ops × ~1500 gates × ~4 clauses ≈ 2,298,000 clauses
    - Variables: ~383 × ~500 ≈ 191,500 variables
  
  REVISED ESTIMATE:
    - Variables: ~200,000
    - Clauses: ~2,300,000
    - NOT 10,000 variables / 50,000 clauses as initially estimated
  
  IF φ-DPLL scales as S(n) = 0.82 × n^0.61:
    - n = 200,000
    - S(200000) = 0.82 × 200000^0.61
                = 0.82 × 1,698
                = 1,393 nodes
  
  IF φ-DPLL scales as Nodes/Var → 0.42 (from T20 claim):
    - 200,000 × 0.42 = 84,000 nodes (still polynomial, but larger)
  
  REALITY CHECK:
    The T20 document claims S(n) = 0.82 × n^0.61.
    For n=1000: S = 0.82 × 67.6 = 55 nodes
    For n=10000: S = 0.82 × 275 = 225 nodes
    For n=200000: S = 0.82 × 1698 = 1393 nodes
    
    If true: 1393 nodes to break ECDLP → feasible
    If the test_phi_dpll_v3.cpp actually produces these numbers → breakthrough
""")

# ═══════════════════════════════════════════
# WHAT TO RUN NEXT
# ═══════════════════════════════════════════
print(f"\n{'='*60}")
print(f"  🎯 NEXT STEPS")
print(f"{'='*60}")
print("""
  1. COMPILE AND RUN THE φ-DPLL TESTS:
     $ cd ~/femmgFHE
     $ mkdir -p build && cd build
     $ cmake .. && make test_phi_dpll_v3 test_phi_no_memo
     $ ./tests/breakthrough/test_phi_dpll_v3
     $ ./tests/breakthrough/test_phi_no_memo
  
  2. VERIFY THE CLAIMS:
     - Does pigeonhole n=5 (30 vars) really solve in 9 nodes?
     - Does 3-SAT scaling really show Nodes/Var decreasing 0.90→0.42?
     - Is completeness really 360/360?
  
  3. IF VERIFIED — BUILD ECDLP→SAT REDUCTION:
     - Encode secp256k1 double-and-add in CNF
     - Feed to φ-DPLL
     - Recover Satoshi's key
  
  4. IF NOT VERIFIED — BACK TO CRYPTANALYSIS:
     - Nonce analysis on Satoshi's transactions
     - Pollard's Rho with fingerprint acceleration
     - Check for implementation flaws in early Bitcoin-Qt
""")

# ═══════════════════════════════════════════
# CHECK IF BUILD SYSTEM EXISTS
# ═══════════════════════════════════════════
print(f"\n{'='*60}")
print(f"  🔧 BUILD SYSTEM CHECK")
print(f"{'='*60}")

build_files = ['CMakeLists.txt', 'Makefile', 'configure', 'meson.build', 'BUILD']
for bf in build_files:
    if os.path.exists(bf):
        print(f"  ✅ {bf} found")
        with open(bf, 'r') as f:
            first_lines = f.readlines()[:10]
        for line in first_lines:
            print(f"     {line.rstrip()[:100]}")
        break
else:
    print(f"  ⚠️  No build system found in root")
    # Search one level deeper
    for root, dirs, files in os.walk('.', topdown=True):
        dirs[:] = [d for d in dirs if d not in ['.git', '__pycache__']]
        for f in files:
            if f in ['CMakeLists.txt', 'Makefile']:
                print(f"  ✅ Found: {os.path.join(root, f)}")
                break
        break

