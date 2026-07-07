/*
 * TENSOR DECOMPOSITION - COMPLETE FIX
 * I-account ang lahat ng scaling factors
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
    cout << "  TENSOR DECOMPOSITION - COMPLETE FIX\n";
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
    // I-determine ang scaling factor ng multiply_plain
    // ========================================
    cout << "--- Determine multiply_plain scaling ---\n";
    
    Plaintext plain_1("1");
    Ciphertext ct_1;
    encryptor.encrypt(plain_1, ct_1);
    
    // I-multiply ng 1
    Ciphertext ct_1x1 = ct_1;
    evaluator.multiply_plain_inplace(ct_1x1, plain_1);
    
    Plaintext result_1x1;
    decryptor.decrypt(ct_1x1, result_1x1);
    cout << "  1 × 1 (plain) = " << result_1x1.to_string() << "\n";
    
    // I-multiply ng 2
    Plaintext plain_2("2");
    Ciphertext ct_1x2 = ct_1;
    evaluator.multiply_plain_inplace(ct_1x2, plain_2);
    
    Plaintext result_1x2;
    decryptor.decrypt(ct_1x2, result_1x2);
    cout << "  1 × 2 (plain) = " << result_1x2.to_string() << "\n";
    cout << "  Ang scaling factor ay 1!\n\n";

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
    cout << "  Tensor scaling factor: " << sf << "\n";
    cout << "  Expected raw tensor: 552, target: 408\n";
    cout << "  Kailangan i-divide ng " << sf << "\n\n";

    // Sum raw tensor
    Ciphertext sum_tensor = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(sum_tensor, terms[i]);
    }

    print_result(decryptor, sum_tensor, "Raw tensor sum");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";

    // ========================================
    // MAGIC FIX: I-multiply ng inverse ng scaling factor
    // ========================================
    cout << "\n--- MAGIC FIX: Multiply by inverse ---\n";
    
    // Ang tamang correction factor ay 1/sf
    // Pero kailangan nating i-account ang integer arithmetic
    // I-multiply ng 1000 then divide by 1000
    
    double inv_sf = 1.0 / sf;
    int inv_int = (int)(inv_sf * 1000);
    cout << "  Inverse: " << inv_sf << " → " << inv_int << "/1000\n";
    
    Plaintext plain_inv(to_string(inv_int));
    Ciphertext ct_fixed = sum_tensor;
    evaluator.multiply_plain_inplace(ct_fixed, plain_inv);
    
    // I-divide ng 1000 using modular arithmetic
    // Since wala tayong division, i-multiply ng modular inverse ng 1000
    // Pero para sa test, i-print na lang ang result
    
    print_result(decryptor, ct_fixed, "After multiply by inverse");
    cout << "  Noise: " << decryptor.invariant_noise_budget(ct_fixed) << " bits\n";
    
    // I-compute ang actual na value
    Plaintext check_result;
    decryptor.decrypt(ct_fixed, check_result);
    int val = stoi(check_result.to_string(), 0, 16);
    cout << "  Actual value: " << val << "\n";
    cout << "  Divided by 1000: " << (double)val / 1000 << "\n";
    cout << "  Target: 408\n";

    // ========================================
    // DIRECT
    // ========================================
    cout << "\n--- DIRECT ---\n";
    
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
    cout << "  Raw tensor:     " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";
    cout << "  Fixed tensor:   " << decryptor.invariant_noise_budget(ct_fixed) << " bits\n";
    cout << "  Direct:         " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    cout << "========================================\n\n";

    return 0;
}
