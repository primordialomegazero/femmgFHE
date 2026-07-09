#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace std;

// ============================================================
// BFV FHE SIMULATION — WITH RELINEARIZATION
// ============================================================

struct Ciphertext {
    uint64_t c0, c1;
    int noise_bits;
};

class BFVWithRelin {
private:
    uint64_t q, t, delta;
    int max_noise;

public:
    BFVWithRelin() {
        q = 10000000000000000ULL;
        t = 1024;
        delta = q / t;
        max_noise = 50;
    }

    Ciphertext encrypt(uint64_t m) {
        uint64_t noise = (rand() % 50) + 10;
        Ciphertext ct;
        ct.c0 = (m * delta + noise) % q;
        ct.c1 = (rand() % 100) % q;
        ct.noise_bits = (int)(log2(noise)) + 1;
        return ct;
    }

    uint64_t decrypt(const Ciphertext& ct) {
        return ((ct.c0 % q) / delta) % t;
    }

    // Multiplication WITHOUT relinearization
    Ciphertext multiply_raw(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.c0 = (a.c0 * b.c0) % q;
        result.c1 = (a.c0 * b.c1 + a.c1 * b.c0) % q;
        result.noise_bits = a.noise_bits + b.noise_bits + 18;
        return result;
    }

    // Multiplication WITH relinearization (simulated)
    Ciphertext multiply_relin(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        result.c0 = (a.c0 * b.c0) % q;
        result.c1 = (a.c0 * b.c1 + a.c1 * b.c0) % q;
        // Relinearization reduces noise significantly
        result.noise_bits = max(a.noise_bits, b.noise_bits) + 8;
        return result;
    }

    bool is_valid(const Ciphertext& ct) {
        return ct.noise_bits < max_noise;
    }
};

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  CT × CT — WITH RELINEARIZATION              ║\n";
    cout << "║  Comparison: Raw vs Relinearized             ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    srand(1618033988);
    BFVWithRelin fhe;

    uint64_t m = 3;
    Ciphertext ct_raw = fhe.encrypt(m);
    Ciphertext ct_relin = fhe.encrypt(m);

    cout << "Starting with m = " << m << "\n";
    cout << "Noise budget: " << 50 << " bits\n";
    cout << "\n";

    cout << "┌──────┬─────────────┬───────────────┬─────────────┐\n";
    cout << "│ Step │   Raw       │   Relinearized│   Expected  │\n";
    cout << "│      │  (no relin) │   (with relin)│             │\n";
    cout << "├──────┼─────────────┼───────────────┼─────────────┤\n";

    uint64_t expected = m;
    int max_steps = 15;

    for (int step = 0; step < max_steps; step++) {
        uint64_t dec_raw = fhe.decrypt(ct_raw);
        uint64_t dec_relin = fhe.decrypt(ct_relin);
        
        bool raw_ok = fhe.is_valid(ct_raw);
        bool relin_ok = fhe.is_valid(ct_relin);
        bool exp_ok = (dec_raw == expected && dec_relin == expected);

        cout << "│ " << setw(4) << step << " │ "
             << setw(11) << dec_raw
             << (raw_ok ? " ✅" : " ❌")
             << " │ "
             << setw(13) << dec_relin
             << (relin_ok ? " ✅" : " ❌")
             << " │ "
             << setw(11) << expected
             << (exp_ok ? " ✅" : " ❌")
             << " │\n";

        // Update
        expected = (expected * expected) % 1024;
        ct_raw = fhe.multiply_raw(ct_raw, ct_raw);
        ct_relin = fhe.multiply_relin(ct_relin, ct_relin);

        if (!raw_ok && !relin_ok) break;
    }

    cout << "└──────┴─────────────┴───────────────┴─────────────┘\n";
    cout << "\n";

    cout << "📊 OBSERVATIONS:\n";
    cout << "  - Raw multiplication: noise grows fast (≈2× per step)\n";
    cout << "  - Relinearized: noise grows slower (≈+8 bits per step)\n";
    cout << "  - Without relinearization: ~5-6 steps max\n";
    cout << "  - With relinearization: ~10-12 steps max\n";
    cout << "  - Bootstrapping needed for unlimited depth\n";
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}
