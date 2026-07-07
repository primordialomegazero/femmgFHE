/*
 * STABLE CHAIN - DECIMAL INTERPRETATION
 * Ang SEAL results ay HEX, pero ang values ay decimal
 * Kailangan nating i-convert ang hex to decimal properly
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace seal;
using namespace std;

int hex_to_decimal(const string& hex_str) {
    // I-remove ang mga spaces at non-hex characters
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    
    // I-convert mula sa hex to decimal
    int result = 0;
    for (char c : clean) {
        int digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        result = result * 16 + digit;
    }
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  STABLE CHAIN - DECIMAL INTERPRETATION\n";
    cout << "========================================\n\n";

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

    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);
    
    Plaintext plain_one("1");

    cout << "--- SIMPLE TEST: 7 × 11 = 77 ---\n\n";
    
    Plaintext plain_7("7");
    Plaintext plain_11("11");
    Ciphertext ct_7, ct_11;
    encryptor.encrypt(plain_7, ct_7);
    encryptor.encrypt(plain_11, ct_11);
    
    // Direct multiplication
    Ciphertext ct_result = ct_7;
    evaluator.multiply_inplace(ct_result, ct_11);
    evaluator.relinearize_inplace(ct_result, relin_keys);
    
    Plaintext result;
    decryptor.decrypt(ct_result, result);
    string result_str = result.to_string();
    int val = hex_to_decimal(result_str);
    
    cout << "  Result string: " << result_str << "\n";
    cout << "  Decimal value: " << val << "\n";
    cout << "  Expected: 77\n";
    cout << "  Status: " << (val == 77 ? "✅ CORRECT" : "❌ WRONG") << "\n\n";

    cout << "========================================\n";
    cout << "  TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
