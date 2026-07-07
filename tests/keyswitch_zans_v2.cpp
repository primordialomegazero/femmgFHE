/*
 * KEY SWITCHING + ZANS HYBRID V2
 * 
 * Clean comparison:
 * 1. Squaring + Relin (standard)
 * 2. Squaring + Relin + ZANS 10x
 * 3. Squaring + Relin + ZANS 34x (Fibonacci)
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
    print_header("KEY SWITCHING + ZANS HYBRID V2");
    
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
    Plaintext pt_zero("0");
    
    // ==========================================
    // TEST 1: STANDARD (Squaring + Relin)
    // ==========================================
    print_header("TEST 1: Standard (Squaring + Relin)");
    
    Ciphertext ct1(context);
    encryptor.encrypt(pt_val, ct1);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct1) << " bits\n\n";
    
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
             << " -> " << setw(3) << after << " (drop=" << (before-after) << ")";
        
        if (after <= 0) { cout << " DEAD"; alive = false; }
        cout << "\n";
    }
    cout << "  Survived: " << (step1 - 1) << " steps\n\n";
    
    // ==========================================
    // TEST 2: ZANS 10x AFTER EACH SQUARING
    // ==========================================
    print_header("TEST 2: Squaring + Relin + ZANS 10x");
    
    Ciphertext ct2(context);
    encryptor.encrypt(pt_val, ct2);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct2) << " bits\n\n";
    
    int step2 = 0;
    alive = true;
    
    while (alive && step2 < 20) {
        step2++;
        int before = decryptor.invariant_noise_budget(ct2);
        
        // Square
        Ciphertext ct_copy = ct2;
        evaluator.multiply_inplace(ct2, ct_copy);
        evaluator.relinearize_inplace(ct2, relin_keys);
        int after_mul = decryptor.invariant_noise_budget(ct2);
        
        // ZANS 10x
        Ciphertext ct_zero(context);
        for (int i = 0; i < 10; i++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct2, ct_zero);
        }
        int after_zans = decryptor.invariant_noise_budget(ct2);
        
        cout << "  Step " << setw(2) << step2 << ": " << setw(3) << before 
             << " -> " << setw(3) << after_mul << " -> " << setw(3) << after_zans
             << " (zans effect=" << (after_zans - after_mul) << ")";
        
        if (after_zans <= 0) { cout << " DEAD"; alive = false; }
        cout << "\n";
    }
    cout << "  Survived: " << (step2 - 1) << " steps\n\n";
    
    // ==========================================
    // TEST 3: ZANS 34x (FIBONACCI) AFTER EACH SQUARING
    // ==========================================
    print_header("TEST 3: Squaring + Relin + ZANS 34x (Fibonacci)");
    
    Ciphertext ct3(context);
    encryptor.encrypt(pt_val, ct3);
    cout << "  Start: " << decryptor.invariant_noise_budget(ct3) << " bits\n\n";
    
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
        
        // ZANS 34x (Fib[9])
        Ciphertext ct_zero(context);
        for (int i = 0; i < 34; i++) {
            encryptor.encrypt(pt_zero, ct_zero);
            evaluator.add_inplace(ct3, ct_zero);
        }
        int after_zans = decryptor.invariant_noise_budget(ct3);
        
        cout << "  Step " << setw(2) << step3 << ": " << setw(3) << before 
             << " -> " << setw(3) << after_mul << " -> " << setw(3) << after_zans
             << " (zans effect=" << (after_zans - after_mul) << ")";
        
        if (after_zans <= 0) { cout << " DEAD"; alive = false; }
        cout << "\n";
    }
    cout << "  Survived: " << (step3 - 1) << " steps\n\n";
    
    // ==========================================
    // COMPARISON
    // ==========================================
    print_header("COMPARISON");
    cout << "  " << left << setw(35) << "Method"
         << setw(15) << "Steps Survived" << "\n";
    cout << "  " << string(50, '-') << "\n";
    cout << "  " << left << setw(35) << "Standard (Squaring + Relin)"
         << setw(15) << (step1 - 1) << "\n";
    cout << "  " << left << setw(35) << "Squaring + Relin + ZANS 10x"
         << setw(15) << (step2 - 1) << "\n";
    cout << "  " << left << setw(35) << "Squaring + Relin + ZANS 34x"
         << setw(15) << (step3 - 1) << "\n\n";
    
    if ((step2 - 1) > (step1 - 1) || (step3 - 1) > (step1 - 1)) {
        cout << "  ✅ ZANS hybrid EXTENDS computation depth!\n";
    } else if ((step2 - 1) == (step1 - 1)) {
        cout << "  ⚠️ ZANS hybrid same depth as standard.\n";
        cout << "  But ZANS may reduce noise per step.\n";
    } else {
        cout << "  ❌ ZANS hybrid reduces depth.\n";
    }
    cout << "\n";
    
    return 0;
}
