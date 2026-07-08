#include "seal/seal.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  MODULUS CYCLING — Simple Down Switching     ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    // Standard BFV params (works with modswitch)
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(PlainModulus::Batching(8192, 20));
    SEALContext context(parms);

    cout << "  Coeff modulus size: " << parms.coeff_modulus().size() << "\n";
    cout << "  Chain indices available:\n";
    
    // Check available chain indices
    auto ctx_data = context.key_context_data();
    int idx = 0;
    while (ctx_data) {
        cout << "    Level " << idx << ": " << ctx_data->total_coeff_modulus_bit_count() << " bits\n";
        ctx_data = ctx_data->next_context_data();
        idx++;
    }
    cout << "  Total levels: " << idx << "\n\n";

    if (idx < 2) {
        cout << "  ⚠️ Only 1 level — modulus switching not possible\n";
        cout << "  Need parameters with multiple primes in coeff_modulus\n\n";
        return 0;
    }

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

    // === TEST 1: Standard ===
    cout << "=== TEST 1: Standard Squaring ===\n";
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct1) << " bits\n";
    
    int s1 = 0;
    for (int i = 1; i <= 6; i++) {
        Ciphertext ct_copy = ct1;
        evaluator.multiply_inplace(ct1, ct_copy);
        evaluator.relinearize_inplace(ct1, relin_keys);
        int noise = decryptor.invariant_noise_budget(ct1);
        cout << "  S" << i << ": " << noise;
        if (noise > 0) s1++; 
        if (noise <= 0) { cout << " DEAD\n"; break; }
        cout << "\n";
    }
    cout << "  Survived: " << s1 << "\n\n";

    // === TEST 2: ModSwitch DOWN after multiply ===
    cout << "=== TEST 2: ModSwitch DOWN after each square ===\n";
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct2) << " bits\n";
    
    int s2 = 0;
    for (int i = 1; i <= 8; i++) {
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        
        int noise_before = decryptor.invariant_noise_budget(ct2);
        
        try {
            evaluator.mod_switch_to_next_inplace(ct2);
            int noise_after = decryptor.invariant_noise_budget(ct2);
            cout << "  S" << i << ": " << noise_before << " → " << noise_after;
            if (noise_after > 0) s2++;
            else { cout << " DEAD\n"; break; }
            cout << "\n";
        } catch (const exception& e) {
            cout << "  S" << i << ": " << noise_before << " → CAN'T SWITCH\n";
            if (noise_before > 0) s2++;
            break;
        }
    }
    cout << "  Survived: " << s2 << "\n\n";

    // === COMPARISON ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  Standard:  " << s1 << " steps\n";
    cout << "║  ModSwitch: " << s2 << " steps\n";
    if (s2 > s1) cout << "║  ✅ MODSWITCH EXTENDS DEPTH!                  ║\n";
    else cout << "║  ⚠️ ModSwitch same or worse                    ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
