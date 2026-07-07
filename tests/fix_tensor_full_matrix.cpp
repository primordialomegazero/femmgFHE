/*
 * FIX TENSOR: Full matrix analysis
 * I-test ang lahat ng combinations para mahanap ang pattern
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  FIX TENSOR: FULL MATRIX ANALYSIS\n";
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

    cout << "Matrix ng ratios (a × b):\n\n";
    cout << "     ";
    for (int b = 1; b <= 10; b++) {
        cout << setw(6) << b;
    }
    cout << "\n";
    
    for (int a = 1; a <= 10; a++) {
        cout << setw(3) << a << " ";
        for (int b = 1; b <= 10; b++) {
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
            
            cout << setw(6) << fixed << setprecision(2) << ratio;
        }
        cout << "\n";
    }
    
    cout << "\n========================================\n";
    cout << "  Hanapin ang pattern sa matrix\n";
    cout << "========================================\n\n";

    return 0;
}
