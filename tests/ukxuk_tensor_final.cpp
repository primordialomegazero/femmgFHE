/*
 * UK×UK WITH TENSOR DECOMPOSITION
 * Final implementation para sa encrypted multiplication
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>

using namespace seal;
using namespace std;

map<pair<int,int>, double> scaling_factors;

void load_lookup_table() {
    ifstream file("scaling_lookup.txt");
    string line;
    bool in_table = false;
    
    while (getline(file, line)) {
        if (line.find("====") != string::npos) {
            in_table = !in_table;
            continue;
        }
        if (!in_table || line.empty()) continue;
        if (line.find("Scaling") != string::npos) continue;
        if (line.find("====") != string::npos) continue;
        if (line.find("     ") != string::npos) continue;
        
        stringstream ss(line);
        int a;
        ss >> a;
        for (int b = 1; b <= 20; b++) {
            double ratio;
            ss >> ratio;
            scaling_factors[{a, b}] = ratio;
        }
    }
}

double get_scaling_factor(int a, int b) {
    auto it = scaling_factors.find({a, b});
    if (it != scaling_factors.end()) {
        return it->second;
    }
    return 1.0;
}

// Tensor decomposition for multiplication
Ciphertext tensor_multiply(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    int a1, int a2, int b1, int b2
) {
    // Decompose: a = a1 + a2, b = b1 + b2
    // a × b = a1×b1 + a1×b2 + a2×b1 + a2×b2
    
    // For now, we assume the values are already encrypted
    // We need to decompose the encrypted values
    // This is the challenge: how to decompose encrypted values?
    
    // For this demo, we'll use plaintext decomposition
    // In practice, we need homomorphic decomposition
    
    cout << "  Tensor decomposition: " << a1 << "+" << a2 << " × " << b1 << "+" << b2 << "\n";
    
    // Since we can't decompose encrypted values directly,
    // this is a proof of concept using plaintext decomposition
    
    // We'll return a dummy ciphertext
    Ciphertext result;
    return result;
}

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  UK×UK WITH TENSOR DECOMPOSITION\n";
    cout << "========================================\n\n";

    load_lookup_table();
    cout << "✅ Loaded " << scaling_factors.size() << " scaling factors\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 30));
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

    // ========================================
    // TEST: 12 x 34 = 408
    // ========================================
    cout << "--- TEST: 12 x 34 = 408 ---\n\n";

    // Encrypt values
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);

    // ========================================
    // APPROACH 1: Direct UK×UK (Baseline)
    // ========================================
    cout << "--- APPROACH 1: Direct UK×UK ---\n";
    
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    print_result(decryptor, ct_direct, "Direct 12x34");
    int noise_direct = decryptor.invariant_noise_budget(ct_direct);
    cout << "  Noise: " << noise_direct << " bits\n\n";

    // ========================================
    // APPROACH 2: Tensor Decomposition (6+6 × 17+17)
    // ========================================
    cout << "--- APPROACH 2: Tensor Decomposition ---\n";
    cout << "  Decomposition: 12 = 6+6, 34 = 17+17\n\n";

    // Since we can't decompose encrypted values yet,
    // we'll use plaintext decomposition for this demo
    
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // Compute tensor terms
    vector<Ciphertext> terms;
    
    // Term 1: 6 × 17
    Ciphertext t1 = ct_6;
    evaluator.multiply_inplace(t1, ct_17);
    evaluator.relinearize_inplace(t1, relin_keys);
    terms.push_back(t1);
    
    // Term 2: 6 × 17
    Ciphertext t2 = ct_6;
    evaluator.multiply_inplace(t2, ct_17);
    evaluator.relinearize_inplace(t2, relin_keys);
    terms.push_back(t2);
    
    // Term 3: 6 × 17
    Ciphertext t3 = ct_6;
    evaluator.multiply_inplace(t3, ct_17);
    evaluator.relinearize_inplace(t3, relin_keys);
    terms.push_back(t3);
    
    // Term 4: 6 × 17
    Ciphertext t4 = ct_6;
    evaluator.multiply_inplace(t4, ct_17);
    evaluator.relinearize_inplace(t4, relin_keys);
    terms.push_back(t4);

    // Apply ZANS to each term
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(terms[i], enc_zero);
        }
    }

    // Sum all terms
    Ciphertext ct_tensor = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(ct_tensor, terms[i]);
        // Apply ZANS after each addition
        for (int j = 0; j < 100; j++) {
            evaluator.add_inplace(ct_tensor, enc_zero);
        }
    }

    print_result(decryptor, ct_tensor, "Tensor result");
    int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
    cout << "  Noise: " << noise_tensor << " bits\n";

    // ========================================
    // APPROACH 3: Tensor with Scaling Correction
    // ========================================
    cout << "\n--- APPROACH 3: Tensor with Scaling Correction ---\n";
    
    double sf = get_scaling_factor(6, 17);
    cout << "  Scaling factor: " << sf << "\n";
    
    // Apply correction
    double inv_sf = 1.0 / sf;
    int inv_int = (int)(inv_sf * 1000);
    cout << "  Inverse: " << inv_sf << " → " << inv_int << "/1000\n";
    
    Plaintext plain_inv(to_string(inv_int));
    Ciphertext ct_corrected = ct_tensor;
    evaluator.multiply_plain_inplace(ct_corrected, plain_inv);
    
    // Apply ZANS after correction
    for (int i = 0; i < 100; i++) {
        evaluator.add_inplace(ct_corrected, enc_zero);
    }
    
    print_result(decryptor, ct_corrected, "Corrected tensor");
    int noise_corrected = decryptor.invariant_noise_budget(ct_corrected);
    cout << "  Noise: " << noise_corrected << " bits\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Direct UK×UK:       " << noise_direct << " bits\n";
    cout << "  Tensor (raw):       " << noise_tensor << " bits\n";
    cout << "  Tensor (corrected): " << noise_corrected << " bits\n";
    cout << "========================================\n";
    cout << "  Improvement: " << (noise_direct - noise_corrected) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}
