import numpy as np

PHI = (1 + np.sqrt(5)) / 2
PSI = (1 - np.sqrt(5)) / 2

def fixed_point_iterate(x):
    if x >= 0:
        return np.sqrt(x + 1.0)
    return -1.0 / x

def clause_energy(clause, state):
    energy = 0.0
    any_true = False
    
    for lit in clause:
        var = abs(lit) - 1
        val = state[var]
        is_true = (lit > 0 and val > 0) or (lit < 0 and val < 0)
        if is_true:
            any_true = True
            break
    
    if not any_true:
        for lit in clause:
            var = abs(lit) - 1
            val = state[var]
            target = PHI if lit > 0 else PSI
            energy += (val - target) ** 2
    
    return energy

def total_energy(clauses, state):
    return sum(clause_energy(c, state) for c in clauses)

def gradient(clauses, state, num_vars):
    grad = np.zeros(num_vars)
    eps = 1e-6
    
    for v in range(num_vars):
        state_plus = state.copy()
        state_minus = state.copy()
        state_plus[v] += eps
        state_minus[v] -= eps
        
        e_plus = total_energy(clauses, state_plus)
        e_minus = total_energy(clauses, state_minus)
        grad[v] = (e_plus - e_minus) / (2 * eps)
    
    return grad

# Generate test case
np.random.seed(42)
num_vars = 10
num_clauses = num_vars * 3

clauses = []
for c in range(num_clauses):
    clause = []
    for k in range(3):
        var = np.random.randint(1, num_vars + 1)
        if np.random.rand() > 0.5:
            var = -var
        clause.append(var)
    clauses.append(clause)

print("=" * 70)
print("GOLDEN FIXED POINT DEBUG - TEXT VERSION")
print("=" * 70)
print(f"\nFormula: {num_vars} vars, {num_clauses} clauses")
print(f"PHI = {PHI:.6f}, PSI = {PSI:.6f}")
print(f"PHI * PSI = {PHI * PSI:.6f} (should be -1)")
print()

# Print clauses
print("Clauses:")
for i, clause in enumerate(clauses):
    print(f"  {i+1}: {clause}")
print()

# Initial state
state = np.array([PHI if v % 2 == 0 else PSI for v in range(num_vars)])
print(f"Initial state: [{', '.join([f'{v:.4f}' for v in state])}]")
print(f"Initial energy: {total_energy(clauses, state):.6f}")
print()

# Trace energy over iterations
energies = []
states_trace = []
grads_trace = []

max_iter = 30
learning_rate = 0.1

print("Iteration trace (first 30 iterations):")
print("-" * 70)
print(f"{'Iter':<6} {'Energy':<12} {'Change':<12} {'|Grad|':<12} {'State[0:3]':<30}")
print("-" * 70)

prev_energy = total_energy(clauses, state)

for iter in range(max_iter):
    energy = total_energy(clauses, state)
    energies.append(energy)
    states_trace.append(state.copy())
    
    grad = gradient(clauses, state, num_vars)
    grads_trace.append(np.linalg.norm(grad))
    
    # Update
    lr = learning_rate / (1.0 + 0.01 * iter)
    state = state - lr * grad
    
    # Fixed point push
    for v in range(num_vars):
        state[v] = fixed_point_iterate(state[v])
        
        # Clamp
        if state[v] > 10.0:
            state[v] = PHI
        if state[v] < -10.0:
            state[v] = PSI
        if abs(state[v]) < 1e-10:
            state[v] = PHI if v % 2 == 0 else PSI
    
    change = energy - prev_energy
    prev_energy = energy
    
    if iter % 3 == 0 or iter < 5:
        state_str = f"[{', '.join([f'{s:.2f}' for s in state[:3]])}...]"
        print(f"{iter:<6} {energy:<12.6f} {change:<12.6f} {grads_trace[-1]:<12.6f} {state_str}")

print()
print(f"Final energy: {energies[-1]:.6f}")
print(f"Total energy change: {energies[0] - energies[-1]:.6f}")
print()

# Analyze clause satisfaction
print("=" * 70)
print("CLAUSE SATISFACTION ANALYSIS")
print("=" * 70)
final_state = states_trace[-1]
print(f"\nFinal state: [{', '.join([f'{v:.4f}' for v in final_state])}]")
print(f"Final state values: {[1 if v > 0 else 0 for v in final_state]}")
print()

