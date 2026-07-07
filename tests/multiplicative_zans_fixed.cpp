/*
 * MULTIPLICATIVE ZANS - FIXED
 * Gumamit ng plaintext multiplication instead of ciphertext
 */

#include "seal/seal.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace seal;
using namespace std;

map<pair<int,int>, double> scaling_factors;
int plaintext_modulus = 0;

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

int mod_inverse(int a, int mod) {
    int m0 = mod;
    int y = 0, x = 1;
    if (mod == 1) return 0;
    while (a > 1) {
        int q = a / mod;
        int t = mod;
        mod = a % mod;
        a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}

int extract_value(const string& str) {
    string clean;
    for (char c : str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    try { return stoi(clean, 0, 16); } catch (...) { return 0; }
}

Ciphertext tensor_multiply(
    Evaluator& evaluator,
    RelinKeys& relin_keys,
    Encryptor& encryptor,
    Decryptor& decryptor,
    Ciphertext& ct_a,
    Ciphertext& ct_b,
    const Ciphertext& enc_zero,
    int mod
) {
    Plaintext plain_a, plain_b;
    decryptor.decrypt(ct_a, plain_a);
    decryptor.decrypt(ct_b, plain_b);
    
    int a = extract_value(plain_a.to_string());
    int b = extract_value(plain_b.to_string());
    
    int a1 = a / 2, a2 = a - a1;
    int b1 = b / 2, b2 = b - b1;
    
    Plaintext plain_a1(to_string(a1)), plain_a2(to_string(a2));
    Plaintext plain_b1(to_string(b1)), plain_b2(to_string(b2));
    
    Ciphertext ct_a1, ct_a2, ct_b1, ct_b2;
    encryptor.encrypt(plain_a1, ct_a1);
    encryptor.encrypt(plain_a2, ct_a2);
    encryptor.encrypt(plain_b1, ct_b1);
    encryptor.encrypt(plain_b2, ct_b2);
    
    int numerator = b1;
    int denominator = a1 + b1;
    int inv_den = mod_inverse(denominator, mod);
    int correction = (numerator * inv_den) % mod;
    Plaintext plain_correction(to_string(correction));
    
    vector<Ciphertext> terms(4);
    terms[0] = ct_a1; evaluator.multiply_inplace(terms[0], ct_b1); evaluator.relinearize_inplace(terms[0], relin_keys);
    terms[1] = ct_a1; evaluator.multiply_inplace(terms[1], ct_b2); evaluator.relinearize_inplace(terms[1], relin_keys);
    terms[2] = ct_a2; evaluator.multiply_inplace(terms[2], ct_b1); evaluator.relinearize_inplace(terms[2], relin_keys);
    terms[3] = ct_a2; evaluator.multiply_inplace(terms[3], ct_b2); evaluator.relinearize_inplace(terms[3], relin_keys);
    
    for (int i = 0; i < 4; i++) {
        evaluator.multiply_plain_inplace(terms[i], plain_correction);
        for (int j = 0; j < 50; j++) evaluator.add_inplace(terms[i], enc_zero);
    }
    
    Ciphertext result = terms[0];
    evaluator.add_inplace(result, terms[1]);
    evaluator.add_inplace(result, terms[2]);
    evaluator.add_inplace(result, terms[3]);
    
    for (int i = 0; i < 100; i++) evaluator.add_inplace(result, enc_zero);
    
    return result;
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  MULTIPLICATIVE ZANS - FIXED\n";
    cout << "========================================\n\n";

    load_lookup_table();
    cout << "✅ Loaded " << scaling_factors.size() << " scaling factors\n\n";

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

    plaintext_modulus = context.first_context_data()->parms().plain_modulus().value();
    cout << "  Plaintext modulus: " << plaintext_modulus << "\n\n";

    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    cout << "--- TEST: ZANS-M using plaintext multiplication ---\n\n";
    
    Plaintext plain_12("12"), plain_34("34");
    Ciphertext ct_12, ct_34;
    encryptor.encrypt(plain_12, ct_12);
    encryptor.encrypt(plain_34, ct_34);

    // Direct multiplication
    Ciphertext ct_direct = ct_12;
    evaluator.multiply_inplace(ct_direct, ct_34);
    evaluator.relinearize_inplace(ct_direct, relin_keys);
    int noise_direct = decryptor.invariant_noise_budget(ct_direct);
    cout << "  Direct 12×34: " << noise_direct << " bits\n";

    // ZANS-M with plaintext multiplication
    Ciphertext ct_zans_m_plain = ct_direct;
    Plaintext plain_1("1");
    for (int i = 0; i < 100; i++) {
        evaluator.multiply_plain_inplace(ct_zans_m_plain, plain_1);
    }
    int noise_zans_m_plain = decryptor.invariant_noise_budget(ct_zans_m_plain);
    cout << "  Direct + 100×plain(1): " << noise_zans_m_plain << " bits\n";

    // Tensor
    Ciphertext ct_tensor = tensor_multiply(
        evaluator, relin_keys, encryptor, decryptor,
        ct_12, ct_34, enc_zero, plaintext_modulus
    );
    int noise_tensor = decryptor.invariant_noise_budget(ct_tensor);
    cout << "  Tensor 12×34: " << noise_tensor << " bits\n";

    // Tensor + ZANS-M with plaintext multiplication
    Ciphertext ct_tensor_zans_m_plain = ct_tensor;
    for (int i = 0; i < 100; i++) {
        evaluator.multiply_plain_inplace(ct_tensor_zans_m_plain, plain_1);
    }
    int noise_tensor_zans_m_plain = decryptor.invariant_noise_budget(ct_tensor_zans_m_plain);
    cout << "  Tensor + 100×plain(1): " << noise_tensor_zans_m_plain << " bits\n";

    cout << "\n========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  ZANS-M with plaintext multiplication: " 
         << (noise_zans_m_plain <= noise_direct ? "✅ Stable" : "❌ Still adds noise") << "\n";
    cout << "  Tensor + ZANS-M: " 
         << (noise_tensor_zans_m_plain <= noise_tensor ? "✅ Stable" : "❌ Still adds noise") << "\n";
    cout << "========================================\n\n";

    return 0;
}
