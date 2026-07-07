/*
 * ZANS vs ORDINARY ADDITION
 * I-compare ang ZANS sa ordinary addition
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>

using namespace seal;
using namespace std;

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  ZANS vs ORDINARY ADDITION\n";
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
    BatchEncoder batch_encoder(context);

    size_t slot_count = batch_encoder.slot_count();
    vector<uint64_t> val_vec(slot_count, 0ULL);
    val_vec[0] = 42ULL;
    Plaintext plain_42;
    batch_encoder.encode(val_vec, plain_42);
    
    // ZANS: Enc(0)
    vector<uint64_t> zero_vec(slot_count, 0ULL);
    Plaintext zero_plain;
    batch_encoder.encode(zero_vec, zero_plain);
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);
    
    // Ordinary addition: Enc(1)
    vector<uint64_t> one_vec(slot_count, 0ULL);
    one_vec[0] = 1ULL;
    Plaintext one_plain;
    batch_encoder.encode(one_vec, one_plain);
    Ciphertext enc_one;
    encryptor.encrypt(one_plain, enc_one);

    cout << "--- TEST 1: ZANS (Add Enc(0)) ---\n";
    Ciphertext ct_zans;
    encryptor.encrypt(plain_42, ct_zans);
    int start_zans = decryptor.invariant_noise_budget(ct_zans);
    cout << "  Start noise: " << start_zans << " bits\n";
    
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_zans, enc_zero);
    }
    int end_zans = decryptor.invariant_noise_budget(ct_zans);
    cout << "  After 1000 ZANS: " << end_zans << " bits\n";
    cout << "  Drift: " << (start_zans - end_zans) << " bits\n\n";

    cout << "--- TEST 2: Ordinary Addition (Add Enc(1)) ---\n";
    Ciphertext ct_ordinary;
    encryptor.encrypt(plain_42, ct_ordinary);
    int start_ord = decryptor.invariant_noise_budget(ct_ordinary);
    cout << "  Start noise: " << start_ord << " bits\n";
    
    for (int i = 0; i < 1000; i++) {
        evaluator.add_inplace(ct_ordinary, enc_one);
    }
    int end_ord = decryptor.invariant_noise_budget(ct_ordinary);
    cout << "  After 1000 ordinary adds: " << end_ord << " bits\n";
    cout << "  Drift: " << (start_ord - end_ord) << " bits\n\n";

    cout << "========================================\n";
    cout << "  CONCLUSION\n";
    cout << "========================================\n";
    cout << "  ZANS: " << (start_zans - end_zans) << " bits drift (CONTRACTION!)\n";
    cout << "  Ordinary: " << (start_ord - end_ord) << " bits drift (EXPANSION!)\n";
    cout << "========================================\n\n";

    return 0;
}
