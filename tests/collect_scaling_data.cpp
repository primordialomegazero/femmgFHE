/*
 * COLLECT ALL SCALING DATA
 * I-log ang lahat ng multiplication results para sa a=1-20, b=1-20
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
    cout << "  COLLECT SCALING DATA\n";
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

    // Open CSV file
    ofstream csv("scaling_data_complete.csv");
    csv << "a,b,expected,actual,ratio,noise\n";

    cout << "Collecting data for a=1 to 20, b=1 to 20...\n\n";
    
    for (int a = 1; a <= 20; a++) {
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
            int noise = decryptor.invariant_noise_budget(ct_result);
            
            csv << a << "," << b << "," << expected << "," << actual << "," << ratio << "," << noise << "\n";
            
            if (a % 5 == 0 && b % 5 == 0) {
                cout << "  " << a << "×" << b << " = " << hex << actual << " (dec: " << dec << actual << "), expected: " << expected << ", ratio: " << ratio << ", noise: " << noise << "\n";
            }
        }
    }
    
    csv.close();
    cout << "\n✅ Data saved to scaling_data_complete.csv\n";
    cout << "   Total entries: 400\n\n";

    return 0;
}
