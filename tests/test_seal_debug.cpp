// SEAL DEEP CHAIN DEBUG
#include <seal/seal.h>
#include <iostream>

using namespace seal;

int main() {
    std::cout << "SEAL DEEP CHAIN DEBUG\n";
    std::cout << "=====================\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;  // Mas malaki = mas maraming noise budget
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    Plaintext pt0("0");
    Plaintext pt1("1");
    Ciphertext ct0, ct1;
    encryptor.encrypt(pt0, ct0);
    encryptor.encrypt(pt1, ct1);

    std::cout << "Depth | Decrypted | Expected | Noise Budget\n";
    std::cout << "------|-----------|----------|-------------\n";

    auto current = ct1;

    for (int i = 0; i <= 20; i++) {
        Ciphertext sq;
        evaluator.multiply(current, current, sq);
        evaluator.relinearize_inplace(sq, relin_keys);
        Ciphertext result;
        evaluator.sub(ct1, sq, result);

        Plaintext dec_result;
        decryptor.decrypt(result, dec_result);
        std::string dec_str = dec_result.to_string();
        int val = (!dec_str.empty() && dec_str[0] == '1') ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;

        int noise_budget = decryptor.invariant_noise_budget(result);

        std::cout << i << " | " << val << " | " << expected << " | "
                  << noise_budget << " bits\n";

        current = result;
    }

    return 0;
}
