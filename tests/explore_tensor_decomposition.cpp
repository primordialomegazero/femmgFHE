/*
 * PHASE 2: TENSOR DECOMPOSITION
 * I-decompose ang multiplication into addition steps
 * Idea: a × b = Σ (a_i × b_i) where a_i, b_i are "smaller" values
 * Then apply ZANS on the sum
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;

void print_noise(const char* label, int noise) {
    std::cout << "  " << label << ": " << noise << " bits\n";
}

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  PHASE 2: TENSOR DECOMPOSITION\n";
    std::cout << "  I-decompose multiplication into additions\n";
    std::cout << "========================================\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
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

    // Pre-compute Enc(0) for ZANS
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // I-encrypt ang test values
    Plaintext plain_a("12");
    Plaintext plain_b("34");
    Ciphertext ct_a, ct_b;
    encryptor.encrypt(plain_a, ct_a);
    encryptor.encrypt(plain_b, ct_b);

    int start_noise_a = decryptor.invariant_noise_budget(ct_a);
    int start_noise_b = decryptor.invariant_noise_budget(ct_b);
    print_noise("Start (Enc(12))", start_noise_a);
    print_noise("Start (Enc(34))", start_noise_b);

    // ========================================
    // APPROACH 1: Direct Multiplication (Baseline)
    // ========================================
    std::cout << "\n--- APPROACH 1: Direct Multiplication (Baseline) ---\n";
    
    Ciphertext ct_direct = ct_a;
    evaluator.multiply_inplace(ct_direct, ct_b);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    int noise_direct = decryptor.invariant_noise_budget(ct_direct);
    print_noise("Direct UKxUK", noise_direct);
    
    // Apply ZANS after direct multiplication
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_direct, enc_zero);
    }
    int noise_direct_zans = decryptor.invariant_noise_budget(ct_direct);
    print_noise("Direct + 1000 ZANS", noise_direct_zans);
    
    Plaintext result_direct;
    decryptor.decrypt(ct_direct, result_direct);
    std::cout << "  Result: " << result_direct.to_string() << " (12 x 34 = 408)\n\n";

    // ========================================
    // APPROACH 2: Tensor Decomposition
    // a × b = (a1 + a2) × (b1 + b2)
    //        = a1×b1 + a1×b2 + a2×b1 + a2×b2
    // I-decompose into smaller multiplications then sum
    // ========================================
    std::cout << "--- APPROACH 2: Tensor Decomposition ---\n";
    std::cout << "  Decompose: 12 = 6 + 6, 34 = 17 + 17\n";
    std::cout << "  12 × 34 = 6×17 + 6×17 + 6×17 + 6×17\n\n";

    // Decompose a into a1 + a2
    Plaintext plain_a1("6");
    Plaintext plain_a2("6");
    Ciphertext ct_a1, ct_a2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);

    // Decompose b into b1 + b2
    Plaintext plain_b1("17");
    Plaintext plain_b2("17");
    Ciphertext ct_b1, ct_b2;
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);

    // Compute each tensor term: a_i × b_j
    std::vector<Ciphertext> tensor_terms;
    
    // Term 1: a1 × b1
    Ciphertext term1 = ct_a1;
    evaluator.multiply_inplace(term1, ct_b1);
    evaluator.relinearize_inplace(term1, relin_keys);
    tensor_terms.push_back(term1);
    
    // Term 2: a1 × b2
    Ciphertext term2 = ct_a1;
    evaluator.multiply_inplace(term2, ct_b2);
    evaluator.relinearize_inplace(term2, relin_keys);
    tensor_terms.push_back(term2);
    
    // Term 3: a2 × b1
    Ciphertext term3 = ct_a2;
    evaluator.multiply_inplace(term3, ct_b1);
    evaluator.relinearize_inplace(term3, relin_keys);
    tensor_terms.push_back(term3);
    
    // Term 4: a2 × b2
    Ciphertext term4 = ct_a2;
    evaluator.multiply_inplace(term4, ct_b2);
    evaluator.relinearize_inplace(term4, relin_keys);
    tensor_terms.push_back(term4);

    // Apply ZANS to each tensor term
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(tensor_terms[i], enc_zero);
        }
    }

    // Sum all tensor terms
    Ciphertext tensor_result = tensor_terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(tensor_result, tensor_terms[i]);
        // Apply ZANS after each addition
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(tensor_result, enc_zero);
        }
    }

    int noise_tensor = decryptor.invariant_noise_budget(tensor_result);
    print_noise("Tensor result", noise_tensor);
    
    Plaintext result_tensor;
    decryptor.decrypt(tensor_result, result_tensor);
    std::cout << "  Result: " << result_tensor.to_string() << " (expected 408)\n\n";

    // ========================================
    // APPROACH 3: Fibonacci-based Tensor Decomposition
    // ========================================
    std::cout << "--- APPROACH 3: Fibonacci-based Tensor Decomposition ---\n";
    std::cout << "  Decompose: 12 = 8 + 3 + 1 (Fibonacci)\n";
    std::cout << "             34 = 21 + 13 (Fibonacci)\n";
    std::cout << "  12 × 34 = (8+3+1) × (21+13)\n\n";

    // Decompose using Fibonacci: 12 = 8 + 3 + 1
    Plaintext plain_fib_a1("8");
    Plaintext plain_fib_a2("3");
    Plaintext plain_fib_a3("1");
    Ciphertext ct_fib_a1, ct_fib_a2, ct_fib_a3;
    encryptor.encrypt(plain_fib_a1, ct_fib_a1);
    encryptor.encrypt(plain_fib_a2, ct_fib_a2);
    encryptor.encrypt(plain_fib_a3, ct_fib_a3);

    // Decompose using Fibonacci: 34 = 21 + 13
    Plaintext plain_fib_b1("21");
    Plaintext plain_fib_b2("13");
    Ciphertext ct_fib_b1, ct_fib_b2;
    encryptor.encrypt(plain_fib_b1, ct_fib_b1);
    encryptor.encrypt(plain_fib_b2, ct_fib_b2);

    // Compute each term
    std::vector<Ciphertext> fib_terms;
    
    // a1 × b1: 8 × 21 = 168
    Ciphertext fib_term1 = ct_fib_a1;
    evaluator.multiply_inplace(fib_term1, ct_fib_b1);
    evaluator.relinearize_inplace(fib_term1, relin_keys);
    fib_terms.push_back(fib_term1);
    
    // a1 × b2: 8 × 13 = 104
    Ciphertext fib_term2 = ct_fib_a1;
    evaluator.multiply_inplace(fib_term2, ct_fib_b2);
    evaluator.relinearize_inplace(fib_term2, relin_keys);
    fib_terms.push_back(fib_term2);
    
    // a2 × b1: 3 × 21 = 63
    Ciphertext fib_term3 = ct_fib_a2;
    evaluator.multiply_inplace(fib_term3, ct_fib_b1);
    evaluator.relinearize_inplace(fib_term3, relin_keys);
    fib_terms.push_back(fib_term3);
    
    // a2 × b2: 3 × 13 = 39
    Ciphertext fib_term4 = ct_fib_a2;
    evaluator.multiply_inplace(fib_term4, ct_fib_b2);
    evaluator.relinearize_inplace(fib_term4, relin_keys);
    fib_terms.push_back(fib_term4);
    
    // a3 × b1: 1 × 21 = 21
    Ciphertext fib_term5 = ct_fib_a3;
    evaluator.multiply_inplace(fib_term5, ct_fib_b1);
    evaluator.relinearize_inplace(fib_term5, relin_keys);
    fib_terms.push_back(fib_term5);
    
    // a3 × b2: 1 × 13 = 13
    Ciphertext fib_term6 = ct_fib_a3;
    evaluator.multiply_inplace(fib_term6, ct_fib_b2);
    evaluator.relinearize_inplace(fib_term6, relin_keys);
    fib_terms.push_back(fib_term6);

    // Apply ZANS to each term
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(fib_terms[i], enc_zero);
        }
    }

    // Sum all terms
    Ciphertext fib_result = fib_terms[0];
    for (int i = 1; i < 6; i++) {
        evaluator.add_inplace(fib_result, fib_terms[i]);
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(fib_result, enc_zero);
        }
    }

    int noise_fib_tensor = decryptor.invariant_noise_budget(fib_result);
    print_noise("Fibonacci Tensor result", noise_fib_tensor);
    
    Plaintext result_fib_tensor;
    decryptor.decrypt(fib_result, result_fib_tensor);
    std::cout << "  Result: " << result_fib_tensor.to_string() << " (expected 408)\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    std::cout << "========================================\n";
    std::cout << "  PHASE 2 SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "  Direct UKxUK:         " << noise_direct << " bits\n";
    std::cout << "  Direct + ZANS:        " << noise_direct_zans << " bits\n";
    std::cout << "  Tensor Decomp:        " << noise_tensor << " bits\n";
    std::cout << "  Fibonacci Tensor:     " << noise_fib_tensor << " bits\n";
    std::cout << "========================================\n";
    std::cout << "  Finding: ";
    if (noise_tensor < noise_direct) {
        std::cout << "Tensor decomposition improves noise!\n";
    } else if (noise_tensor == noise_direct) {
        std::cout << "Tensor decomposition does NOT improve noise.\n";
    } else {
        std::cout << "Tensor decomposition makes noise worse.\n";
    }
    std::cout << "========================================\n\n";

    return 0;
}
