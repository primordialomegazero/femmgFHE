#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace std;

// ============================================================
// BFV FHE SIMULATION — ct × ct WITHOUT BOOTSTRAPPING
// ============================================================

struct Ciphertext {
    uint64_t c0, c1;
    int noise_bits;  // estimated noise in bits
};

class BFVSim {
private:
    uint64_t q;      // ciphertext modulus
    uint64_t t;      // plaintext modulus
    uint64_t delta;  // scaling factor
    int max_noise;

public:
    BFVSim() {
        q = 10000000000000000ULL;  // ~54 bits
        t = 1024;
        delta = q / t;              // scaling
        max_noise = 50;             // noise budget in bits
    }

    // Simulate encryption — may noise agad
    Ciphertext encrypt(uint64_t m) {
        uint64_t noise = (rand() % 100) + 10;  // 10-110 noise
        Ciphertext ct;
        ct.c0 = (m * delta + noise) % q;
        ct.c1 = (rand() % 100) % q;  // random component
        ct.noise_bits = (int)(log2(noise)) + 1;
        return ct;
    }

    // Decrypt — estimate kung tama pa
    uint64_t decrypt(const Ciphertext& ct) {
        uint64_t m = (ct.c0 % q) / delta;
        return m % t;
    }

    // ============================================================
    // MULTIPLICATION: ct × ct
    // ============================================================
    Ciphertext multiply(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result;
        
        // BFV multiplication — 3 terms!
        result.c0 = (a.c0 * b.c0) % q;
        result.c1 = (a.c0 * b.c1 + a.c1 * b.c0) % q;
        // Note: c2 = a.c1 * b.c1 (dropped without relinearization!)
        
        // Estimate noise growth
        // Each multiplication roughly doubles+t adds noise
        int noise_growth = 15 + (rand() % 10);  // 15-25 bits growth
        result.noise_bits = a.noise_bits + b.noise_bits + noise_growth;
        
        return result;
    }

    // With relinearization (simulated)
    Ciphertext multiply_relinearize(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result = multiply(a, b);
        // Relinearization reduces noise
        result.noise_bits = max(a.noise_bits, b.noise_bits) + 10;
        return result;
    }

    // Check if ciphertext is still valid
    bool is_valid(const Ciphertext& ct) {
        return ct.noise_bits < max_noise;
    }

    void print_status(const Ciphertext& ct, const string& label) {
        cout << label << ": "
             << "noise=" << ct.noise_bits << " bits"
             << (is_valid(ct) ? " ✅" : " ❌ OVERFLOW")
             << endl;
    }
};

// ============================================================
// TEST: ct × ct CHAIN (SQUARING)
// ============================================================

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  CT × CT SIMULATION — NO BOOTSTRAPPING        ║\n";
    cout << "║  BFV Multiplication Chain Test                ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    srand(1618033988);  // Golden ratio seed

    BFVSim fhe;

    // Start with m = 2
    uint64_t m = 2;
    Ciphertext ct = fhe.encrypt(m);
    
    cout << "Starting with m = " << m << "\n";
    fhe.print_status(ct, "Initial");
    cout << "\n";

    // Chain of squaring: ct = ct × ct
    vector<int> expected;
    uint64_t current = m;
    for (int i = 0; i < 8; i++) {
        expected.push_back(current);
        current = (current * current) % 1024;
    }

    cout << "Expected sequence: ";
    for (int i = 0; i < 8; i++) {
        cout << expected[i];
        if (i < 7) cout << " → ";
    }
    cout << "\n\n";

    cout << "--- CHAINED SQUARING (ct × ct) ---\n";
    
    for (int step = 0; step < 8; step++) {
        if (!fhe.is_valid(ct)) {
            cout << "❌ STOPPED at step " << step << " — noise overflow!\n";
            break;
        }

        uint64_t decrypted = fhe.decrypt(ct);
        bool correct = (decrypted == expected[step]);

        cout << "Step " << step << ": "
             << "dec=" << setw(4) << decrypted
             << " exp=" << setw(4) << expected[step]
             << (correct ? " ✅" : " ❌")
             << " | noise=" << ct.noise_bits << " bits";

        if (!fhe.is_valid(ct)) {
            cout << " ⚠️ CRITICAL";
        }
        cout << "\n";

        // Multiply: ct = ct × ct
        ct = fhe.multiply(ct, ct);
    }

    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    return 0;
}
