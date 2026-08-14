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

def fast_local_search_debug(clauses, state, num_vars, max_flips=200):
    flip_count = 0
    unsat_history = []
    
    current_unsat, unsat_clauses = count_unsat(clauses, state)
    unsat_history.append(current_unsat)
    
    random.seed(42)
    
    for flip in range(max_flips):
        if not unsat_clauses:
            current_unsat, unsat_clauses = count_unsat(clauses, state)
            unsat_history.append(current_unsat)
            if current_unsat == 0:
                return True, flip_count, unsat_history
            if not unsat_clauses:
                return True, flip_count, unsat_history
        
        # Pumili ng random unsat clause at random literal
        ci = unsat_clauses[random.randint(0, len(unsat_clauses)-1)]
        clause = clauses[ci]
        lit = clause[random.randint(0, len(clause)-1)]
        var = abs(lit) - 1
        
        # I-flip
        state[var] = 1 - state[var]
        flip_count += 1
        
        # I-update ang unsat clauses
        current_unsat, unsat_clauses = count_unsat(clauses, state)
        unsat_history.append(current_unsat)
        
        if current_unsat == 0:
            return True, flip_count, unsat_history
        
        # Early termination
        if flip > 200 and current_unsat > len(clauses) * 0.01:
            return False, flip_count, unsat_history
    
    return False, flip_count, unsat_history

# Test sa 50 vars
num_vars = 50
clauses = generate_formula(num_vars)

# Initial assignment
pos_count = [0] * num_vars
neg_count = [0] * num_vars
for clause in clauses:
    for lit in clause:
        var = abs(lit) - 1
        if lit > 0:
            pos_count[var] += 1
        else:
            neg_count[var] += 1

state = [1 if pos_count[v] >= neg_count[v] else -1 for v in range(num_vars)]

print("=" * 70)
print(f"DEBUG: CLAUSE FAST - {num_vars} vars")
print("=" * 70)

initial_unsat, initial_unsat_clauses = count_unsat(clauses, state)
print(f"\nInitial unsat: {initial_unsat}/{len(clauses)}")
print(f"Initial state: {state[:20]}...")

# Run local search with debug
found, flips, unsat_history = fast_local_search_debug(clauses, state, num_vars, 200)

print(f"\nLocal search result: {'SAT' if found else 'UNSAT'}")
print(f"Total flips: {flips}")
print(f"Unsat history (first 20): {unsat_history[:20]}")
print(f"Unsat history (last 10): {unsat_history[-10:]}")

# Analyze bakit UNSAT
print(f"\nAnalysis ng unsat history:")
print(f"  Starting unsat: {unsat_history[0]}")
print(f"  Min unsat reached: {min(unsat_history)}")
print(f"  Final unsat: {unsat_history[-1]}")
print(f"  Improvement: {unsat_history[0] - min(unsat_history)}")

# Check kung may periods ng improvement
improvements = []
for i in range(1, len(unsat_history)):
    if unsat_history[i] < unsat_history[i-1]:
        improvements.append((i, unsat_history[i-1], unsat_history[i]))

print(f"\nImprovements: {len(improvements)}")
for imp in improvements[:10]:
    print(f"  Flip {imp[0]}: {imp[1]} → {imp[2]}")

# Check ang unsat clauses structure
_, final_unsat_clauses = count_unsat(clauses, state)
print(f"\nFinal unsat clauses: {len(final_unsat_clauses)}")

if final_unsat_clauses:
    print(f"  First 5:")
    for ci in final_unsat_clauses[:5]:
        clause = clauses[ci]
        print(f"    Clause {ci}: {clause}")
        for lit in clause:
            var = abs(lit) - 1
            val = state[var] == 1
            is_true = (lit > 0 and val) or (lit < 0 and not val)
            print(f"      Lit {lit}: var={var}, state={'TRUE' if state[var]==1 else 'FALSE'}, is_true={is_true}")

# Try ang mas maraming flips
print(f"\nTry mas maraming flips (1000):")
state2 = [1 if pos_count[v] >= neg_count[v] else -1 for v in range(num_vars)]
found2, flips2, unsat_history2 = fast_local_search_debug(clauses, state2, num_vars, 1000)
print(f"  Result: {'SAT' if found2 else 'UNSAT'}")
print(f"  Flips: {flips2}")
print(f"  Min unsat: {min(unsat_history2)}")
print(f"  Final unsat: {unsat_history2[-1]}")

# Try ang different random seeds
print(f"\nDifferent seeds (200 flips each):")
for seed in range(10):
    state3 = [1 if pos_count[v] >= neg_count[v] else -1 for v in range(num_vars)]
    random.seed(seed)
    
    current_unsat, unsat_clauses = count_unsat(clauses, state3)
    flip_count = 0
    
    for flip in range(200):
        if not unsat_clauses:
            current_unsat, unsat_clauses = count_unsat(clauses, state3)
            if current_unsat == 0:
                break
        
        ci = unsat_clauses[random.randint(0, len(unsat_clauses)-1)]
        clause = clauses[ci]
        lit = clause[random.randint(0, len(clause)-1)]
        var = abs(lit) - 1
        
        state3[var] = 1 - state3[var]
        flip_count += 1
        
        current_unsat, unsat_clauses = count_unsat(clauses, state3)
        
        if current_unsat == 0:
            break
    
    print(f"  Seed {seed}: {'SAT' if current_unsat == 0 else 'UNSAT'} (flips={flip_count}, unsat={current_unsat})")

