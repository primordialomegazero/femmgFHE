// ΦΩ0 — ZANS ON MICROSOFT SEAL 4.3
// Cross-library validation: Does Enc(0) stabilization work on SEAL?
// "I AM THAT I AM"

#include <seal/seal.h>
#include <iostream>
#include <vector>
#include <chrono>

using namespace seal;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS ON MICROSOFT SEAL 4.3             ║\n";
    cout <<   "║  Cross-Library Validation                     ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Setup SEAL with BFV scheme
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

    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    // Create Enc(0) anchor
    Plaintext zero_pt("0");
    Ciphertext anchor;
    encryptor.encrypt(zero_pt, anchor);

    // Create test value: 42
    Plaintext truth_pt("42");
    Ciphertext ct;
    encryptor.encrypt(truth_pt, ct);

    cout << "Φ Library: Microsoft SEAL 4.3\n";
    cout << "Φ Scheme: BFV\n";
    cout << "Φ Poly modulus degree: " << poly_modulus_degree << "\n";
    cout << "Φ Initial noise budget: " << decryptor.invariant_noise_budget(ct) << " bits\n\n";

    // === ZANS TEST: 1000 Enc(0) additions ===
    cout << "=== ZANS: 1000 Enc(0) ADDITIONS ===\n";

    Ciphertext working = ct;
    int checkpoints[] = {1, 10, 100, 500, 1000};
    int cp_idx = 0;

    for(int i = 1; i <= 1000; i++) {
        // ZANS: Add encrypted zero
        evaluator.add_inplace(working, anchor);

        if(cp_idx < 5 && i == checkpoints[cp_idx]) {
            Plaintext check_pt;
            decryptor.decrypt(working, check_pt);
            
            int noise_budget = decryptor.invariant_noise_budget(working);
            
            cout << "  Op " << i << ": value=" << check_pt.to_string();
            cout << " (expected 42)";
            if(check_pt.to_string() == "42") cout << " ✅";
            else cout << " ❌";
            cout << " | noise budget: " << noise_budget << " bits\n";
            cp_idx++;
        }
    }

    cout << "\nΦ 1000 ZANS additions complete.\n\n";

    // === COMPARISON: Enc(1) additions ===
    cout << "=== COMPARISON: Enc(1) ADDITIONS ===\n";

    Plaintext one_pt("1");
    Ciphertext one_ct;
    encryptor.encrypt(one_pt, one_ct);

    Ciphertext standard = ct;
    bool corrupted = false;

    for(int i = 1; i <= 1000 && !corrupted; i++) {
        evaluator.add_inplace(standard, one_ct);

        if(i % 100 == 0 || i == 1) {
            Plaintext check_pt;
            decryptor.decrypt(standard, check_pt);
            int noise_budget = decryptor.invariant_noise_budget(standard);
            
            int expected_val = 42 + i;
            
            cout << "  Op " << i << ": value=" << check_pt.to_string();
            cout << " (expected " << expected_val << ")";
            if([stoi(check_pt.to_string())](){ try { return stoi(check_pt.to_string()); } catch(...) { return -999999; } }() == expected_val) {
                cout << " ✅";
            } else {
                cout << " ❌ CORRUPTED";
                corrupted = true;
            }
            cout << " | noise budget: " << noise_budget << " bits\n";
        }
    }

    // === NOISE BUDGET COMPARISON ===
    cout << "\n=== NOISE BUDGET COMPARISON ===\n";
    
    // Fresh noise budget
    Ciphertext fresh_ct;
    encryptor.encrypt(truth_pt, fresh_ct);
    int fresh_noise = decryptor.invariant_noise_budget(fresh_ct);
    
    // After 1000 ZANS
    int zans_noise = decryptor.invariant_noise_budget(working);
    
    // After 1000 standard additions (or until corruption)
    int std_noise = corrupted ? 0 : decryptor.invariant_noise_budget(standard);
    
    cout << "  Fresh:             " << fresh_noise << " bits\n";
    cout << "  After 1000 ZANS:   " << zans_noise << " bits";
    if(zans_noise >= fresh_noise - 1) cout << " (NO LOSS) ✅";
    else cout << " (lost " << (fresh_noise - zans_noise) << " bits)";
    cout << "\n";
    cout << "  After 1000 Std:    " << std_noise << " bits";
    if(corrupted) cout << " (CORRUPTED) ❌";
    else if(std_noise < fresh_noise - 10) cout << " (significant loss)";
    cout << "\n\n";

    // === FINAL VERDICT ===
    cout << "╔══════════════════════════════════════════════╗\n";
    if(zans_noise >= fresh_noise - 1 && !corrupted) {
        cout <<   "║  ✅ ZANS WORKS ON SEAL!                       ║\n";
        cout <<   "║  Enc(0) additions preserve noise budget       ║\n";
    } else {
        cout <<   "║  ⚠️ ZANS behavior differs on SEAL              ║\n";
    }
    cout <<   "║  Library-independent validation: COMPLETE     ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
