/*
 * FIX TENSOR: I-test ang supported modulus sizes
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

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
    cout << "  FIX TENSOR: SUPPORTED MODULUS SIZES\n";
    cout << "========================================\n\n";

    // I-test ang supported modulus sizes
    vector<int> mod_sizes = {12, 15, 18, 20, 22, 25, 28, 30};
    
    for (int bits : mod_sizes) {
        cout << "--- Modulus: " << bits << " bits ---\n";
        
        try {
            EncryptionParameters parms(scheme_type::bfv);
            parms.set_poly_modulus_degree(16384);
            parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
            parms.set_plain_modulus(PlainModulus::Batching(16384, bits));
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

            // Test: 12 × 34 = 408
            Plaintext plain_12("12");
            Plaintext plain_34("34");
            Plaintext plain_6("6");
            Plaintext plain_17("17");
            
            Ciphertext ct_12, ct_34, ct_6, ct_17;
            encryptor.encrypt(plain_12, ct_12);
            encryptor.encrypt(plain_34, ct_34);
            encryptor.encrypt(plain_6, ct_6);
            encryptor.encrypt(plain_17, ct_17);

            // Direct
            Ciphertext ct_direct = ct_12;
            evaluator.multiply_inplace(ct_direct, ct_34);
            evaluator.relinearize_inplace(ct_direct, relin_keys);
            
            print_result(decryptor, ct_direct, "Direct 12×34");
            cout << "  Noise: " << decryptor.invariant_noise_budget(ct_direct) << " bits\n";
            
            // Tensor
            Ciphertext term1 = ct_6;
            evaluator.multiply_inplace(term1, ct_17);
            evaluator.relinearize_inplace(term1, relin_keys);
            
            Ciphertext term2 = ct_6;
            evaluator.multiply_inplace(term2, ct_17);
            evaluator.relinearize_inplace(term2, relin_keys);
            
            Ciphertext term3 = ct_6;
            evaluator.multiply_inplace(term3, ct_17);
            evaluator.relinearize_inplace(term3, relin_keys);
            
            Ciphertext term4 = ct_6;
            evaluator.multiply_inplace(term4, ct_17);
            evaluator.relinearize_inplace(term4, relin_keys);

            Ciphertext sum_tensor = term1;
            evaluator.add_inplace(sum_tensor, term2);
            evaluator.add_inplace(sum_tensor, term3);
            evaluator.add_inplace(sum_tensor, term4);
            
            print_result(decryptor, sum_tensor, "Tensor sum");
            cout << "  Noise: " << decryptor.invariant_noise_budget(sum_tensor) << " bits\n";
            cout << "  ✅ SUCCESS\n\n";
            
        } catch (const exception& e) {
            cout << "  ❌ ERROR: " << e.what() << "\n\n";
        }
    }

    cout << "========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  Hanapin kung saang modulus gumagana ang tensor\n";
    cout << "========================================\n\n";

    return 0;
}
