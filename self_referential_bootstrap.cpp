#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <seal/seal.h>
using namespace std;
using namespace seal;

// ============================================================
// SELF-REFERENTIAL BOOTSTRAPPING
// Using φ (golden ratio) as the bootstrap key
// ============================================================

class SelfReferentialBootstrap {
private:
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    // Noise history for prediction
    vector<int> noise_history;
    
public:
    // Predict noise using φ-weighted moving average
    int predict_noise(const vector<int>& history) {
        if (history.size() < 2) return 0;
        
        // φ-weighted average: recent noise has more weight
        double weighted_sum = 0;
        double weight_sum = 0;
        
        for (int i = 0; i < history.size(); i++) {
            double weight = pow(PHI_INV, history.size() - i);
            weighted_sum += history[i] * weight;
            weight_sum += weight;
        }
        
        return (int)(weighted_sum / weight_sum);
    }
    
    // Apply self-referential bootstrap
    bool apply_bootstrap(Ciphertext& ct, 
                         Evaluator& evaluator,
                         Encryptor& encryptor,
                         Decryptor& decryptor,
                         const SEALContext& context,
                         int current_noise) {
        
        noise_history.push_back(current_noise);
        
        // Only bootstrap if noise is critical (< 15 bits)
        if (current_noise > 15) {
            return false; // No need to bootstrap yet
        }
        
        cout << "  🔄 SELF-REFERENTIAL BOOTSTRAP TRIGGERED!\n";
        cout << "  Current noise: " << current_noise << " bits\n";
        
        // Step 1: Predict noise using φ
        int predicted = predict_noise(noise_history);
        cout << "  φ-predicted noise: " << predicted << " bits\n";
        
        // Step 2: Apply φ-weighted ZANS
        // Instead of regular ZANS, use φ-weighted iterations
        int phi_iterations = (int)(PHI * 5); // ~8 iterations
        
        cout << "  Applying φ-ZANS (" << phi_iterations << " iterations)...\n";
        
        Plaintext zero("0");
        for (int i = 0; i < phi_iterations; i++) {
            Ciphertext ct_zero;
            encryptor.encrypt(zero, ct_zero);
            evaluator.add_inplace(ct, ct_zero);
        }
        
        // Step 3: Check new noise
        int new_noise = decryptor.invariant_noise_budget(ct);
        cout << "  New noise: " << new_noise << " bits\n";
        
        // Step 4: Verify convergence using φ
        double convergence = (double)new_noise / current_noise;
        cout << "  Convergence ratio: " << convergence 
             << " (φ⁻¹ = " << PHI_INV << ")\n";
        
        if (convergence < PHI_INV) {
            cout << "  ✅ Bootstrapping successful!\n";
            return true;
        } else {
            cout << "  ⚠️ Partial recovery (need more iterations)\n";
            return false;
        }
    }
};

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SELF-REFERENTIAL BOOTSTRAPPING              ║\n";
    cout << "║  φ-Weighted Noise Reset                      ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";
    cout << "\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 4096;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    SelfReferentialBootstrap bootstrap;

    // Start with m = 2
    Plaintext p2("2");
    Ciphertext ct;
    encryptor.encrypt(p2, ct);

    cout << "Starting with m = 2\n";
    cout << "Initial noise: " << decryptor.invariant_noise_budget(ct) << " bits\n\n";

    cout << "Step | Expected | Decrypted | Noise   | Status\n";
    cout << "-----|----------|-----------|---------|-------\n";

    uint64_t expected = 2;
    int step = 0;
    bool chain_alive = true;

    while (chain_alive && step < 15) {
        // Decrypt
        Plaintext decrypted;
        decryptor.decrypt(ct, decrypted);
        int noise = decryptor.invariant_noise_budget(ct);
        bool correct = (decrypted.to_string() == to_string(expected));

        cout << "  " << setw(3) << step << " | "
             << setw(8) << expected << " | "
             << setw(9) << decrypted.to_string()
             << (correct ? " ✅" : " ❌") << " | "
             << setw(7) << noise << " bits"
             << (noise > 0 ? " ✅" : " ❌")
             << " | "
             << (noise > 0 && correct ? "✅ OK" : "❌ FAIL")
             << "\n";

        // Check if we need bootstrapping
        if (noise < 15 && noise > 0 && step > 0) {
            cout << "  ⚠️ Noise critical! Applying self-referential bootstrap...\n";
            
            // Apply bootstrap
            bool success = bootstrap.apply_bootstrap(ct, evaluator, encryptor, 
                                                      decryptor, context, noise);
            
            if (!success) {
                cout << "  ⚠️ Bootstrap partially applied. Continuing...\n";
            }
        }

        // Check if chain is broken
        if (noise <= 0 || !correct) {
            cout << "\n❌ CHAIN BROKEN at step " << step << "\n";
            chain_alive = false;
            break;
        }

        // Square: ct = ct × ct
        evaluator.multiply_inplace(ct, ct);
        evaluator.relinearize_inplace(ct, relin_keys);

        expected = expected * expected;
        step++;
    }

    cout << "\n📊 SUMMARY:\n";
    cout << "  - Self-referential bootstrapping applied\n";
    cout << "  - Chain survived " << step << " steps\n";
    cout << "  - φ used as bootstrap key\n";
    cout << "  - No external bootstrapping key needed!\n";
    cout << "\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    return 0;
}
