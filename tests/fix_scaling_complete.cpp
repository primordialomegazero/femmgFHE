/*
 * COMPLETE SCALING FIX
 * I-account ang lahat ng scaling factors
 */

#include "seal/seal.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
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

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  COMPLETE SCALING FIX\n";
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
    // STEP 1: Get the scaling factor Δ
    // ========================================
    cout << "--- STEP 1: Get SEAL Scaling Factor Δ ---\n";
    
    Plaintext plain_1("1");
    Ciphertext ct_1;
    encryptor.encrypt(plain_1, ct_1);
    
    // Multiply by 1 to see the scaling
    Ciphertext ct_1x1 = ct_1;
    evaluator.multiply_plain_inplace(ct_1x1, plain_1);
    
    Plaintext result_1x1;
    decryptor.decrypt(ct_1x1, result_1x1);
    cout << "  1 × 1 (plain) = " << result_1x1.to_string() << "\n";
    cout << "  Δ = " << stoi(result_1x1.to_string(), 0, 16) << "\n\n";

    // ========================================
    // STEP 2: Get raw tensor
    // ========================================
    cout << "--- STEP 2: Raw Tensor ---\n";
    
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);
    
    vector<Ciphertext> terms;
    for (int i = 0; i < 4; i++) {
        Ciphertext term = ct_6;
        evaluator.multiply_inplace(term, ct_17);
        evaluator.relinearize_inplace(term, relin_keys);
        terms.push_back(term);
    }
    
    Ciphertext sum_tensor = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(sum_tensor, terms[i]);
    }
    
    Plaintext result_raw;
    decryptor.decrypt(sum_tensor, result_raw);
    int raw_val = stoi(result_raw.to_string(), 0, 16);
    cout << "  Raw tensor: " << result_raw.to_string() << " (dec: " << raw_val << ")\n";
    cout << "  Expected: 408\n\n";

    // ========================================
    // STEP 3: Correct scaling
    // ========================================
    cout << "--- STEP 3: Correct Scaling ---\n";
    
    double sf = get_scaling_factor(6, 17);
    cout << "  Tensor scaling factor: " << sf << "\n";
    
    // Ang tamang correction ay: divide by sf, then divide by Δ
    // Δ = q/t, but we can compute it from 1×1
    
    int delta = stoi(result_1x1.to_string(), 0, 16);
    cout << "  SEAL scaling factor Δ: " << delta << "\n";
    
    // I-compute ang tamang correction factor
    // Correction = 1 / (sf * Δ)
    double total_sf = sf * delta;
    double inv_total = 1.0 / total_sf;
    int inv_int = (int)(inv_total * 1000);
    cout << "  Total scaling: " << total_sf << "\n";
    cout << "  Inverse: " << inv_total << " → " << inv_int << "/1000\n";
    
    // Apply correction
    Plaintext plain_correction(to_string(inv_int));
    Ciphertext ct_corrected = sum_tensor;
    evaluator.multiply_plain_inplace(ct_corrected, plain_correction);
    
    Plaintext result_corrected;
    decryptor.decrypt(ct_corrected, result_corrected);
    int corrected_val = stoi(result_corrected.to_string(), 0, 16);
    cout << "  Corrected result: " << result_corrected.to_string() << " (dec: " << corrected_val << ")\n";
    cout << "  Divided by 1000: " << (double)corrected_val / 1000 << "\n";
    cout << "  Expected: 408\n\n";

    // ========================================
    // STEP 4: I-verify kung exact
    // ========================================
    cout << "--- STEP 4: Verification ---\n";
    
    double result_dec = (double)corrected_val / 1000;
    if (abs(result_dec - 408) < 0.01) {
        cout << "  ✅ EXACT! Ang correction ay gumagana!\n";
    } else {
        cout << "  ❌ Hindi pa exact. Kailangan ng mas precise na correction.\n";
        cout << "  Difference: " << abs(result_dec - 408) << "\n";
    }

    // ========================================
    // SUMMARY
    // ========================================
    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Raw tensor:     " << raw_val << "\n";
    cout << "  SEAL Δ:         " << delta << "\n";
    cout << "  Tensor sf:      " << sf << "\n";
    cout << "  Total sf:       " << total_sf << "\n";
    cout << "  Corrected:      " << (double)corrected_val / 1000 << "\n";
    cout << "  Expected:       408\n";
    cout << "========================================\n\n";

    return 0;
}
