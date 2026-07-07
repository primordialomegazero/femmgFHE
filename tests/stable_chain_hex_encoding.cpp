/*
 * STABLE CHAIN - HEX ENCODING
 * I-encode ang mga values as HEX para mag-match
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace seal;
using namespace std;

string to_hex(int value) {
    stringstream ss;
    ss << hex << value;
    return ss.str();
}

int hex_to_int(const string& hex_str) {
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    return stoi(clean, 0, 16);
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  STABLE CHAIN - HEX ENCODING\n";
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

    cout << "--- TEST: 7 × 11 = 77 (HEX: 4D) ---\n\n";
    
    // I-encode ang 7 at 11 as HEX
    string hex_7 = to_hex(7);   // "7"
    string hex_11 = to_hex(11); // "b"
    string hex_77 = to_hex(77); // "4d"
    
    cout << "  HEX values:\n";
    cout << "    7 = 0x" << hex_7 << "\n";
    cout << "    11 = 0x" << hex_11 << "\n";
    cout << "    77 = 0x" << hex_77 << "\n\n";
    
    Plaintext plain_7(hex_7);
    Plaintext plain_11(hex_11);
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
    int val = hex_to_int(result_str);
    
    cout << "  Result string: " << result_str << "\n";
    cout << "  Expected hex: " << hex_77 << "\n";
    cout << "  Decimal value: " << val << "\n";
    cout << "  Expected: " << 77 << "\n";
    cout << "  Status: " << (val == 77 ? "✅ CORRECT" : "❌ WRONG") << "\n\n";

    cout << "========================================\n";
    cout << "  TEST COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
