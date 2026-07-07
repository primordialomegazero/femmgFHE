import matplotlib.pyplot as plt
import numpy as np

# Data from the matrix
data = {
    (2,10): 1.60, (2,12): 1.50, (2,14): 1.43, (2,16): 1.38, (2,18): 1.33, (2,20): 1.60,
    (4,10): 1.60, (4,12): 1.50, (4,14): 1.43, (4,16): 1.38, (4,18): 1.33, (4,20): 1.60,
    (6,10): 1.60, (6,12): 1.50, (6,14): 1.43, (6,16): 1.38, (6,18): 1.33, (6,20): 1.60,
    (8,10): 1.60, (8,12): 1.50, (8,14): 1.43, (8,16): 1.38, (8,18): 1.33, (8,20): 1.60,
    (10,10): 2.56, (10,12): 2.40, (10,14): 2.29, (10,16): 2.20, (10,18): 2.13, (10,20): 2.56,
}

# Extract patterns
a_values = [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]
b_values = [10, 12, 14, 16, 18, 20]

for a in a_values:
    ratios = []
    for b in b_values:
        if (a,b) in data:
            ratios.append(data[(a,b)])
        else:
            ratios.append(None)
    print(f"a={a}: {ratios}")

# Analyze the formula
print("\nAnalyzing formula: Ratio = (a + b) / (a + b - X)")
for X in range(1, 20):
    for a in [2, 10]:
        for b in [10, 20]:
            if (a,b) in data:
                predicted = (a + b) / (a + b - X)
                actual = data[(a,b)]
                diff = abs(predicted - actual)
                if diff < 0.01:
                    print(f"X={X}: a={a}, b={b} -> predicted={predicted:.3f}, actual={actual:.3f}")
