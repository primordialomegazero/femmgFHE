// SEAL BFV CROSS-VALIDATION
#include <seal/seal.h>
#include <iostream>
#include <vector>

using namespace seal;

int main() {
    std::cout << "SEAL BFV CROSS-VALIDATION\n";
    std::cout << "=========================\n\n";

    // Setup BFV parameters
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 8192;
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

    // Encrypt 0 at 1
    Plaintext pt0("0");
    Plaintext pt1("1");
    Ciphertext ct0, ct1;
    encryptor.encrypt(pt0, ct0);
    encryptor.encrypt(pt1, ct1);

    std::cout << "1. INITIAL ENCRYPTION:\n";
    Plaintext dec0, dec1;
    decryptor.decrypt(ct0, dec0);
    decryptor.decrypt(ct1, dec1);
    std::cout << "   Decrypt(0) = " << dec0.to_string() << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << dec1.to_string() << " (exp 1)\n\n";

    // Homomorphic NAND
    std::cout << "2. HOMOMORPHIC NAND:\n";
    
    // NAND(1,1) = 0
    Ciphertext prod_11;
    evaluator.multiply(ct1, ct1, prod_11);
    evaluator.relinearize_inplace(prod_11, relin_keys);
    Ciphertext nand_11;
    evaluator.sub(ct1, prod_11, nand_11);
    
    Plaintext dec_nand_11;
    decryptor.decrypt(nand_11, dec_nand_11);
    std::cout << "   NAND(1,1) = " << dec_nand_11.to_string() << " (exp 0)\n";

    // NAND(0,0) = 1
    Ciphertext prod_00;
    evaluator.multiply(ct0, ct0, prod_00);
    evaluator.relinearize_inplace(prod_00, relin_keys);
    Ciphertext nand_00;
    evaluator.sub(ct1, prod_00, nand_00);
    
    Plaintext dec_nand_00;
    decryptor.decrypt(nand_00, dec_nand_00);
    std::cout << "   NAND(0,0) = " << dec_nand_00.to_string() << " (exp 1)\n\n";

    // Deep chain (20 depths)
    std::cout << "3. DEEP CHAIN (20 depths):\n";
    auto current = ct1;
    int errors = 0;

    for (int i = 0; i <= 20; i++) {
        Ciphertext sq;
        evaluator.multiply(current, current, sq);
        evaluator.relinearize_inplace(sq, relin_keys);
        Ciphertext result;
        evaluator.sub(ct1, sq, result);

        Plaintext dec_result;
        decryptor.decrypt(result, dec_result);
        int val = 0;
        std::string dec_str = dec_result.to_string();
        if (!dec_str.empty() && dec_str[0] == '1') val = 1;
        int expected = (i % 2 == 0) ? 1 : 0;

        if (val != expected) errors++;
        current = result;
    }

    std::cout << "   Errors: " << errors << "/21\n\n";

    std::cout << "4. RESULT:\n";
    std::cout << (errors == 0 ? "   SEAL BFV — 0 ERRORS ✓" : "   May errors") << "\n";

    return 0;
}
