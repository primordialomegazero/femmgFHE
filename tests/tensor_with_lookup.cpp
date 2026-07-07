/*
 * TENSOR DECOMPOSITION WITH LOOKUP TABLE
 * Gamit ang pre-computed scaling factors
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

using namespace seal;
using namespace std;

// Lookup table para sa scaling factors
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
        
        // Parse: "  1    1.0000  1.0000  1.0000 ..."
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
    return 1.0; // Default
}

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  TENSOR DECOMPOSITION WITH LOOKUP\n";
    cout << "========================================\n\n";

    // Load lookup table
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
    // Tensor decomposition: 12 = 6+6, 34 = 17+17
    // ========================================
    cout << "--- TEST: 12 x 34 = 408 ---\n";
    cout << "  Decomposition: 12 = 6+6, 34 = 17+17\n\n";

    // I-encrypt ang mga values
    Plaintext plain_6("6");
    Plaintext plain_17("17");
    Ciphertext ct_6, ct_17;
    encryptor.encrypt(plain_6, ct_6);
    encryptor.encrypt(plain_17, ct_17);

    // Compute tensor terms: 6x17, 6x17, 6x17, 6x17
    vector<Ciphertext> terms;
    for (int i = 0; i < 4; i++) {
        Ciphertext term = ct_6;
        evaluator.multiply_inplace(term, ct_17);
        evaluator.relinearize_inplace(term, relin_keys);
        terms.push_back(term);
    }

    // Get scaling factor para sa 6x17
    double sf = get_scaling_factor(6, 17);
    cout << "  Scaling factor para sa 6x17: " << sf << "\n";

    // I-apply ang correction sa bawat term (divide by scaling factor)
    // Hindi pwedeng i-divide directly sa SEAL, so we'll use a workaround
    // Instead, we'll multiply by the inverse (1/sf) as a plaintext

    // I-sum ang mga terms
    Ciphertext sum_tensor = terms[0];
    for (int i = 1; i < 4; i++) {
        evaluator.add_inplace(sum_tensor, terms[i]);
    }

    print_result(decryptor, sum_tensor, "Raw tensor sum");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";

    // I-correct ang result sa pamamagitan ng pag-multiply ng inverse
    // Since hindi supported ang division, we'll use modulus operation
    // Alternative: I-multiply ng (1/sf) as plaintext
    double inv_sf = 1.0 / sf;
    Plaintext plain_correction(to_string((int)(inv_sf * 1000)));
    evaluator.multiply_plain_inplace(sum_tensor, plain_correction);

    print_result(decryptor, sum_tensor, "Corrected tensor sum");
    cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";

    // ========================================
    // Baseline: Direct multiplication
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
    cout << "  Direct:        " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
    cout << "========================================\n";

    return 0;
}
