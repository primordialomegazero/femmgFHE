import random

PHI = (1 + 5**0.5) / 2
PSI = (1 - 5**0.5) / 2

def generate_formula(num_vars, seed=42):
    random.seed(seed)
    num_clauses = num_vars * 3
    clauses = []
    for c in range(num_clauses):
        clause = []
        for k in range(3):
            var = random.randint(1, num_vars)
            if random.random() > 0.5:
                var = -var
            clause.append(var)
        clauses.append(clause)
    return clauses

def count_unsat(clauses, state):
    unsat = 0
    unsat_clauses = []
    for ci, clause in enumerate(clauses):
        sat = False
        for lit in clause:
            var = abs(lit) - 1
            val = state[var] == 1
            if (lit > 0 and val) or (lit < 0 and not val):
                sat = True
                break
        if not sat:
            unsat += 1
            unsat_clauses.append(ci)
    return unsat, unsat_clauses

def phase1_entanglement(clauses, num_vars):
    pos_count = [0] * num_vars
    neg_count = [0] * num_vars
    clause_count = [0] * num_vars
    
    for clause in clauses:
        for lit in clause:
            var = abs(lit) - 1
            clause_count[var] += 1
            if lit > 0:
                pos_count[var] += 1
            else:
                neg_count[var] += 1
    
    state = []
    for v in range(num_vars):
        ratio = neg_count[v] / clause_count[v] if clause_count[v] > 0 else 0.5
        if abs(ratio - 0.618) < 0.1:
            state.append(1 if v % 2 == 0 else -1)
        elif ratio > 0.618:
            state.append(-1)
        else:
            state.append(1)
    
    return state

def phase2_collapse(clauses, state, num_vars):
    collapse_count = 0
    for iteration in range(10):
        changed = False
        for clause in clauses:
            sat = False
            false_literals = []
            
            for lit in clause:
                var = abs(lit) - 1
                val = state[var] == 1
                if (lit > 0 and val) or (lit < 0 and not val):
                    sat = True
                    break
                else:
                    false_literals.append(lit)
            
            if not sat and false_literals:
                lit = false_literals[0]
                var = abs(lit) - 1
                state[var] = 1 - state[var]
                collapse_count += 1
                changed = True
                break
        
        if not changed:
            break
    
    return collapse_count

# Test with 50 vars
num_vars = 50
clauses = generate_formula(num_vars)

print("=" * 70)
print(f"PIPELINE DEBUG - {num_vars} vars")
print("=" * 70)

# Phase 1
state = phase1_entanglement(clauses, num_vars)
unsat, unsat_clauses = count_unsat(clauses, state)
print(f"\nPhase 1 (Entanglement):")
print(f"  Unsat clauses: {unsat}/{len(clauses)}")
print(f"  State: {state[:20]}...")

# Phase 2
collapse_count = phase2_collapse(clauses, state, num_vars)
unsat_after_p2, unsat_clauses_p2 = count_unsat(clauses, state)
print(f"\nPhase 2 (Black Hole Collapse):")
print(f"  Collapses performed: {collapse_count}")
print(f"  Unsat clauses: {unsat_after_p2}/{len(clauses)}")
print(f"  Improvement: {unsat - unsat_after_p2} clauses")

# Analyze remaining unsat clauses
if unsat_clauses_p2:
    print(f"\n  Remaining unsat clauses:")
    for ci in unsat_clauses_p2[:5]:
        clause = clauses[ci]
        print(f"    Clause {ci}: {clause}")
        for lit in clause:
            var = abs(lit) - 1
            val = state[var] == 1
            is_true = (lit > 0 and val) or (lit < 0 and not val)
            print(f"      Lit {lit}: var={var}, state={'TRUE' if state[var]==1 else 'FALSE'}, is_true={is_true}")

# Check kung may pattern sa unsat clauses
print(f"\n  Pattern analysis ng unsat clauses:")
all_neg = 0
all_pos = 0
mixed = 0
for ci in unsat_clauses_p2:
    clause = clauses[ci]
    if all(l < 0 for l in clause):
        all_neg += 1
    elif all(l > 0 for l in clause):
        all_pos += 1
    else:
        mixed += 1

print(f"    All negative: {all_neg}")
print(f"    All positive: {all_pos}")
print(f"    Mixed: {mixed}")

# Check variable occurrences in unsat clauses
from collections import Counter
var_counter = Counter()
for ci in unsat_clauses_p2:
    for lit in clauses[ci]:
        var_counter[abs(lit) - 1] += 1

print(f"\n  Top variables sa unsat clauses:")
for var, count in var_counter.most_common(10):
    print(f"    Var {var+1}: {count} occurrences, current={state[var]}")

# Phase 3 simulation
print(f"\nPhase 3 (Local Search) simulation:")
current_unsat = unsat_after_p2
print(f"  Starting unsat: {current_unsat}")

# Simulate 10 flips
for flip in range(10):
    best_var = None
    best_unsat = current_unsat + 1
    
    for v in range(num_vars):
        state[v] = 1 - state[v]
        new_unsat, _ = count_unsat(clauses, state)
        state[v] = 1 - state[v]
        
        if new_unsat < best_unsat:
            best_unsat = new_unsat
            best_var = v
    
    if best_var is None:
        print(f"  Flip {flip}: No improvement possible")
        break
    
    state[best_var] = 1 - state[best_var]
    current_unsat = best_unsat
    print(f"  Flip {flip}: Var {best_var+1} → unsat={current_unsat}")
    
    if current_unsat == 0:
        print(f"  SOLVED after {flip+1} flips!")
        break

print(f"\nFinal unsat: {current_unsat}")
