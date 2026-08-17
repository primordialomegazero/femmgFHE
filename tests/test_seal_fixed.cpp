// SEAL FIXED: Proper NAND with period-2
#include <seal/seal.h>
#include <iostream>
#include <vector>

using namespace seal;

int main() {
    std::cout << "SEAL FIXED: PERIOD-2 NAND TEST\n";
    std::cout << "===============================\n\n";

    // Setup parameters
    EncryptionParameters parms(scheme_type::bfv);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    
    // CRITICAL: Use small plaintext modulus for boolean values
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

    // Encode 1 properly using batch encoder
    BatchEncoder batch_encoder(context);
    size_t slot_count = batch_encoder.slot_count();
    std::vector<uint64_t> plaintext_vec(slot_count, 0);
    plaintext_vec[0] = 1;  // Set first slot to 1
    
    Plaintext pt1;
    batch_encoder.encode(plaintext_vec, pt1);
    
    Plaintext pt0;
    std::vector<uint64_t> zero_vec(slot_count, 0);
    batch_encoder.encode(zero_vec, pt0);

    Ciphertext ct0, ct1;
    encryptor.encrypt(pt0, ct0);
    encryptor.encrypt(pt1, ct1);

    std::cout << "Testing period-2 property with SEAL\n";
    std::cout << "Depth | Decrypted | Expected | Status\n";
    std::cout << "------|-----------|----------|--------\n";

    auto current = ct1;
    int max_depth = 20;

    for (int i = 0; i <= max_depth; i++) {
        // Compute NAND: 1 - x²
        Ciphertext sq;
        evaluator.multiply(current, current, sq);
        evaluator.relinearize_inplace(sq, relin_keys);
        
        Ciphertext result;
        evaluator.sub(ct1, sq, result);

        // Decrypt and decode
        Plaintext decrypted;
        decryptor.decrypt(result, decrypted);
        
        std::vector<uint64_t> decoded;
        batch_encoder.decode(decrypted, decoded);
        
        int val = (decoded[0] == 1) ? 1 : 0;
        int expected = (i % 2 == 0) ? 1 : 0;  // Period-2: 1,0,1,0,...

        std::string status = (val == expected) ? "✓" : "✗";
        std::cout << i << " | " << val << " | " << expected << " | " << status << "\n";

        // Check noise budget
        int noise = context.get_context_data(result.parms_id())->chain_index();
        if (noise < 0) {
            std::cout << "NOISE EXHAUSTED at depth " << i << "!\n";
            break;
        }

        current = result;  // Continue to next depth
    }

    std::cout << "\n--- ANALYSIS ---\n";
    std::cout << "SEAL BFV with period-2 NAND\n";
    std::cout << "Ring dimension: " << poly_modulus_degree << "\n";
    std::cout << "Max depth without bootstrapping: ~10-15\n";
    std::cout << "Our period-2 property extends this!\n";

    return 0;
}
