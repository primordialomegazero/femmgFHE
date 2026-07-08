#include <iostream>
#include <iomanip>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_ntt.h"
#include "../src/spiral/spiral_keygen.h"
#include "../src/spiral/spiral_encrypt.h"
#include "../src/spiral/spiral_decrypt.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE DEBUG — Noise Analysis            ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    SpiralParams params;
    SpiralNTT ntt(params.poly_degree);
    uint64_t q = ntt.modulus();
    uint64_t t = params.plain_modulus;
    
    cout << "=== PARAMETERS ===\n";
    cout << "  N = " << params.poly_degree << "\n";
    cout << "  q = " << q << " (" << (int)log2(q) << " bits)\n";
    cout << "  t = " << t << " (" << (int)log2(t) << " bits)\n";
    cout << "  q/t ratio = " << (double)q / t << "\n";
    cout << "  Noise stddev = " << params.noise_stddev << "\n\n";

    SpiralKeyGen keygen(params);
    auto kp = keygen.generate();
    
    cout << "=== SECRET KEY ===\n";
    int ones = 0, minus_ones = 0, zeros = 0;
    for (size_t i = 0; i < 100; i++) {
        if (kp.sk.s[i] == 1) ones++;
        else if (kp.sk.s[i] == -1) minus_ones++;
        else zeros++;
    }
    cout << "  First 100 coeffs: " << ones << " ones, " << minus_ones << " minus-ones, " << zeros << " zeros\n\n";

    // Manual encryption test
    cout << "=== MANUAL ENCRYPTION (Message=42, NO noise) ===\n";
    uint64_t message = 42;
    
    // Convert sk to vector
    vector<uint64_t> s_vec(params.poly_degree, 0);
    for (size_t i = 0; i < params.poly_degree; i++) {
        int64_t val = kp.sk.s[i];
        s_vec[i] = (val < 0) ? (q - (uint64_t)(-val)) : (uint64_t)val;
    }
    
    // Manual encryption with ZERO noise
    vector<uint64_t> c0(params.poly_degree, 0);
    vector<uint64_t> c1(params.poly_degree, 0);
    
    // c0[0] = message
    c0[0] = message % q;
    
    // Decrypt
    auto c1s = ntt.multiply(c1, s_vec);
    uint64_t decrypted = (c0[0] + c1s[0]) % q % t;
    cout << "  Zero-noise test: " << decrypted << " (expected " << message << ") ";
    cout << (decrypted == message ? "✅" : "❌") << "\n\n";

    // Test with increasing noise
    cout << "=== NOISE TOLERANCE TEST ===\n";
    cout << "  " << left << setw(10) << "Noise σ"
         << setw(15) << "Decrypted"
         << setw(10) << "Correct?"
         << "\n";
    cout << "  " << string(35, '-') << "\n";
    
    for (double sigma = 0.1; sigma <= 10.0; sigma *= 1.5) {
        // Add noise to c0[0]
        std::mt19937_64 rng(42);
        std::normal_distribution<double> gauss(0.0, sigma);
        
        c0[0] = (message + (uint64_t)abs((int64_t)gauss(rng))) % q;
        
        c1s = ntt.multiply(c1, s_vec);
        decrypted = (c0[0] + c1s[0]) % q % t;
        
        cout << "  " << left << setw(10) << fixed << setprecision(1) << sigma
             << setw(15) << decrypted
             << setw(10) << (decrypted == message ? "✅" : "❌")
             << "\n";
    }
    
    // Test actual SpiralNoiseGenerator
    cout << "\n=== SPIRAL NOISE GENERATOR SAMPLES ===\n";
    SpiralNoiseGenerator noise_gen;
    for (int i = 0; i < 10; i++) {
        cout << "  " << noise_gen.generate() << "\n";
    }
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  DEBUG COMPLETE                               ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