sat_count = 0
unsat_count = 0
unsat_clauses = []

for i, clause in enumerate(clauses):
    sat = False
    for lit in clause:
        var = abs(lit) - 1
        val = final_state[var]
        is_true = (lit > 0 and val > 0) or (lit < 0 and val < 0)
        if is_true:
            sat = True
            break
    
    energy = clause_energy(clause, final_state)
    if sat:
        sat_count += 1
    else:
        unsat_count += 1
        unsat_clauses.append((i+1, clause, energy))

print(f"Satisfied clauses: {sat_count}/{len(clauses)}")
print(f"Unsatisfied clauses: {unsat_count}/{len(clauses)}")
print()

if unsat_clauses:
    print("Unsatisfied clauses:")
    for idx, clause, energy in unsat_clauses[:5]:
        print(f"  Clause {idx}: {clause} → energy={energy:.4f}")
        for lit in clause:
            var = abs(lit) - 1
            val = final_state[var]
            is_true = (lit > 0 and val > 0) or (lit < 0 and val < 0)
            print(f"    Lit {lit}: var={var}, val={val:.4f}, is_true={is_true}")
print()

# Local minima analysis
print("=" * 70)
print("LOCAL MINIMA ANALYSIS")
print("=" * 70)
print()

e_final = total_energy(clauses, final_state)
found_better = False

for i in range(num_vars):
    for target_val in [PHI, PSI]:
        state_test = final_state.copy()
        state_test[i] = target_val
        e_test = total_energy(clauses, state_test)
        
        if e_test < e_final:
            print(f"  Var {i+1}: Setting to {target_val:.2f} reduces energy: {e_final:.4f} → {e_test:.4f}")
            found_better = True

if not found_better:
    print("  No single-variable change reduces energy")
    print("  → Sa local minima tayo, kailangan ng multi-variable jump")
print()

# Check kung may obvious solution
print("=" * 70)
print("EXHAUSTIVE SEARCH (para sa 10 vars)")
print("=" * 70)
print()

best_energy = float('inf')
best_assignment = None

for bits in range(2**num_vars):
    test_state = np.array([PHI if (bits >> v) & 1 else PSI for v in range(num_vars)])
    e = total_energy(clauses, test_state)
    
    if e < best_energy:
        best_energy = e
        best_assignment = [(bits >> v) & 1 for v in range(num_vars)]

print(f"Best possible energy: {best_energy:.6f}")
print(f"Best assignment: {best_assignment}")
print(f"Final energy from gradient descent: {e_final:.6f}")
print(f"Gap: {e_final - best_energy:.6f}")
print()

if best_energy == 0:
    print("✓ May solution na may energy=0")
    print(f"  Assignment: {best_assignment}")
else:
    print("✗ Walang solution na may energy=0 (UNSAT formula)")
print()

# Energy landscape sampling
print("=" * 70)
print("ENERGY LANDSCAPE SAMPLING")
print("=" * 70)
print()

# Sample random states and check energy distribution
np.random.seed(123)
random_energies = []
for _ in range(1000):
    random_state = np.random.choice([PHI, PSI], size=num_vars)
    random_energies.append(total_energy(clauses, random_state))

print(f"Random sampling (1000 states):")
print(f"  Min energy: {min(random_energies):.6f}")
print(f"  Max energy: {max(random_energies):.6f}")
print(f"  Mean energy: {np.mean(random_energies):.6f}")
print(f"  Std energy: {np.std(random_energies):.6f}")
print()

# Check kung gaano kalayo ang final state sa best
final_binary = [1 if v > 0 else 0 for v in final_state]
distance = sum(1 for a, b in zip(final_binary, best_assignment) if a != b)
print(f"Final state (binary): {final_binary}")
print(f"Best assignment:      {best_assignment}")
print(f"Hamming distance: {distance}/{num_vars}")
print()

if distance > 0:
    print("Variables na mali:")
    for i in range(num_vars):
        if final_binary[i] != best_assignment[i]:
            val = final_state[i]
            print(f"  Var {i+1}: current={val:.4f} ({final_binary[i]}), should be {best_assignment[i]}")
