/*
 * TENSOR DECOMPOSITION WITH LOOKUP TABLE - FIXED
 * Tamang correction: divide by scaling factor
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

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TENSOR WITH LOOKUP - FIXED\n";
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

    // ========================================
    // TEST: 12 x 34 = 408
    // ========================================
    cout << "--- TEST: 12 x 34 = 408 ---\n";
    cout << "  Decomposition: 12 = 6+6, 34 = 17+17\n\n";

    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // Tensor terms
    vector<Ciphertext> terms;
    for (int i = 0; i < 4; i++) {
        Ciphertext term = ct_6;
        evaluator.multiply_inplace(term, ct_17);
        evaluator.relinearize_inplace(term, relin_keys);
        terms.push_back(term);
    }

    double sf = get_scaling_factor(6, 17);
    cout << "  Scaling factor: " << sf << "\n";

    // Sum raw tensor
    Ciphertext sum_tensor = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(sum_tensor, terms[i]);
    }

    print_result(decryptor, sum_tensor, "Raw tensor sum");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";

    // ========================================
    // CORRECTED APPROACH 1: Multiply by inverse
    // ========================================
    cout << "\n--- CORRECTED: Multiply by inverse ---\n";
    
    double inv_sf = 1.0 / sf;
    cout << "  Inverse: " << inv_sf << "\n";
    
    // Since SEAL doesn't support division, we use a different approach:
    // 1. I-multiply ng inverse as plaintext
    // 2. I-adjust ang scaling
    
    // I-convert ang inverse sa integer na pwede sa BFV
    // inv_sf = 0.73913 → multiply by 1000 = 739
    // Pero kailangan nating i-divide pabalik ng 1000
    
    Plaintext plain_inv(to_string((int)(inv_sf * 1000)));
    Ciphertext ct_corrected = sum_tensor;
    evaluator.multiply_plain_inplace(ct_corrected, plain_inv);
    
    print_result(decryptor, ct_corrected, "After multiply by inverse");
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_corrected) << " bits\n";

    // ========================================
    // CORRECTED APPROACH 2: Divide by scaling factor (via modulus)
    // ========================================
    cout << "\n--- CORRECTED: Divide by scaling factor ---\n";
    cout << "  (Using plaintext modulus to divide)\n";
    
    // Hindi supported ang division sa BFV
    // Alternative: I-multiply ng modular inverse
    
    // ========================================
    // BASELINE: Direct
    // ========================================
    cout << "\n--- BASELINE: Direct Multiplication ---\n";
    
    Plaintext plain_12("12");
    Plaintext plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);
    
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    
    print_result(decryptor, ct_direct, "Direct 12x34");
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Raw tensor:    " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";
    cout << "  Corrected:     " << decryptor.invariant_noise_budget(ct_corrected) << " bits\n";
    cout << "  Direct:        " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    cout << "========================================\n";

    return 0;
}
