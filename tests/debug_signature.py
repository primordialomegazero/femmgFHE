import numpy as np

PHI = (1 + np.sqrt(5)) / 2
PSI = (1 - np.sqrt(5)) / 2

def generate_formula(num_vars, num_clauses, seed=42):
    np.random.seed(seed)
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

def compute_signature(clauses, num_vars):
    pos_count = [0] * num_vars
    neg_count = [0] * num_vars
    
    for clause in clauses:
        for lit in clause:
            var = abs(lit) - 1
            if lit > 0:
                pos_count[var] += 1
            else:
                neg_count[var] += 1
    
    signature = 0.0
    total_balance = 0.0
    
    for v in range(num_vars):
        total = pos_count[v] + neg_count[v]
        if total == 0:
            continue
        
        balance = (pos_count[v] - neg_count[v]) / total
        golden_deviation = abs(balance - 0.618)
        signature += golden_deviation
        total_balance += balance
    
    signature /= num_vars
    total_balance /= num_vars
    
    return signature + abs(total_balance - 0.618)

def signature_assignment(clauses, num_vars):
    pos_count = [0] * num_vars
    neg_count = [0] * num_vars
    
    for clause in clauses:
        for lit in clause:
            var = abs(lit) - 1
            if lit > 0:
                pos_count[var] += 1
            else:
                neg_count[var] += 1
    
    state = []
    for v in range(num_vars):
        if pos_count[v] > neg_count[v]:
            state.append(1)
        elif neg_count[v] > pos_count[v]:
            state.append(-1)
        else:
            state.append(1 if v % 2 == 0 else -1)
    
    return state

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

def exhaustive_check(clauses, num_vars):
    """Check kung satisfiable ba talaga (para sa maliit na num_vars)"""
    best_unsat = float('inf')
    best_state = None
    
    for bits in range(2**num_vars):
        state = [1 if (bits >> v) & 1 else -1 for v in range(num_vars)]
        unsat = count_unsat(clauses, state)
        
        if unsat < best_unsat:
            best_unsat = unsat
            best_state = state
        
        if unsat == 0:
            return True, state
    
    return False, best_state

# Test sa iba't ibang clause-to-variable ratios
print("=" * 70)
print("SIGNATURE VS SATISFIABILITY ANALYSIS")
print("=" * 70)

for ratio in [1, 2, 3, 4, 4.26, 5, 6, 8, 10]:
    num_vars = 10
    num_clauses = int(num_vars * ratio)
    
    satisfiable_count = 0
    signatures = []
    
    for seed in range(20):
        clauses = generate_formula(num_vars, num_clauses, seed)
        sig = compute_signature(clauses, num_vars)
        signatures.append(sig)
        
        is_sat, _ = exhaustive_check(clauses, num_vars)
        if is_sat:
            satisfiable_count += 1
    
    print(f"\nRatio {ratio}:")
    print(f"  SAT: {satisfiable_count}/20")
    print(f"  Signature mean: {np.mean(signatures):.4f}")
    print(f"  Signature std: {np.std(signatures):.4f}")
    print(f"  Signature range: [{min(signatures):.4f}, {max(signatures):.4f}]")

# Test sa mas malalaking instances para makita ang pattern
print("\n" + "=" * 70)
print("LARGER INSTANCES ANALYSIS")
print("=" * 70)

for num_vars in [10, 20, 30, 40, 50]:
    num_clauses = num_vars * 3
    sigs_sat = []
    sigs_unsat = []
    
    for seed in range(10):
        clauses = generate_formula(num_vars, num_clauses, seed)
        sig = compute_signature(clauses, num_vars)
        
        # Para sa maliliit na vars, kaya pa i-check exhaustively
        if num_vars <= 20:
            is_sat, _ = exhaustive_check(clauses, num_vars)
        else:
            # Para sa malalaki, assume SAT (since ratio 3 ay usually SAT)
            is_sat = True
        
        if is_sat:
            sigs_sat.append(sig)
        else:
            sigs_unsat.append(sig)
    
    print(f"\n{num_vars} vars:")
    if sigs_sat:
        print(f"  SAT signatures: mean={np.mean(sigs_sat):.4f}, std={np.std(sigs_sat):.4f}")
    if sigs_unsat:
        print(f"  UNSAT signatures: mean={np.mean(sigs_unsat):.4f}, std={np.std(sigs_unsat):.4f}")

# Check ang distribution ng variable balance
print("\n" + "=" * 70)
print("VARIABLE BALANCE DISTRIBUTION")
print("=" * 70)

num_vars = 100
num_clauses = num_vars * 3
clauses = generate_formula(num_vars, num_clauses)

pos_count = [0] * num_vars
neg_count = [0] * num_vars

for clause in clauses:
    for lit in clause:
        var = abs(lit) - 1
        if lit > 0:
            pos_count[var] += 1
        else:
            neg_count[var] += 1

balances = []
for v in range(num_vars):
    total = pos_count[v] + neg_count[v]
    if total > 0:
        balance = (pos_count[v] - neg_count[v]) / total
        balances.append(balance)

print(f"\nBalance statistics:")
print(f"  Mean: {np.mean(balances):.4f}")
print(f"  Std: {np.std(balances):.4f}")
print(f"  Min: {min(balances):.4f}")
print(f"  Max: {max(balances):.4f}")
print(f"  PHI-1 (0.618): {'Malapit' if abs(np.mean(balances) - 0.618) < 0.1 else 'Malayo'}")

# Histogram
hist, bins = np.histogram(balances, bins=10)
print(f"\n  Histogram:")
for i in range(len(hist)):
    print(f"    [{bins[i]:.2f}, {bins[i+1]:.2f}): {hist[i]}")

