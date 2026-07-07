/*
 * KEY SWITCHING + ZANS HYBRID
 * 
 * Test kung ang key switching + ZANS ay pwedeng
 * mag-extend ng computation depth.
 * 
 * Tests:
 * 1. Pure Squaring (baseline)
 * 2. Squaring + Key Switch
 * 3. Squaring + Key Switch + ZANS stabilization
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace seal;
using namespace std;

void print_header(const string& title) {
    cout << "\n" << string(65, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(65, '=') << "\n\n";
}

int main() {
    print_header("KEY SWITCHING + ZANS HYBRID");
    
    // Setup SEAL
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
    
    // Create a second key for key switching
    KeyGenerator keygen2(context);
    SecretKey sk2 = keygen2.secret_key();
    GaloisKeys gal_keys;
    // We'll use relinearization keys as our "key switch" mechanism
    
    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder batch_encoder(context);
    
    Plaintext pt_val("42");
    Plaintext pt_zero("0");
    
    // ==========================================
    // TEST 1: PURE SQUARING (BASELINE)
    // ==========================================
    print_header("TEST 1: Pure Squaring (Baseline)");
    
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    int noise1 = decryptor.invariant_noise_budget(ct1);
    cout << "  Start: " << noise1 << " bits\n\n";
    
    int step1 = 0;
    bool alive = true;
    
    while (alive && step1 < 20) {
        step1++;
        int before = decryptor.invariant_noise_budget(ct1);
        
        Ciphertext ct_copy = ct1;
        evaluator.multiply_inplace(ct1, ct_copy);
        evaluator.relinearize_inplace(ct1, relin_keys);
        
        int after = decryptor.invariant_noise_budget(ct1);
        
        cout << "  Step " << setw(2) << step1 << ": " << setw(3) << before 
             << " -> " << setw(3) << after;
        
        if (after <= 0) {
            cout << " DEAD";
            alive = false;
        }
        cout << "\n";
    }
    cout << "  Steps survived: " << (step1 - 1) << "\n\n";
    
    // ==========================================
    // TEST 2: SQUARING + RELIN (Key Switch)
    // ==========================================
    print_header("TEST 2: Squaring + Relinearization");
    cout << "  Note: Relinearization IS a form of key switching.\n";
    cout << "  It switches from product key back to original key.\n\n";
    
    // Relinearization is already applied in Test 1.
    // Let's test WITHOUT relinearization to see the difference.
    
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    int noise2_start = decryptor.invariant_noise_budget(ct2);
    cout << "  Start: " << noise2_start << " bits\n\n";
    
    int step2 = 0;
    alive = true;
    
    while (alive && step2 < 20) {
        step2++;
        int before = decryptor.invariant_noise_budget(ct2);
        
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        // NO relinearization - mas malaki ang ciphertext
        
        int after = decryptor.invariant_noise_budget(ct2);
        
        cout << "  Step " << setw(2) << step2 << ": " << setw(3) << before 
             << " -> " << setw(3) << after;
        
        // Check if still decryptable
        try {
            Plaintext pt_test;
            decryptor.decrypt(ct2, pt_test);
            if (after <= 0) {
                cout << " DEAD (noise=0)";
                alive = false;
            }
        } catch (...) {
            cout << " DEAD (decrypt failed)";
            alive = false;
        }
        cout << "\n";
    }
    cout << "  Steps survived: " << (step2 - 1) << "\n\n";
    
    // ==========================================
    // TEST 3: SQUARING + RELIN + ZANS
    // ==========================================
    print_header("TEST 3: Squaring + Relin + ZANS Hybrid");
    cout << "  After each squaring: relinearize + ZANS stabilize\n\n";
    
    Ciphertext ct3(context);
    encryptor.encrypt(pt_val, ct3);
    int noise3 = decryptor.invariant_noise_budget(ct3);
    cout << "  Start: " << noise3 << " bits\n\n";
    
    int step3 = 0;
    alive = true;
    
    while (alive && step3 < 20) {
        step3++;
        int before = decryptor.invariant_noise_budget(ct3);
        
        // Square
        Ciphertext ct_copy = ct3;
        evaluator.multiply_inplace(ct3, ct_copy);
        evaluator.relinearize_inplace(ct3, relin_keys);
        
        int after_mul = decryptor.invariant_noise_budget(ct3);
        
        // ZANS stabilize - 10 Enc(0) additions
        Ciphertext ct_zero(context);
        for (int i = 0; i < 10; i++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct3, ct_zero);
        }
        
        int after_zans = decryptor.invariant_noise_budget(ct3);
        
        cout << "  Step " << setw(2) << step3 << ": " << setw(3) << before 
             << " -> " << setw(3) << after_mul << " (mul) -> " 
             << setw(3) << after_zans << " (zans)";
        
        if (after_zans <= 0) {
            cout << " DEAD";
            alive = false;
        }
        cout << "\n";
    }
    cout << "  Steps survived: " << (step3 - 1) << "\n\n";
    
    // ==========================================
    // TEST 4: MODSWITCH INSTEAD OF RELIN
    // ==========================================
    print_header("TEST 4: Squaring + ModSwitch + ZANS");
    
    // Try with modswitch as alternative key switching
    try {
        Ciphertext ct4(context);
        encryptor.encrypt(pt_val, ct4);
        
        cout << "  Start: " << decryptor.invariant_noise_budget(ct4) << " bits\n\n";
        
        int step4 = 0;
        alive = true;
        
        while (alive && step4 < 20) {
            step4++;
            int before = decryptor.invariant_noise_budget(ct4);
            
            // Square
            Ciphertext ct_copy = ct4;
            evaluator.multiply_inplace(ct4, ct_copy);
            evaluator.relinearize_inplace(ct4, relin_keys);
            
            int after_mul = decryptor.invariant_noise_budget(ct4);
            
            // ModSwitch
            try {
                evaluator.mod_switch_to_next_inplace(ct4);
            } catch (...) {
                // modswitch chain ended
            }
            
            int after_mod = decryptor.invariant_noise_budget(ct4);
            
            // ZANS after modswitch
            Ciphertext ct_zero2(context);
            for (int i = 0; i < 10; i++) {
                encryptor.encrypt(pt_zero, ct_zero2);
                evaluator.add_inplace(ct4, ct_zero2);
            }
            
            int after_zans = decryptor.invariant_noise_budget(ct4);
            
            cout << "  Step " << setw(2) << step4 << ": " << setw(3) << before 
                 << " -> " << setw(3) << after_mul 
                 << " -> " << setw(3) << after_mod << " (mod) -> "
                 << setw(3) << after_zans << " (zans)";
            
            if (after_zans <= 0) {
                cout << " DEAD";
                alive = false;
            }
            cout << "\n";
        }
        cout << "  Steps survived: " << (step4 - 1) << "\n\n";
        
    } catch (const exception& e) {
        cout << "  Error: " << e.what() << "\n\n";
    }
    
    // ==========================================
    // COMPARISON
    // ==========================================
    print_header("COMPARISON");
    cout << "  " << left << setw(35) << "Method"
         << setw(15) << "Steps Survived" << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(35) << "Pure Squaring + Relin"
         << setw(15) << (step1 - 1) << "\n";
    cout << "  " << left << setw(35) << "Squaring NO Relin"
         << setw(15) << (step2 - 1) << "\n";
    cout << "  " << left << setw(35) << "Squaring + Relin + ZANS"
         << setw(15) << (step3 - 1) << "\n\n";
    
    cout << "  Key insight:\n";
    cout << "  - Relinearization IS key switching\n";
    cout << "  - It reduces ciphertext size after multiplication\n";
    cout << "  - Without it, ciphertext grows and becomes unusable\n";
    cout << "  - ZANS addition after relin may help stabilize\n\n";
    
    return 0;
}
