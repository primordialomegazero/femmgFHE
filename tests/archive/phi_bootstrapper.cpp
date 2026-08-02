#include "seal/seal.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace seal;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PHI_INV = 0.6180339887498948482;
const double NOISE_FIXED = 1.82815;

void self_referential_bootstrap(Ciphertext &ct,
                                 Evaluator &evaluator,
                                 Encryptor &encryptor,
                                 Decryptor &decryptor,
                                 RelinKeys &relin_keys,
                                 const SEALContext &context,
                                 int target_noise = 50) {
    
    int initial_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Bootstrap start: " << initial_noise << " bits → target: " << target_noise << " bits\n";
    
    int deficit = max(0, target_noise - initial_noise);
    int zans_iterations = static_cast<int>(deficit / PHI_INV);
    if (zans_iterations > 500) zans_iterations = 500;
    
    cout << "  Applying " << zans_iterations << " ZANS iterations...\n";
    
    Plaintext pt_zero("0");
    for (int i = 0; i < zans_iterations; i++) {
        Ciphertext ct_zero(context, ct.parms_id());
        encryptor.encrypt(pt_zero, ct_zero);
        evaluator.add_inplace(ct, ct_zero);
    }
    
    int after_zans = decryptor.invariant_noise_budget(ct);
    cout << "  After ZANS: " << after_zans << " bits\n";
    
    // Self-referential fold
    Ciphertext ct_copy = ct;
    evaluator.multiply_inplace(ct, ct_copy);
    evaluator.relinearize_inplace(ct, relin_keys);
    
    int after_fold = decryptor.invariant_noise_budget(ct);
    cout << "  After fold: " << after_fold << " bits\n";
    
    // Recovery ZANS (Fib 34)
    for (int i = 0; i < 34; i++) {
        Ciphertext ct_zero(context, ct.parms_id());
        encryptor.encrypt(pt_zero, ct_zero);
        evaluator.add_inplace(ct, ct_zero);
    }
    
    int final_noise = decryptor.invariant_noise_budget(ct);
    cout << "  Final noise: " << final_noise << " bits\n";
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SELF-REFERENTIAL BOOTSTRAPPER                ║\n";
    cout << "║  φ = 1 + 1/φ → Regeneration through folding  ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(PlainModulus::Batching(8192, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);

    Plaintext pt_val("42");
    Ciphertext ct(context);
    encryptor.encrypt(pt_val, ct);

    int start_noise = decryptor.invariant_noise_budget(ct);
    cout << "Initial noise: " << start_noise << " bits\n\n";

    // Test 1: Bootstrap at full noise
    cout << "=== TEST 1: Bootstrap at full noise ===\n";
    Ciphertext ct1 = ct;
    self_referential_bootstrap(ct1, evaluator, encryptor, decryptor, relin_keys, context, 100);
    cout << "\n";

    // Test 2: Square to exhaustion, then bootstrap
    cout << "=== TEST 2: Square to exhaustion, then bootstrap ===\n";
    Ciphertext ct2 = ct;
    
    int steps = 0;
    while (decryptor.invariant_noise_budget(ct2) > 30 && steps < 5) {
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        steps++;
    }
    cout << "  After " << steps << " squarings: " << decryptor.invariant_noise_budget(ct2) << " bits\n";
    
    self_referential_bootstrap(ct2, evaluator, encryptor, decryptor, relin_keys, context, 80);
    
    int extra_steps = 0;
    while (decryptor.invariant_noise_budget(ct2) > 0 && extra_steps < 5) {
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        extra_steps++;
        cout << "  Post-bootstrap square " << extra_steps << ": " << decryptor.invariant_noise_budget(ct2) << " bits\n";
    }
    cout << "\n";

    // Test 3: Fibonacci-spaced bootstrapping
    cout << "=== TEST 3: Fibonacci-spaced bootstrapping ===\n";
    Ciphertext ct3 = ct;
    vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    Plaintext pt_zero("0");
    
    for (int i = 0; i < 8 && decryptor.invariant_noise_budget(ct3) > 0; i++) {
        Ciphertext ct_copy = ct3;
        evaluator.multiply_inplace(ct3, ct_copy);
        evaluator.relinearize_inplace(ct3, relin_keys);
        
        int noise = decryptor.invariant_noise_budget(ct3);
        cout << "  Step " << (i+1) << " noise: " << noise;
        
        if (noise > 0 && i < (int)fib.size()) {
            for (int j = 0; j < fib[i]; j++) {
                Ciphertext ct_zero(context, ct.parms_id());
                encryptor.encrypt(pt_zero, ct_zero);
                evaluator.add_inplace(ct3, ct_zero);
            }
            cout << " → " << decryptor.invariant_noise_budget(ct3) << " (Fib-ZANS " << fib[i] << ")";
        }
        cout << "\n";
        if (noise <= 0) break;
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  φ = 1 + 1/φ → Self-reference = Regeneration ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
