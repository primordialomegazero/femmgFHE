import numpy as np

PHI = (1 + np.sqrt(5)) / 2
PSI = (1 - np.sqrt(5)) / 2

def generate_formula(num_vars, seed=42):
    np.random.seed(seed)
    num_clauses = num_vars * 3
    clauses = []
    for c in range(num_clauses):
        clause = []
        for k in range(3):
            var = np.random.randint(1, num_vars + 1)
            if np.random.random() > 0.5:
                var = -var
            clause.append(var)
        clauses.append(clause)
    return clauses

def build_soft_entanglements(clauses, num_vars):
    ents = []
    var_occurrences = [[] for _ in range(num_vars)]
    
    for clause in clauses:
        for lit1 in clause:
            for lit2 in clause:
                if lit1 == lit2:
                    continue
                var1 = abs(lit1) - 1
                var2 = abs(lit2) - 1
                var_occurrences[var1].append(var2)
    
    pair_count = {}
    for v1 in range(num_vars):
        for v2 in var_occurrences[v1]:
            if v1 < v2:
                key = f"{v1}_{v2}"
                pair_count[key] = pair_count.get(key, 0) + 1
    
    for key, count in pair_count.items():
        parts = key.split("_")
        v1 = int(parts[0])
        v2 = int(parts[1])
        
        ent = {
            'var1': v1,
            'var2': v2,
            'weight': min(count / 10.0, 1.0)
        }
        ents.append(ent)
    
    return ents

def soft_propagate(clauses, probs, ents, num_vars, max_iter=20):
    propagation_count = 0
    
    for iter in range(max_iter):
        new_probs = probs.copy()
        
        # Update batay sa clause satisfaction
        for clause in clauses:
            prob_unsat = 1.0
            for lit in clause:
                var = abs(lit) - 1
                prob_true = probs[var]
                prob_lit_true = prob_true if lit > 0 else (1 - prob_true)
                prob_unsat *= (1 - prob_lit_true)
            
            if prob_unsat > 0.3:
                for lit in clause:
                    var = abs(lit) - 1
                    if lit > 0:
                        new_probs[var] += 0.1 * prob_unsat
                    else:
                        new_probs[var] -= 0.1 * prob_unsat
        
        # Update batay sa entanglements
        for ent in ents:
            avg = (new_probs[ent['var1']] + new_probs[ent['var2']]) / 2.0
            new_probs[ent['var1']] += ent['weight'] * (avg - new_probs[ent['var1']])
            new_probs[ent['var2']] += ent['weight'] * (avg - new_probs[ent['var2']])
        
        # Clamp
        for v in range(num_vars):
            if new_probs[v] > 0.95:
                new_probs[v] = 0.95
            if new_probs[v] < 0.05:
                new_probs[v] = 0.05
        
        # Check convergence
        change = sum(abs(new_probs[v] - probs[v]) for v in range(num_vars))
        
        probs = new_probs
        propagation_count += 1
        
        if change < 0.01:
            break
    
    return probs, propagation_count

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

# Test sa 50 vars
num_vars = 50
clauses = generate_formula(num_vars)
ents = build_soft_entanglements(clauses, num_vars)

print("=" * 70)
print(f"SOFT ENTANGLEMENT DEBUG - {num_vars} vars")
print("=" * 70)
print(f"\nEntanglements found: {len(ents)}")
print(f"Clauses: {len(clauses)}")
print(f"PHI = {PHI:.6f}, PSI = {PSI:.6f}")
print(f"PHI * PSI = {PHI * PSI:.6f}")

# Initial probabilities
probs = np.array([0.618] * num_vars)
print(f"\nInitial probs: {probs[:10]}...")
print(f"Mean: {np.mean(probs):.4f}, Std: {np.std(probs):.4f}")

# Run propagation
probs, prop_count = soft_propagate(clauses, probs, ents, num_vars)
print(f"\nAfter propagation ({prop_count} iterations):")
print(f"Probs: {probs[:10]}...")
print(f"Mean: {np.mean(probs):.4f}, Std: {np.std(probs):.4f}")
print(f"Min: {np.min(probs):.4f}, Max: {np.max(probs):.4f}")

# Convert to binary
state = [1 if p > 0.5 else -1 for p in probs]
unsat = count_unsat(clauses, state)
print(f"\nBinary assignment:")
print(f"State: {state[:20]}...")
print(f"Unsat clauses: {unsat}/{len(clauses)}")

# Analyze unsat clauses
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

print(f"\nUnsat clauses analysis:")
print(f"  Count: {len(unsat_clauses)}")
if unsat_clauses:
    print(f"  First 5:")
    for ci in unsat_clauses[:5]:
        clause = clauses[ci]
        print(f"    Clause {ci}: {clause}")
        for lit in clause:
            var = abs(lit) - 1
            val = state[var] == 1
            is_true = (lit > 0 and val) or (lit < 0 and not val)
            print(f"      Lit {lit}: var={var}, prob={probs[var]:.3f}, state={'TRUE' if state[var]==1 else 'FALSE'}, is_true={is_true}")

# Check kung may solution na malapit
print(f"\nChecking kung may solution sa 1-3 flips:")
best_1flip = (None, unsat)
best_2flip = (None, unsat)
best_3flip = (None, unsat)

for i in range(num_vars):
    test_state = state.copy()
    test_state[i] = 1 - test_state[i]
    u = count_unsat(clauses, test_state)
    if u < best_1flip[1]:
        best_1flip = ((i,), u)

for i in range(num_vars):
    for j in range(i+1, num_vars):
        test_state = state.copy()
        test_state[i] = 1 - test_state[i]
        test_state[j] = 1 - test_state[j]
        u = count_unsat(clauses, test_state)
        if u < best_2flip[1]:
            best_2flip = ((i, j), u)

for i in range(num_vars):
    for j in range(i+1, num_vars):
        for k in range(j+1, num_vars):
            test_state = state.copy()
            test_state[i] = 1 - test_state[i]
            test_state[j] = 1 - test_state[j]
            test_state[k] = 1 - test_state[k]
            u = count_unsat(clauses, test_state)
            if u < best_3flip[1]:
                best_3flip = ((i, j, k), u)

print(f"Best 1-flip: {best_1flip}")
print(f"Best 2-flip: {best_2flip}")
print(f"Best 3-flip: {best_3flip}")

# Check exhaustive para sa 50 vars (2^50 ay malaki, so sample lang)
print(f"\nRandom sampling (1000 states):")
np.random.seed(123)
random_unsat = []
for _ in range(1000):
    random_state = [1 if np.random.random() > 0.5 else -1 for _ in range(num_vars)]
    random_unsat.append(count_unsat(clauses, random_state))

print(f"  Min unsat: {min(random_unsat)}")
print(f"  Mean unsat: {np.mean(random_unsat):.1f}")
print(f"  Std unsat: {np.std(random_unsat):.1f}")

# Check kung may solution
print(f"\nBest unsat from random: {min(random_unsat)}")
if min(random_unsat) == 0:
    print("  May solution!")
else:
    print(f"  Walang solution na nakita (pero baka mayroon)")
