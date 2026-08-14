#!/usr/bin/env python3
"""
VISUALIZE SOLVER PERFORMANCE - NO EXTERNAL LIBS
================================================
Pure Python visualization ng scaling at bottlenecks
"""

import time
import random

random.seed(42)

def generate_3sat(num_vars, ratio=3.0):
    num_clauses = int(num_vars * ratio)
    clauses = []
    for _ in range(num_clauses):
        clause = []
        for _ in range(3):
            var = random.randint(1, num_vars)
            if random.random() < 0.5:
                var = -var
            clause.append(var)
        clauses.append(clause)
    return clauses

def simple_dpll_solve(clauses, num_vars):
    """Simple DPLL solver para sa timing"""
    assignment = [0] * num_vars
    
    def is_satisfied(clause, assignment):
        for lit in clause:
            var = abs(lit) - 1
            if assignment[var] == 0:
                continue
            val = assignment[var] == 1
            if (lit > 0 and val) or (lit < 0 and not val):
                return True
        return False
    
    def propagate(assignment):
        changed = True
        while changed:
            changed = False
            for clause in clauses:
                if is_satisfied(clause, assignment):
                    continue
                
                unassigned = 0
                last_lit = 0
                for lit in clause:
                    var = abs(lit) - 1
                    if assignment[var] == 0:
                        unassigned += 1
                        last_lit = lit
                
                if unassigned == 1:
                    var = abs(last_lit) - 1
                    assignment[var] = 1 if last_lit > 0 else -1
                    changed = True
                elif unassigned == 0:
                    return False
        return True
    
    def solve_recursive(assignment, steps):
        steps[0] += 1
        
        if not propagate(assignment):
            return False
        
        complete = True
        for val in assignment:
            if val == 0:
                complete = False
                break
        
        if complete:
            return True
        
        best_var = -1
        best_count = -1
        for v in range(num_vars):
            if assignment[v] != 0:
                continue
            count = 0
            for clause in clauses:
                if is_satisfied(clause, assignment):
                    continue
                for lit in clause:
                    if abs(lit) - 1 == v:
                        count += 1
                        break
            if count > best_count:
                best_count = count
                best_var = v
        
        if best_var == -1:
            return False
        
        assignment[best_var] = 1
        if solve_recursive(assignment, steps):
            return True
        
        assignment[best_var] = -1
        if solve_recursive(assignment, steps):
            return True
        
        assignment[best_var] = 0
        return False
    
    steps = [0]
    result = solve_recursive(assignment, steps)
    return result, steps[0]

def print_ascii_graph(values, labels, title, width=50):
    """Print ASCII bar graph"""
    print(f"\n{title}")
    print("=" * (width + 20))
    
    if not values:
        print("No data")
        return
    
    max_val = max(values) if max(values) > 0 else 1
    
    for i, (val, label) in enumerate(zip(values, labels)):
        bar_len = int(val / max_val * width)
        bar = "█" * bar_len
        print(f"{label:>6} | {bar} {val:>10.2f}")
    
    print("=" * (width + 20))

def print_ascii_line_graph(x_values, y_values, title, xlabel, ylabel, width=60, height=20):
    """Print ASCII line graph"""
    print(f"\n{title}")
    print(f"{ylabel}")
    
    if not y_values:
        print("No data")
        return
    
    max_y = max(y_values) if max(y_values) > 0 else 1
    min_y = 0
    
    # Create grid
    grid = []
    for row in range(height):
        grid.append([' '] * width)
    
    # Plot points
    for i, (x, y) in enumerate(zip(x_values, y_values)):
        col = int((i / (len(x_values) - 1)) * (width - 1)) if len(x_values) > 1 else 0
        row = height - 1 - int((y - min_y) / (max_y - min_y) * (height - 1)) if max_y > min_y else height - 1
        if 0 <= row < height and 0 <= col < width:
            grid[row][col] = '*'
    
    # Print grid
    for row in range(height):
        y_val = max_y - (row / (height - 1)) * (max_y - min_y) if height > 1 else max_y
        print(f"{y_val:>8.2f} | {''.join(grid[row])}")
    
    # X-axis
    print(f"{'':>10}+{'-' * width}")
    print(f"{'':>10}  {xlabel}")
    
    # X labels
    if len(x_values) > 1:
        step = max(1, len(x_values) // 5)
        labels = []
        positions = []
        for i in range(0, len(x_values), step):
            labels.append(str(x_values[i]))
            positions.append(int((i / (len(x_values) - 1)) * (width - 1)))
        
        label_line = " " * (10 + positions[0]) + labels[0]
        for i in range(1, len(labels)):
            spaces = positions[i] - positions[i-1] - len(labels[i-1])
            label_line += " " * max(0, spaces) + labels[i]
        
        print(label_line)

def main():
    print("VISUALIZING SOLVER PERFORMANCE (ASCII)\n")
    
    # Test different sizes
    sizes = [5, 10, 15, 20, 25, 30, 40, 50]
    times = []
    steps_list = []
    results = []
    
    for num_vars in sizes:
        clauses = generate_3sat(num_vars)
        
        start = time.time()
        result, steps = simple_dpll_solve(clauses, num_vars)
        elapsed = time.time() - start
        
        times.append(elapsed * 1000)
        steps_list.append(steps)
        results.append(result)
        
        print(f"  {num_vars:>3} vars: {elapsed*1000:>8.2f}ms, {steps:>6} steps, {'SAT' if result else 'UNSAT'}")
    
    # ASCII bar graph for time
    print_ascii_graph(times, [f"{s}v" for s in sizes], "SOLVE TIME (ms)")
    
    # ASCII bar graph for steps
    print_ascii_graph(steps_list, [f"{s}v" for s in sizes], "SOLVE STEPS")
    
    # ASCII line graph for time scaling
    print_ascii_line_graph(sizes, times, "TIME SCALING", "Variables", "Time (ms)")
    
    # ASCII line graph for steps scaling
    print_ascii_line_graph(sizes, steps_list, "STEPS SCALING", "Variables", "Steps")
    
    # Log scale visualization
    import math
    log_times = [math.log10(t + 0.1) for t in times]
    log_steps = [math.log10(s + 0.1) for s in steps_list]
    
    print_ascii_line_graph(sizes, log_times, "LOG TIME SCALING", "Variables", "Log10(Time)")
    print_ascii_line_graph(sizes, log_steps, "LOG STEPS SCALING", "Variables", "Log10(Steps)")
    
    # Analysis
    print("\n" + "=" * 60)
    print("ANALYSIS")
    print("=" * 60)
    
    if len(times) >= 3:
        # Check kung exponential o polynomial
        ratios = []
        for i in range(1, len(times)):
            if times[i-1] > 0:
                ratios.append(times[i] / times[i-1])
        
        avg_ratio = sum(ratios) / len(ratios)
        print(f"\nAverage time ratio between consecutive sizes: {avg_ratio:.2f}x")
        
        if avg_ratio < 3:
            print("Scaling: POLYNOMIAL (promising!)")
        elif avg_ratio < 5:
            print("Scaling: MODERATE (still manageable)")
        else:
            print("Scaling: EXPONENTIAL (problematic)")
        
        # Estimate sa larger sizes
        if times[-1] > 0:
            for target in [100, 200, 500]:
                extrapolated = times[-1] * (avg_ratio ** (len(times) - 1))
                print(f"\nExtrapolated time for {target} vars: {extrapolated:.2f}ms ({extrapolated/1000:.2f}s)")

if __name__ == "__main__":
    main()
