/*
 * CREATE LOOKUP TABLE FOR SCALING FACTORS
 * I-store ang lahat ng scaling factors para sa a=1-20, b=1-20
 */

#include "seal/seal.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  CREATE LOOKUP TABLE\n";
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

    // Open lookup table file
    ofstream lookup("scaling_lookup.txt");
    lookup << "Scaling Factor Lookup Table (a x b)\n";
    lookup << "====================================\n\n";
    lookup << "     ";
    for (int b = 1; b <= 20; b++) {
        lookup << setw(8) << b;
    }
    lookup << "\n";

    for (int a = 1; a <= 20; a++) {
        lookup << setw(3) << a << " ";
        for (int b = 1; b <= 20; b++) {
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
            
            lookup << setw(8) << fixed << setprecision(4) << ratio;
        }
        lookup << "\n";
    }
    
    lookup.close();
    cout << "✅ Lookup table saved to scaling_lookup.txt\n\n";

    return 0;
}
