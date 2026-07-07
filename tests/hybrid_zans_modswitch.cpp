/*
 * HYBRID ZANS-M + MODULUS SWITCHING
 * 
 * Test 1: Squaring + ModSwitch
 * Test 2: ZANS-M (×Enc(1)) + ModSwitch
 * Test 3: Pure Squaring (baseline)
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <chrono>
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
    print_header("HYBRID ZANS-M + MODULUS SWITCHING");
    
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
    BatchEncoder batch_encoder(context);
    
    Plaintext pt_val("42");
    Ciphertext fresh_ct(context);
    encryptor.encrypt(pt_val, fresh_ct);
    int fresh_noise = decryptor.invariant_noise_budget(fresh_ct);
    cout << "  N = " << poly_modulus_degree << "\n";
    cout << "  Fresh noise budget: " << fresh_noise << " bits\n\n";
    
    // ==========================================
    // TEST 1: SQUARING + MODSWITCH
    // ==========================================
    print_header("TEST 1: Squaring + ModSwitch");
    cout << "  ct = modswitch(ct x ct)\n\n";
    
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    bool alive = true;
    int step1 = 0;
    
    while (alive && step1 < 50) {
        step1++;
        int noise_before = decryptor.invariant_noise_budget(ct1);
        Ciphertext ct_copy = ct1;
        evaluator.multiply_inplace(ct1, ct_copy);
        evaluator.relinearize_inplace(ct1, relin_keys);
        int noise_after = decryptor.invariant_noise_budget(ct1);
        
        try {
            evaluator.mod_switch_to_next_inplace(ct1);
            int noise_after_mod = decryptor.invariant_noise_budget(ct1);
            if (noise_after_mod <= 0) alive = false;
        } catch (...) {
            alive = false;
        }
        
        if (step1 <= 10 || step1 % 5 == 0) {
            cout << "  Step " << setw(2) << step1 << ": before=" << setw(3) << noise_before
                 << " after=" << setw(3) << noise_after
                 << " modswitch=" << setw(3) << decryptor.invariant_noise_budget(ct1);
            if (!alive) cout << " DEAD";
            cout << "\n";
        }
    }
    cout << "  Total steps survived: " << (step1 - 1) << "\n";
    
    // ==========================================
    // TEST 2: ZANS-M + MODSWITCH
    // ==========================================
    print_header("TEST 2: ZANS-M (xEnc(1)) + ModSwitch");
    cout << "  ct = modswitch(ct x Enc(1))\n\n";
    
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    Plaintext pt_one("1");
    alive = true;
    int step2 = 0;
    
    while (alive && step2 < 50) {
        step2++;
        int noise_before = decryptor.invariant_noise_budget(ct2);
        Ciphertext ct_one(context);
        encryptor.encrypt(pt_one, ct_one);
        evaluator.multiply_inplace(ct2, ct_one);
        evaluator.relinearize_inplace(ct2, relin_keys);
        int noise_after = decryptor.invariant_noise_budget(ct2);
        
        try {
            evaluator.mod_switch_to_next_inplace(ct2);
            int noise_after_mod = decryptor.invariant_noise_budget(ct2);
            if (noise_after_mod <= 0) alive = false;
        } catch (...) {
            alive = false;
        }
        
        if (step2 <= 10 || step2 % 5 == 0) {
            cout << "  Step " << setw(2) << step2 << ": before=" << setw(3) << noise_before
                 << " after=" << setw(3) << noise_after
                 << " modswitch=" << setw(3) << decryptor.invariant_noise_budget(ct2);
            if (!alive) cout << " DEAD";
            cout << "\n";
        }
    }
    cout << "  Total steps survived: " << (step2 - 1) << "\n";
    
    // ==========================================
    // TEST 3: PURE SQUARING (BASELINE)
    // ==========================================
    print_header("TEST 3: Pure Squaring (Baseline)");
    cout << "  ct = ct x ct (no modswitch)\n\n";
    
    Ciphertext ct3(context);
    encryptor.encrypt(pt_val, ct3);
    alive = true;
    int step3 = 0;
    
    while (alive && step3 < 50) {
        step3++;
        int noise_before = decryptor.invariant_noise_budget(ct3);
        Ciphertext ct_copy = ct3;
        evaluator.multiply_inplace(ct3, ct_copy);
        evaluator.relinearize_inplace(ct3, relin_keys);
        int noise_after = decryptor.invariant_noise_budget(ct3);
        
        alive = (noise_after > 0);
        
        if (step3 <= 10 || step3 % 5 == 0) {
            cout << "  Step " << setw(2) << step3 << ": before=" << setw(3) << noise_before
                 << " after=" << setw(3) << noise_after;
            if (!alive) cout << " DEAD";
            cout << "\n";
        }
    }
    cout << "  Total steps survived: " << (step3 - 1) << "\n";
    
    // ==========================================
    // COMPARISON
    // ==========================================
    print_header("COMPARISON");
    
    cout << "  " << left << setw(30) << "Method"
         << setw(15) << "Steps Survived" << "\n";
    cout << "  " << string(45, '-') << "\n";
    cout << "  " << left << setw(30) << "Squaring + ModSwitch"
         << setw(15) << (step1 - 1) << "\n";
    cout << "  " << left << setw(30) << "ZANS-M + ModSwitch"
         << setw(15) << (step2 - 1) << "\n";
    cout << "  " << left << setw(30) << "Pure Squaring"
         << setw(15) << (step3 - 1) << "\n\n";
    
    if (step2 > step1 && step2 > step3) {
        cout << "  ✅ ZANS-M + ModSwitch WINS! More steps survived.\n";
    } else if (step1 > step2 && step1 > step3) {
        cout << "  ✅ Squaring + ModSwitch WINS!\n";
    } else if (step3 > step1 && step3 > step2) {
        cout << "  ✅ Pure Squaring WINS! (ModSwitch not helping)\n";
    } else if (step1 == step2 && step2 == step3) {
        cout << "  ⚠️ All methods survived the same number of steps.\n";
    } else {
        cout << "  ⚠️ Mixed/tie results.\n";
    }
    
    cout << "\n";
    return 0;
}
