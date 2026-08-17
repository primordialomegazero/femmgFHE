// SEAL CLEAN: Minimum working BFV NAND
#include <seal/seal.h>
#include <iostream>

using namespace seal;

int main() {
    std::cout << "SEAL CLEAN BFV NAND TEST\n";
    std::cout << "========================\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
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
    BatchEncoder batch_encoder(context);

    size_t slot_count = batch_encoder.slot_count();
    std::cout << "Slots: " << slot_count << "\n\n";

    std::vector<uint64_t> ones(slot_count, 1);
    std::vector<uint64_t> zeros(slot_count, 0);

    Plaintext pt1, pt0;
    batch_encoder.encode(ones, pt1);
    batch_encoder.encode(zeros, pt0);

    Ciphertext ct1, ct0;
    encryptor.encrypt(pt1, ct1);
    encryptor.encrypt(pt0, ct0);

    // NAND(1,1) = 0
    Ciphertext prod;
    evaluator.multiply(ct1, ct1, prod);
    evaluator.relinearize_inplace(prod, relin_keys);
    Ciphertext nand_11;
    evaluator.sub(ct1, prod, nand_11);

    Plaintext dec_nand;
    decryptor.decrypt(nand_11, dec_nand);
    std::vector<uint64_t> res(slot_count);
    batch_encoder.decode(dec_nand, res);
    std::cout << "NAND(1,1) = " << res[0] << " (exp 0)\n";

    // NAND(0,0) = 1
    Ciphertext prod_00;
    evaluator.multiply(ct0, ct0, prod_00);
    evaluator.relinearize_inplace(prod_00, relin_keys);
    Ciphertext nand_00;
    evaluator.sub(ct1, prod_00, nand_00);

    Plaintext dec_nand_00;
    decryptor.decrypt(nand_00, dec_nand_00);
    batch_encoder.decode(dec_nand_00, res);
    std::cout << "NAND(0,0) = " << res[0] << " (exp 1)\n";

    return 0;
}
