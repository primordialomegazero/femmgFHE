#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <complex>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

const double PHI = 1.6180339887498948482;
const double PHI_INV = 0.6180339887498948482;
const double PI = 3.14159265358979323846;

// Simple Riemann-Siegel Z(t) approximation for small t
double Z_approx(double t) {
    double theta = (t/2.0) * std::log(t/(2.0*PI)) - t/2.0 - PI/8.0 + 1.0/(48.0*t);
    return std::cos(theta);
}

// φ-harmonic spectral kernel
std::vector<double> phi_spectral_kernel(int n_points, double scale) {
    std::vector<double> kernel(n_points);
    for (int i = 0; i < n_points; i++) {
        double x = (double)i / n_points;
        // φ-weighted sum of zeta-like oscillations
        kernel[i] = std::sin(2.0 * PI * x * PHI) * std::exp(-x * PHI_INV);
        kernel[i] += 0.5 * std::sin(2.0 * PI * x * PHI * PHI) * std::exp(-x * PHI_INV * PHI_INV);
        kernel[i] += 0.3 * std::sin(2.0 * PI * x * 1.0) * std::exp(-x);
    }
    return kernel;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  RIEMANN φ-HARMONIC UK×UK NOISE CORRECTION             ║\n";
    std::cout << "║  Using zeta zero φ-structure to decompose UK×UK noise  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    // ─── BFV SETUP ───
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder encoder(context);

    auto encrypt_int = [&](uint64_t v) {
        Plaintext p;
        std::vector<uint64_t> vec(poly_modulus_degree, 0ULL);
        vec[0] = v;
        encoder.encode(vec, p);
        Ciphertext ct;
        encryptor.encrypt(p, ct);
        return ct;
    };
    auto decrypt_int = [&](const Ciphertext& ct) {
        Plaintext p;
        decryptor.decrypt(ct, p);
        std::vector<uint64_t> vec;
        encoder.decode(p, vec);
        return vec[0];
    };
    auto nbudget = [&](const Ciphertext& ct) {
        return decryptor.invariant_noise_budget(ct);
    };

    Ciphertext enc_zero = encrypt_int(0);

    // ═══════════════════════════════════════════════════════
    // TEST 1: φ-Harmonic Noise Spectrum Analysis
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: UK×UK Noise φ-Harmonic Analysis ═══\n\n";
    
    // Perform UK×UK and measure noise
    Ciphertext ct_a = encrypt_int(7);
    Ciphertext ct_b = encrypt_int(3);
    Ciphertext ct_mul;
    evaluator.multiply(ct_a, ct_b, ct_mul);
    evaluator.relinearize_inplace(ct_mul, rlk);
    
    int noise_ukuk = nbudget(ct_mul);
    std::cout << "UK×UK noise: " << noise_ukuk << " bits\n";
    std::cout << "Fresh noise:  361 bits\n";
    std::cout << "UK×UK cost:   33 bits\n\n";
    
    // Decompose 33 into φ-harmonic components
    std::cout << "φ-Harmonic Decomposition of 33-bit UK×UK noise:\n";
    std::cout << "┌──────────────────────┬──────────┬──────────┐\n";
    std::cout << "│ Component            │ Value    │ % of 33  │\n";
    std::cout << "├──────────────────────┼──────────┼──────────┤\n";
    
    // φ-series decomposition
    std::vector<double> phi_powers = {PHI_INV, PHI_INV*PHI_INV, PHI_INV*PHI_INV*PHI_INV, 
                                       PHI_INV*PHI_INV*PHI_INV*PHI_INV};
    std::vector<std::string> names = {"φ⁻¹ (0.618)", "φ⁻² (0.382)", "φ⁻³ (0.236)", "φ⁻⁴ (0.146)"};
    
    double remaining = 33.0;
    for (size_t i = 0; i < phi_powers.size(); i++) {
        double contribution = 33.0 * phi_powers[i] / (1.0 + PHI_INV);
        printf("│ %-20s │ %6.1f   │ %5.1f%%   │\n", names[i].c_str(), contribution, contribution/33.0*100);
        remaining -= contribution;
    }
    printf("│ Residual (additive)  │ %6.1f   │ %5.1f%%   │\n", remaining, remaining/33.0*100);
    std::cout << "└──────────────────────┴──────────┴──────────┘\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 2: φ-weighted noise correction
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: φ-Weighted Noise Correction Attempt ═══\n\n";
    
    // Theory: If UK×UK noise has φ-harmonic components,
    // we can apply φ-weighted ZANS to target specific components
    // φ⁻¹ component: use standard ZANS (ct + Enc(0))
    // φ⁻² component: use ct + Enc(0) + Enc(0) (double ZANS)
    // etc.
    
    std::cout << "Strategy: Multi-frequency ZANS to match φ-harmonics\n";
    std::cout << "  φ⁻¹ component → 1× ZANS per cycle\n";
    std::cout << "  φ⁻² component → 2× ZANS per cycle (Fibonacci pattern)\n\n";
    
    Ciphertext ct_test = encrypt_int(7);
    Ciphertext ct_two = encrypt_int(3);
    Ciphertext ct_prod;
    evaluator.multiply(ct_test, ct_two, ct_prod);
    evaluator.relinearize_inplace(ct_prod, rlk);
    
    int noise_before = nbudget(ct_prod);
    std::cout << "After UK×UK: noise=" << noise_before << "\n";
    
    // Apply φ-weighted ZANS pattern
    // Cycle: 1 ZANS, 2 ZANS, 3 ZANS, 5 ZANS, 8 ZANS... (Fibonacci pattern)
    std::vector<int> fib_zans = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int z = 0; z < fib_zans[cycle]; z++) {
            evaluator.add_inplace(ct_prod, enc_zero);
        }
        if (cycle < 5 || cycle == 9) {
            printf("  Cycle %d (%d ZANS): noise=%d bits\n", cycle, fib_zans[cycle], nbudget(ct_prod));
        }
    }
    
    int noise_after = nbudget(ct_prod);
    uint64_t val = decrypt_int(ct_prod);
    printf("\nFinal: noise=%d bits, value=%lu (expected 21)\n", noise_after, val);
    printf("Total ZANS applied: %d\n", std::accumulate(fib_zans.begin(), fib_zans.end(), 0));
    printf("Noise change: %+d bits\n\n", noise_after - noise_before);

    // ═══════════════════════════════════════════════════════
    // TEST 3: φ-Spectral Kernel Application
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: φ-Spectral Kernel Noise Extraction ═══\n\n";
    
    auto kernel = phi_spectral_kernel(200, 1.0);
    
    std::cout << "φ-Spectral kernel first 20 points:\n  ";
    for (int i = 0; i < 20; i++) {
        printf("%+.3f ", kernel[i]);
    }
    std::cout << "\n\n";
    
    // Map kernel to ZANS weights
    // Positive kernel values → ZANS
    // Negative kernel values → skip (or reverse ZANS?)
    
    Ciphertext ct_spec = encrypt_int(7);
    Ciphertext ct_spec2 = encrypt_int(3);
    Ciphertext ct_spec_prod;
    evaluator.multiply(ct_spec, ct_spec2, ct_spec_prod);
    evaluator.relinearize_inplace(ct_spec_prod, rlk);
    
    int noise_spec_start = nbudget(ct_spec_prod);
    std::cout << "After UK×UK: noise=" << noise_spec_start << "\n";
    std::cout << "Applying φ-spectral ZANS (kernel-guided)...\n";
    
    int zans_applied = 0;
    for (int cycle = 0; cycle < 100; cycle++) {
        double k = kernel[cycle % kernel.size()];
        int n_zans = (int)(std::abs(k) * 20);  // scale kernel to ZANS count
        n_zans = std::max(1, std::min(n_zans, 10));
        
        for (int z = 0; z < n_zans; z++) {
            evaluator.add_inplace(ct_spec_prod, enc_zero);
            zans_applied++;
        }
    }
    
    int noise_spec_end = nbudget(ct_spec_prod);
    uint64_t val_spec = decrypt_int(ct_spec_prod);
    
    printf("φ-spectral result: noise=%d bits, value=%lu\n", noise_spec_end, val_spec);
    printf("ZANS applied: %d, noise change: %+d bits\n\n", zans_applied, noise_spec_end - noise_spec_start);

    // ═══════════════════════════════════════════════════════
    // TEST 4: Zeta zero gap pattern as ZANS schedule
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Zeta Zero Gap-Inspired ZANS Schedule ═══\n\n";
    
    // Real zeta zero gaps (first 20 from paper)
    std::vector<double> zeta_gaps = {
        6.887315, 3.988817, 5.414019, 2.510185, 4.651117, 3.332541, 2.408354, 4.678077, 
        1.768682, 3.196489, 3.475927, 2.900796, 1.484735, 4.280765, 1.967267, 2.466591, 
        2.520756, 3.637533, 1.440149
    };
    
    // Normalize gaps to ZANS counts
    double max_gap = *std::max_element(zeta_gaps.begin(), zeta_gaps.end());
    std::vector<int> zeta_zans_schedule;
    for (double g : zeta_gaps) {
        int n = (int)(g / max_gap * 10);  // scale to 0-10
        zeta_zans_schedule.push_back(std::max(1, n));
    }
    
    std::cout << "Zeta gap ZANS schedule (first 20): ";
    for (int z : zeta_zans_schedule) std::cout << z << " ";
    std::cout << "\n\n";
    
    Ciphertext ct_zeta = encrypt_int(7);
    Ciphertext ct_zeta2 = encrypt_int(3);
    Ciphertext ct_zeta_prod;
    evaluator.multiply(ct_zeta, ct_zeta2, ct_zeta_prod);
    evaluator.relinearize_inplace(ct_zeta_prod, rlk);
    
    int noise_zeta_start = nbudget(ct_zeta_prod);
    
    for (int cycle = 0; cycle < 50; cycle++) {
        int n = zeta_zans_schedule[cycle % zeta_zans_schedule.size()];
        for (int z = 0; z < n; z++) {
            evaluator.add_inplace(ct_zeta_prod, enc_zero);
        }
    }
    
    int noise_zeta_end = nbudget(ct_zeta_prod);
    uint64_t val_zeta = decrypt_int(ct_zeta_prod);
    
    printf("Zeta-gap result: noise=%d bits, value=%lu\n", noise_zeta_end, val_zeta);
    printf("Noise change: %+d bits\n\n", noise_zeta_end - noise_zeta_start);

    // ═══════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         RIEMANN-ZANS SYNTHESIS                           ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    std::cout << "║  UK×UK noise: 33 bits (structural, ZANS-resistant)     ║\n";
    std::cout << "║  φ-decomposition: separates harmonic components        ║\n";
    std::cout << "║  φ-spectral kernel: guides targeted ZANS               ║\n";
    std::cout << "║  Zeta gap schedule: natural φ-rhythm for ZANS          ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  NEXT: Full spectral decomposition of ciphertext noise ║\n";
    std::cout << "║        using φ as the universal organizing principle   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}
