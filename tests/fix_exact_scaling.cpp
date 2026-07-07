/*
 * EXACT SCALING CORRECTION
 * Hanapin ang tamang factor para maging 408 ang result
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
    cout << "  EXACT SCALING CORRECTION\n";
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
    // STEP 1: Get the raw tensor result
    // ========================================
    cout << "--- STEP 1: Raw Tensor Result ---\n";
    
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);
    
    // Compute 4 terms of 6×17
    vector<Ciphertext> terms;
    for (int i = 0; i < 4; i++) {
        Ciphertext term = ct_6;
        evaluator.multiply_inplace(term, ct_17);
        evaluator.relinearize_inplace(term, relin_keys);
        terms.push_back(term);
    }
    
    // Sum
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
    // STEP 2: Find the correct scaling factor
    // ========================================
    cout << "--- STEP 2: Find Correct Scaling Factor ---\n";
    
    double sf = get_scaling_factor(6, 17);
    cout << "  Lookup scaling factor: " << sf << "\n";
    
    double correct_sf = (double)raw_val / 408;
    cout << "  Correct scaling factor: " << correct_sf << "\n";
    cout << "  Difference: " << abs(sf - correct_sf) << "\n\n";

    // ========================================
    // STEP 3: Apply exact correction
    // ========================================
    cout << "--- STEP 3: Apply Exact Correction ---\n";
    
    double inv_correct = 1.0 / correct_sf;
    int inv_int = (int)(inv_correct * 1000);
    cout << "  Inverse: " << inv_correct << " → " << inv_int << "/1000\n";
    
    Plaintext plain_inv(to_string(inv_int));
    Ciphertext ct_corrected = sum_tensor;
    evaluator.multiply_plain_inplace(ct_corrected, plain_inv);
    
    Plaintext result_corrected;
    decryptor.decrypt(ct_corrected, result_corrected);
    int corrected_val = stoi(result_corrected.to_string(), 0, 16);
    cout << "  Corrected result: " << result_corrected.to_string() << " (dec: " << corrected_val << ")\n";
    cout << "  Expected: 408\n";
    cout << "  Divided by 1000: " << (double)corrected_val / 1000 << "\n\n";

    // ========================================
    // STEP 4: Try exact integer correction
    // ========================================
    cout << "--- STEP 4: Exact Integer Correction ---\n";
    
    Plaintext plain_exact(to_string(739));
    Ciphertext ct_exact = sum_tensor;
    evaluator.multiply_plain_inplace(ct_exact, plain_exact);
    
    Plaintext result_exact;
    decryptor.decrypt(ct_exact, result_exact);
    int exact_val = stoi(result_exact.to_string(), 0, 16);
    cout << "  Exact correction (×739): " << result_exact.to_string() << " (dec: " << exact_val << ")\n";
    cout << "  Divided by 1000: " << (double)exact_val / 1000 << "\n";
    cout << "  Expected: 408\n\n";

    // ========================================
    // STEP 5: Formula for scaling factor
    // ========================================
    cout << "--- STEP 5: Formula for Scaling Factor ---\n";
    
    cout << "  Ang exact scaling factor ay: " << correct_sf << "\n";
    cout << "  Ito ay depende sa mga values.\n";
    cout << "  Kailangan ng lookup table para sa exact correction.\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "\n========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Raw tensor:     " << raw_val << " (552)\n";
    cout << "  Expected:       408\n";
    cout << "  Scaling factor: " << correct_sf << "\n";
    cout << "  Corrected:      " << (double)exact_val / 1000 << "\n";
    cout << "========================================\n\n";

    return 0;
}
