import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

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

print("=== GOLDEN FIXED POINT DEBUG ===\n")
print(f"Formula: {num_vars} vars, {num_clauses} clauses")
print(f"PHI = {PHI:.6f}, PSI = {PSI:.6f}\n")

# Initial state
state = np.array([PHI if v % 2 == 0 else PSI for v in range(num_vars)])
print(f"Initial state: {state}\n")
print(f"Initial energy: {total_energy(clauses, state):.6f}\n")

# Trace energy over iterations
energies = []
states_trace = []

max_iter = 50
learning_rate = 0.1

for iter in range(max_iter):
    energy = total_energy(clauses, state)
    energies.append(energy)
    states_trace.append(state.copy())
    
    grad = gradient(clauses, state, num_vars)
    
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
    
    if iter % 5 == 0:
        print(f"Iter {iter}: energy={energy:.6f}, state={state}")

print(f"\nFinal energy: {energies[-1]:.6f}")
print(f"Energy change: {energies[0] - energies[-1]:.6f}")

# Plot energy landscape
plt.figure(figsize=(12, 4))

plt.subplot(1, 3, 1)
plt.plot(range(len(energies)), energies, 'b-', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('Energy')
plt.title('Energy Trajectory')
plt.grid(True)

# Plot state evolution
plt.subplot(1, 3, 2)
states_array = np.array(states_trace)
for v in range(min(5, num_vars)):
    plt.plot(range(len(states_trace)), [s[v] for s in states_trace], 
             label=f'Var {v+1}', linewidth=1.5)
plt.axhline(y=PHI, color='r', linestyle='--', alpha=0.5, label='PHI')
plt.axhline(y=PSI, color='g', linestyle='--', alpha=0.5, label='PSI')
plt.axhline(y=0, color='k', linestyle='-', alpha=0.3)
plt.xlabel('Iteration')
plt.ylabel('Value')
plt.title('State Evolution')
plt.legend()
plt.grid(True)

# Plot gradient magnitude
plt.subplot(1, 3, 3)
grad_magnitudes = []
for s in states_trace:
    g = gradient(clauses, s, num_vars)
    grad_magnitudes.append(np.linalg.norm(g))

plt.plot(range(len(grad_magnitudes)), grad_magnitudes, 'r-', linewidth=2)
plt.xlabel('Iteration')
plt.ylabel('|Gradient|')
plt.title('Gradient Magnitude')
plt.grid(True)

plt.tight_layout()
plt.savefig('fixed_point_debug.png', dpi=150)
plt.show()

# Analyze clauses satisfaction
print("\n=== CLAUSE SATISFACTION ANALYSIS ===\n")
final_state = states_trace[-1]
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
    status = "SAT" if sat else f"UNSAT (energy={energy:.4f})"
    print(f"  Clause {i+1}: {clause} → {status}")

# Check kung may local minima
print("\n=== LOCAL MINIMA ANALYSIS ===\n")
for i in range(num_vars):
    for target in [PHI, PSI]:
        state_test = final_state.copy()
        state_test[i] = target
        e_test = total_energy(clauses, state_test)
        e_final = total_energy(clauses, final_state)
        if e_test < e_final:
            print(f"  Var {i+1}: Setting to {target:.2f} reduces energy from {e_final:.4f} to {e_test:.4f}")
