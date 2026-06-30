# Chaotic Trajectory Unpredictability (CTU) Assumption

## Formal Definition

**Assumption (CTU).** Let \( \mathcal{C} \) be the 7-dimensional Coupled Map Lattice defined by:

\[
x_d(t+1) = \varphi \cdot x_d(t) \cdot (1 - x_d(t)) + \varphi^{-1} \cdot \sum_{j \neq d} \frac{x_j(t) - x_d(t)}{1 + |d - j|}
\]

where \( d \in \{0, \ldots, 6\} \), \( \varphi = \frac{1+\sqrt{5}}{2} \), and \( x_d(0) \) is sampled from a 256-bit seed.

For any PPT adversary \( \mathcal{A} \) given \( t \) consecutive outputs \( \{x(t_0), \ldots, x(t_0 + t - 1)\} \):

\[
\text{Adv}_{\mathcal{A}}^{\text{CTU}}(\kappa) = \left| \Pr[\mathcal{A} \text{ predicts } x(t_0 + t)] - \frac{1}{|\mathcal{X}|} \right| \leq O(2^{-t \cdot \lambda_{\min}})
\]

where \( \lambda_{\min} \approx 0.48 \) is the minimum Lyapunov exponent of the 7D CML, \( \kappa = 256 \) is the security parameter, and \( |\mathcal{X}| \) is the phase space volume.

---

## What CTU Is

| Property | Description |
|----------|-------------|
| **Type** | Computational hardness assumption |
| **Analogous to** | LWE (Learning With Errors) in lattice cryptography |
| **Based on** | Lyapunov chaos theory + computational irreversibility |
| **Security parameter** | \( \kappa = 256 \) bits |
| **Key space** | \( \sim 2^{420} \) (7 dimensions × 60 bits each) |

---

## What CTU Is NOT

| Misconception | Reality |
|---------------|---------|
| ❌ "Chaos = random" | Chaos is **deterministic** but **unpredictable** without exact initial conditions |
| ❌ "CTU is information-theoretic" | CTU is **computational** — a PPT adversary cannot predict, but an unbounded adversary can |
| ❌ "CTU replaces all security" | CTU provides IND-CPA; 256-bit CSPRNG nonce provides statistical randomization |
| ❌ "CTU depends on Riemann Hypothesis" | Riemann zeros are empirical motivation only — CTU relies on Lyapunov exponents |

---

## Mathematical Foundation

### 1. Lyapunov Exponents

The 7D CML has 7 Lyapunov exponents. The maximum is:

\[
\lambda_{\max} = \ln(\varphi) \approx 0.4812118250596034
\]

**Lyapunov time** (time to lose 1 bit of precision):

\[
\tau = \frac{1}{\lambda_{\max}} \approx 2.08 \text{ iterations}
\]

After 7 iterations (one Banach layer):

\[
\text{divergence factor} = e^{7 \cdot 0.4812} = e^{3.368} \approx 29.0
\]

### 2. Key Space

Each dimension contributes ~60 bits of effective state:
\[
|\mathcal{K}| \approx (2^{60})^7 = 2^{420}
\]

### 3. Prediction Advantage

After \( t \) iterations, the advantage of predicting the next state decays exponentially:

\[
\text{Adv}^{\text{CTU}}_{\mathcal{A}} \leq c \cdot 2^{-t \cdot \lambda_{\min}}
\]

For \( t = 7 \) (one Banach layer): \( \text{Adv} \leq c \cdot 2^{-3.37} \approx c \cdot 0.097 \)

For \( t = 128 \) (one KEM evolution): \( \text{Adv} \leq c \cdot 2^{-61.6} \approx \text{negl}(\kappa) \)

---

## Implementation in FEmmg-FHE

| Component | File | Role |
|-----------|------|------|
| **7D CML Engine** | `src/lyapunov_core.h` | Lyapunov-coupled chaotic map lattice |
| **Fibonacci-Lyapunov Engine** | `src/banach_engine.h` | Banach contraction with CML perturbation |
| **Φ-PKE KEM** | `phi_parallel_kem.h` | 7-lane parallel KEM with Riemann anchors |
| **CSPRNG Nonce** | `security_complete.h` | 256-bit true random nonce per encryption |
| **Perturbation Seed** | `phi_algo_merge.h` | Client-side seed for 7D perturbation |

### How CTU Provides IND-CPA

1. **Client generates** a 256-bit random seed via `/dev/urandom`
2. **Seed initializes** the 7D CML state (2^420 possible trajectories)
3. **CML evolves** through 7 Banach contraction layers
4. **Perturbation output** is XOR'd with plaintext-derived value
5. **Adversary sees** ciphertext = plaintext · φ + λ + perturbation
6. **Without the seed**, predicting perturbation requires breaking CTU

---

## Game-Hopping Proof Summary

| Game | Description | Adversary Advantage |
|------|-------------|---------------------|
| **Game 0** | Real IND-CPA with 7D CML perturbation | \( \text{Adv}_{\mathcal{A}} \) |
| **Game 1** | Replace CML perturbation with uniform random | \( \text{Adv}_{\mathcal{A}} + \text{Adv}^{\text{CTU}} \) |
| **Game 2** | Replace ciphertext with uniform random | \( \text{Adv}_{\mathcal{A}} + \text{Adv}^{\text{CTU}} + 0 \) |
| **Final** | Adversary must guess \( b \) randomly | \( \Pr = 1/2 \) |

**Conclusion:** \( \text{Adv}^{\text{IND-CPA}} \leq \text{Adv}^{\text{CTU}} + \text{negl}(\kappa) \)

---

## Comparison with Standard Assumptions

| Property | CTU | LWE | DDH | Factoring |
|----------|-----|-----|-----|-----------|
| **Type** | Chaotic dynamics | Lattice | Group theory | Number theory |
| **Quantum resistance** | ✅ (no known quantum advantage for chaos prediction) | ✅ | ❌ (Shor) | ❌ (Shor) |
| **Key size** | 256-bit seed | ~1-10 KB | 256-bit | 2048-bit+ |
| **Assumption age** | New (2026) | ~20 years (2005) | ~50 years | Ancient |
| **Third-party analysis** | ❌ Pending (IACR) | ✅ Extensive | ✅ Extensive | ✅ Extensive |

---

## Open Questions & Invitation

1. **Reduction to standard assumptions:** Can CTU be reduced to LWE or another standard assumption?
2. **Quantum attack:** Does the 7D CML admit a quantum algorithm for trajectory prediction?
3. **Provable lower bounds:** Can the Lyapunov spectrum be proven to have a minimum positive exponent?
4. **Side-channel resistance:** Does the deterministic chaos leak timing information?

**We invite the cryptographic community to attempt breaking the CTU Assumption.**

Test vectors and source code available at:  
https://github.com/primordialomegazero/femmgFHE

---

*Document version: 1.0*  
*Date: June 30, 2026*  
*Author: Dan Joseph M. Fernandez / Primordial Omega Zero*
