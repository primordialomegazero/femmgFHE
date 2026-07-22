// Security estimation for φ-LWE KEM
#include <stdio.h>
#include <math.h>

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   φ-LWE KEM — Security Estimation                     ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");

    int N_vals[] = {8, 16, 32, 64, 128, 256};
    double q = 3329.0;
    
    printf("  %4s | %6s | %10s | %12s | %12s\n", "N", "dim", "log2(q^N)", "BKZ cost", "Security");
    printf("  ------+--------+-------------+--------------+-----------\n");
    
    for (int j = 0; j < 6; j++) {
        int N = N_vals[j];
        int dim = 2 * N;  // φ-extension dimension
        
        // The LWE secret is ternary: { -1, 0, 1 } with 3^dim possibilities
        double entropy = dim * log2(3.0);
        
        // Lattice dimension for attack
        int lattice_dim = dim + 1;  // LWE lattice dimension ≈ dim+1
        
        // BKZ cost estimate (simplified: log2 cost ≈ 0.292 * β)
        // For a 16-dim lattice, BKZ with β=dim solves it trivially
        double bkz_beta = lattice_dim;  // Full BKZ
        double log2_cost = 0.292 * bkz_beta;  // Core-SVP model
        
        // True security: min(entropy, 2^log2_cost) but lattice attacks
        // dominate for small dimensions
        double sec_bits = fmin(entropy, log2_cost);
        
        // Heuristic: for very small dim, attack is trivial
        if (lattice_dim < 60) {
            sec_bits = fmin(sec_bits, lattice_dim * 0.5);  // Very weak
        }
        if (lattice_dim < 30) {
            sec_bits = fmin(sec_bits, 5.0);  // Essentially broken
        }
        
        printf("  %4d | %6d | %11.1f | %12.1f | %8.1f bits", 
               N, lattice_dim, N*log2(q), bkz_beta, sec_bits);
        
        if (sec_bits < 80) printf(" ⚠️ WEAK");
        if (sec_bits < 40) printf(" ❌ BROKEN");
        printf("\n");
    }
    
    printf("\n  ╔══════════════════════════════════════════════════════╗\n");
    printf(  "  ║   HONEST ASSESSMENT                                   ║\n");
    printf(  "  ║   N=8:  16-dim lattice → trivially breakable         ║\n");
    printf(  "  ║   N=64: 128-dim lattice → needs analysis             ║\n");
    printf(  "  ║   N=256: 512-dim lattice → competitive with Kyber    ║\n");
    printf(  "  ║   For real security: N ≥ 256 (like Kyber-512)       ║\n");
    printf(  "  ║   The micro sizes are proofs of concept              ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}
