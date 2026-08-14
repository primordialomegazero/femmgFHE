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

def build_graph(clauses, num_vars):
    var_to_clauses = [[] for _ in range(num_vars)]
    clause_to_vars = [[] for _ in clauses]
    var_pos_count = [0] * num_vars
    var_neg_count = [0] * num_vars
    
    for ci, clause in enumerate(clauses):
        for lit in clause:
            var = abs(lit) - 1
            var_to_clauses[var].append(ci)
            clause_to_vars[ci].append(var)
            if lit > 0:
                var_pos_count[var] += 1
            else:
                var_neg_count[var] += 1
    
    return var_to_clauses, clause_to_vars, var_pos_count, var_neg_count

def count_unsat(clauses, state):
    unsat = 0
    for clause in clauses:
        sat = False
        for lit in clause:
            var = abs(lit) - 1
            val = state[var] == 1
            if (lit > 0 and val) or (lit < 0 and not val):
                sat = True
                break
        if not sat:
            unsat += 1
    return unsat

def find_collapse_set(clauses, state, graph, num_vars):
    var_to_clauses, clause_to_vars, var_pos_count, var_neg_count = graph
    
    # Hanapin ang unsatisfied clauses
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
            unsat_clauses.append(ci)
    
    if not unsat_clauses:
        return []
    
    # Hanapin ang variable impact
    var_impact = {}
    for ci in unsat_clauses:
        for var in clause_to_vars[ci]:
            var_impact[var] = var_impact.get(var, 0) + 1
    
    # Sort by impact
    impacts = sorted(var_impact.items(), key=lambda x: x[1], reverse=True)
    
    # Top 20%
    collapse_set = []
    top_count = max(1, int(len(impacts) * 0.2))
    for i in range(min(top_count, len(impacts))):
        collapse_set.append(impacts[i][0])
    
    return collapse_set, unsat_clauses, impacts

# Test with 50 vars
num_vars = 50
clauses = generate_formula(num_vars)
graph = build_graph(clauses, num_vars)

print("=" * 70)
print(f"GRAPH COLLAPSE DEBUG - {num_vars} vars")
print("=" * 70)

# Initial state
var_to_clauses, clause_to_vars, var_pos_count, var_neg_count = graph
state = []
for v in range(num_vars):
    if var_neg_count[v] > var_pos_count[v]:
        state.append(0)
    else:
        state.append(1)

initial_unsat = count_unsat(clauses, state)
print(f"\nInitial unsat: {initial_unsat}")
print(f"Initial state: {state}")

# Track over steps
prev_state = None
prev_collapse_set = None
cycle_detected = False
cycle_start = 0

for step in range(50):
    collapse_set, unsat_clauses, impacts = find_collapse_set(clauses, state, graph, num_vars)
    
    if not collapse_set:
        print(f"\nStep {step}: SOLVED!")
        break
    
    unsat_count = count_unsat(clauses, state)
    
    if step < 10:
        print(f"\nStep {step}: unsat={unsat_count}, collapse_set_size={len(collapse_set)}")
        print(f"  Collapse set: {collapse_set[:5]}...")
        print(f"  Unsatisfied clauses: {len(unsat_clauses)}")
        if impacts:
            print(f"  Top impacts: {impacts[:5]}")
    
    # Check for cycle
    if collapse_set == prev_collapse_set:
        if not cycle_detected:
            cycle_detected = True
            cycle_start = step
            print(f"\n  CYCLE DETECTED at step {step}!")
            print(f"  Same collapse set: {collapse_set}")
    elif cycle_detected:
        print(f"  Cycle broken at step {step} (length: {step - cycle_start})")
        cycle_detected = False
    
    # Apply collapse
    for var in collapse_set:
        state[var] = 1 - state[var]
    
    prev_collapse_set = collapse_set.copy()
    prev_state = state.copy()

# Analyze why stuck
print("\n" + "=" * 70)
print("STUCK ANALYSIS")
print("=" * 70)

final_unsat = count_unsat(clauses, state)
print(f"\nFinal unsat: {final_unsat}")
print(f"Final state: {state[:20]}...")

# Check kung may solution na malapit
print("\nChecking kung may solution sa 1-2 flips...")
best_1flip = (None, final_unsat)
best_2flip = (None, final_unsat)

for i in range(num_vars):
    test_state = state.copy()
    test_state[i] = 1 - test_state[i]
    unsat = count_unsat(clauses, test_state)
    if unsat < best_1flip[1]:
        best_1flip = ((i,), unsat)

for i in range(num_vars):
    for j in range(i+1, num_vars):
        test_state = state.copy()
        test_state[i] = 1 - test_state[i]
        test_state[j] = 1 - test_state[j]
        unsat = count_unsat(clauses, test_state)
        if unsat < best_2flip[1]:
            best_2flip = ((i, j), unsat)

print(f"Best 1-flip: {best_1flip}")
print(f"Best 2-flip: {best_2flip}")

# Count variable occurrences
print("\nVariable occurrence counts (first 10 vars):")
for v in range(10):
    print(f"  Var {v+1}: pos={var_pos_count[v]}, neg={var_neg_count[v]}")

# Check clause types
print("\nClause types (positive vs negative literals):")
all_pos = sum(1 for c in clauses if all(l > 0 for l in c))
all_neg = sum(1 for c in clauses if all(l < 0 for l in c))
mixed = len(clauses) - all_pos - all_neg
print(f"  All positive: {all_pos}")
print(f"  All negative: {all_neg}")
print(f"  Mixed: {mixed}")
