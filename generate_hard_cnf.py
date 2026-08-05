#!/usr/bin/env python3
import sys

def generate_pigeonhole(n_holes):
    """Pigeonhole: n+1 pigeons, n holes = UNSAT"""
    pigeons = n_holes + 1
    n_vars = pigeons * n_holes
    clauses = []
    
    # At least one hole per pigeon
    for p in range(pigeons):
        clause = [p * n_holes + h + 1 for h in range(n_holes)]
        clauses.append(clause)
    
    # No two pigeons in same hole
    for p1 in range(pigeons):
        for p2 in range(p1+1, pigeons):
            for h in range(n_holes):
                clauses.append([-(p1 * n_holes + h + 1), -(p2 * n_holes + h + 1)])
    
    return n_vars, clauses

if len(sys.argv) < 2:
    print("Usage: python3 generate_hard_cnf.py <n_holes>")
    sys.exit(1)

n = int(sys.argv[1])
n_vars, clauses = generate_pigeonhole(n)

print(f"p cnf {n_vars} {len(clauses)}")
for c in clauses:
    print(" ".join(map(str, c)) + " 0")
