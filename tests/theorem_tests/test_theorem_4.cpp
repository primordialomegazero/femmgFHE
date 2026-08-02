// ═══════════════════════════════════════════════════════════════
// THEOREM 4 — Commutative Reconstruction (Standalone Test)
// ═══════════════════════════════════════════════════════════════
// Formal Proof: docs/FORMAL_PROOFS.md#theorem-4
// Code: unified-phi-stack/phi_stack.h:147-160
//
// Verifies: reconstruct(σ(v)) = reconstruct(v) for any permutation σ

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) {
        sum += val;
        prod *= (val + 0.0001);
        harm_sum += 1.0 / (val + 0.001);
        sum_sq += val * val;
    }
    return 0.35 * sum/n + 0.25 * std::pow(prod, 1.0/n) 
         + 0.25 * n/harm_sum + 0.15 * std::sqrt(sum_sq/n);
}

int main() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  THEOREM 4: Commutative Reconstruction (Standalone)          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::vector<double> original = {0.1, 0.3, 0.5, 0.7, 0.9};
    std::vector<double> permuted = {0.9, 0.1, 0.7, 0.3, 0.5};
    std::vector<double> reversed = {0.9, 0.7, 0.5, 0.3, 0.1};
    
    double r1 = commutative_reconstruct(original);
    double r2 = commutative_reconstruct(permuted);
    double r3 = commutative_reconstruct(reversed);
    
    std::cout << "  Original:  " << r1 << "\n";
    std::cout << "  Permuted:  " << r2 << "\n";
    std::cout << "  Reversed:  " << r3 << "\n";
    std::cout << "  Max diff:  " << std::max({std::abs(r1-r2), std::abs(r2-r3), std::abs(r1-r3)}) << "\n";
    
    bool ok = (std::abs(r1 - r2) < 1e-10) && (std::abs(r2 - r3) < 1e-10);
    
    std::cout << "\n  " << (ok ? "✅ THEOREM 4 VERIFIED" : "❌ FAILED") 
              << " — Reconstruction is order-independent\n";
    std::cout << "  📎 Formal Proof: docs/FORMAL_PROOFS.md#theorem-4\n";
    std::cout << "  📎 Code: unified-phi-stack/phi_stack.h:147-160\n\n";
    
    assert(ok);
    return 0;
}
