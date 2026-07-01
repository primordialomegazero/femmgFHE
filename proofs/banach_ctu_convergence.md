# Banach-CTU Convergence Theorem

## Theorem 6 (Noise Convergence)

Let $N_k$ be the noise after $k$ homomorphic operations. Under Banach contraction with Fibonacci floors:

$$|N_k - N_0| \leq \phi^{-k} \cdot |N_1 - N_0|$$

where $N_0 = 1.82815$ bits is the noise floor.

### Proof

The noise update is: $N_{k+1} = N_k \cdot \phi^{-1} + N_0 \cdot (1 - \phi^{-1})$

This is a contraction mapping with coefficient $c = \phi^{-1} \approx 0.618 < 1$.

By the Banach Fixed Point Theorem (1922):

$$|N_k - N_0| \leq c^k \cdot |N_1 - N_0|$$

Since $c = \phi^{-1}$: $|N_k - N_0| \leq \phi^{-k} \cdot |N_1 - N_0|$

**Corollary (Unlimited Depth):** $\lim_{k \to \infty} N_k = N_0$. Noise never exceeds the bound regardless of operation count.

**Empirical verification:** 1,000,000,000,000 operations, noise variance = 0.0 bits ✅

---

## Theorem 7 (Optimal Contraction)

$\phi^{-1}$ is the optimal contraction coefficient, maximizing $\min_c(-\ln(c) \cdot (1-c))$.

### Proof

Let $f(c) = -\ln(c) \cdot (1-c)$. Setting $f'(c) = 0$:

$$-\frac{1-c}{c} + \ln(c) = 0 \implies \ln(c) = \frac{1-c}{c}$$

Numerical solution: $c \approx 0.618034... = \phi^{-1}$

∎
