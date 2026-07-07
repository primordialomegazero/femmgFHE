/*
 * FIX TENSOR: Hanapin ang formula para sa scaling factor
 * 
 * Observation:
 * - 3 × 2 = 6 (correct)
 * - 2 × 17 = 46 (instead of 34)
 * - 4 × 17 = 92 (instead of 68)
 * - 8 × 14 = 160 (instead of 112)
 * 
 * Ang scaling factor ay depende sa mga numbers!
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;
using namespace std;

void print_result(Decryptor& dec, Ciphertext& ct, const char* label) {
    Plaintext result;
    dec.decrypt(ct, result);
    cout << "  " << label << ": " << result.to_string() << "\n";
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX TENSOR: FORMULA ANALYSIS\n";
    cout << "========================================\n\n";

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
    // TEST 1: Iba't ibang combinations
    // ========================================
    cout << "--- TEST 1: Iba't ibang combinations ---\n";
    
    vector<pair<int, int>> tests = {
        {2, 17}, {3, 17}, {4, 17}, {5, 17},
        {2, 14}, {3, 14}, {4, 14}, {5, 14},
        {2, 20}, {3, 20}, {4, 20}, {5, 20}
    };
    
    for (auto [a, b] : tests) {
        Plaintext plain_a(to_string(a));
        Plaintext plain_b(to_string(b));
        Ciphertext ct_a, ct_b;
        encryptor.encrypt(plain_a, ct_a);
        encryptor.encrypt(plain_b, ct_b);
        
        Ciphertext ct_result = ct_a;
        evaluator.multiply_inplace(ct_result, ct_b);
        evaluator.relinearize_inplace(ct_result, relin_keys);
        
        Plaintext result;
        decryptor.decrypt(ct_result, result);
        int actual = stoi(result.to_string(), 0, 16);
        int expected = a * b;
        double ratio = (double)actual / expected;
        
        cout << "  " << a << " × " << b << " = " << hex << actual << " (dec: " << dec << actual << "), expected: " << expected << ", ratio: " << ratio << "\n";
    }
    
    cout << "\n";

    // ========================================
    // TEST 2: Ang formula
    // ========================================
    cout << "--- TEST 2: Formula Analysis ---\n";
    cout << "  Ang scaling factor ay (a + b) / ?\n";
    cout << "  Para sa 2×17: (2+17)/? = 1.35294\n";
    cout << "  ? = 19 / 1.35294 = 14.043\n";
    cout << "  ? ≈ 14\n";
    cout << "  So (a+b)/14 = 19/14 = 1.357\n";
    cout << "  Pero ang actual ay 46/34 = 1.35294\n";
    cout << "  Hindi exact.\n\n";
    
    cout << "  Ang scaling factor ay (a + b) / 14.043\n";
    cout << "  Bakit 14.043?\n";
    cout << "  14.043 ≈ (17 - 2.957)\n";
    cout << "  Hindi pa rin exact.\n\n";

    // ========================================
    // TEST 3: Ang tunay na formula
    // ========================================
    cout << "--- TEST 3: Ang Tunay na Formula ---\n";
    cout << "  Ang scaling factor ay (a + b) / (a - b + 2a?)\n";
    cout << "  Para sa 2×17: (2+17) / (17-2) = 19/15 = 1.2667\n";
    cout << "  Hindi.\n\n";
    
    cout << "  Ang scaling factor ay (a + b) / (a + b - 2a)\n";
    cout << "  = (a+b) / (b-a)\n";
    cout << "  Para sa 2×17: 19/15 = 1.2667\n";
    cout << "  Hindi.\n\n";
    
    cout << "  Ang scaling factor ay (a + b) / (a + b - 2b)\n";
    cout << "  = (a+b) / (a-b)\n";
    cout << "  Para sa 2×17: 19/(-15) = -1.2667\n";
    cout << "  Hindi.\n\n";

    // ========================================
    // SUMMARY
    // ========================================
    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Ang scaling factor ay hindi linear.\n";
    cout << "  Kailangan ng mas malalim na analysis.\n";
    cout << "========================================\n\n";

    return 0;
}
