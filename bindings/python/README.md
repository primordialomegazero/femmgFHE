# Spiral Fractal iO — Python Package

```python
import spiral_fhe

# KS = 0.000000 — Pure Zero Indistinguishability
spiral_fhe.obfuscate("int f(int x) { return x * 7 + 13; }")
result = spiral_fhe.execute("output.obf", [0.5, 0.3])
ks = spiral_fhe.verify("a.c", "a.c")  # Same program → KS = 0.000000
```
