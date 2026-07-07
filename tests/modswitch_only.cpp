/*
 * MODSWITCH COMPARISON (Fixed)
 * 
 * Test 1: Squaring + ModSwitch
 * Test 2: Pure Squaring (no modswitch)
 * 
 * Simple, walang ZANS-M para iwas mismatch.
 */

#include "seal/seal.h"
#include <iostream>
#include <iomanip>

using namespace seal;
using namespace std;

void print_header(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n\n";
}

int main() {
    print_header("MODSWITCH vs PURE SQUARING");
    
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
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
    
    // ==========================================
    // TEST 1: SQUARING + MODSWITCH
    // ==========================================
    print_header("TEST 1: Squaring + ModSwitch");
    
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    cout << "  Initial noise: " << decryptor.invariant_noise_budget(ct1) << " bits\n\n";
    
    bool alive = true;
    int step1 = 0;
    
    while (alive && step1 < 30) {
        step1++;
        int before = decryptor.invariant_noise_budget(ct1);
        
        // Square
        Ciphertext ct_copy = ct1;
        evaluator.multiply_inplace(ct1, ct_copy);
        evaluator.relinearize_inplace(ct1, relin_keys);
        int after_mul = decryptor.invariant_noise_budget(ct1);
        
        // ModSwitch
        try {
            evaluator.mod_switch_to_next_inplace(ct1);
            int after_mod = decryptor.invariant_noise_budget(ct1);
            
            cout << "  Step " << setw(2) << step1 
                 << ": " << setw(3) << before << " -> " << setw(3) << after_mul 
                 << " -> " << setw(3) << after_mod;
            
            if (after_mod <= 0) {
                cout << " DEAD";
                alive = false;
            }
            cout << "\n";
        } catch (const exception& e) {
            cout << "  Step " << setw(2) << step1 << ": modswitch failed - " << e.what() << "\n";
            alive = false;
        }
    }
    cout << "  Steps survived: " << (step1 - 1) << "\n";
    
    // Verify decryption
    Plaintext result1;
    decryptor.decrypt(ct1, result1);
    cout << "  Final value: " << result1.to_string() << "\n\n";
    
    // ==========================================
    // TEST 2: PURE SQUARING
    // ==========================================
    print_header("TEST 2: Pure Squaring (No ModSwitch)");
    
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    cout << "  Initial noise: " << decryptor.invariant_noise_budget(ct2) << " bits\n\n";
    
    alive = true;
    int step2 = 0;
    
    while (alive && step2 < 30) {
        step2++;
        int before = decryptor.invariant_noise_budget(ct2);
        
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        int after = decryptor.invariant_noise_budget(ct2);
        
        cout << "  Step " << setw(2) << step2 
             << ": " << setw(3) << before << " -> " << setw(3) << after;
        
        if (after <= 0) {
            cout << " DEAD";
            alive = false;
        }
        cout << "\n";
    }
    cout << "  Steps survived: " << (step2 - 1) << "\n";
    
    Plaintext result2;
    decryptor.decrypt(ct2, result2);
    cout << "  Final value: " << result2.to_string() << "\n\n";
    
    // ==========================================
    // COMPARISON
    // ==========================================
    print_header("COMPARISON");
    cout << "  Squaring + ModSwitch: " << (step1 - 1) << " steps survived\n";
    cout << "  Pure Squaring:        " << (step2 - 1) << " steps survived\n\n";
    
    if ((step1 - 1) > (step2 - 1)) {
        cout << "  ✅ ModSwitch EXTENDS computation depth!\n";
    } else if ((step1 - 1) == (step2 - 1)) {
        cout << "  ⚠️ ModSwitch does NOT extend depth (but may reduce noise).\n";
    } else {
        cout << "  ❌ ModSwitch REDUCES depth (unexpected).\n";
    }
    cout << "\n";
    
    return 0;
}
