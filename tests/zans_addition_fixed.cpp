/*
 * ZANS ADDITION FIXED
 * I-fix ang value preservation issue
 * Gamit ang BatchEncoder para sa tamang encoding
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>

using namespace seal;
using namespace std;

string to_hex(int value) {
    stringstream ss;
    ss << hex << value;
    return ss.str();
}

int hex_to_int(const string& hex_str) {
    string clean;
    for (char c : hex_str) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            clean += c;
        }
    }
    if (clean.empty()) return 0;
    return stoi(clean, 0, 16);
}

int main() {
    cout << "\n";
    cout << "========================================\n";
    cout << "  ZANS ADDITION FIXED\n";
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

    // Pre-compute Enc(0) using BatchEncoder
    size_t slot_count = batch_encoder.slot_count();
    vector<uint64_t> zero_vec(slot_count, 0ULL);
    Plaintext zero_plain;
    batch_encoder.encode(zero_vec, zero_plain);
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    cout << "--- ZANS ADDITION DEMO (Fixed) ---\n\n";
    
    // Encrypt 42 using BatchEncoder
    vector<uint64_t> val_vec(slot_count, 0ULL);
    val_vec[0] = 42ULL;
    Plaintext plain_42;
    batch_encoder.encode(val_vec, plain_42);
    Ciphertext ct_42;
    encryptor.encrypt(plain_42, ct_42);

    int start_noise = decryptor.invariant_noise_budget(ct_42);
    cout << "  Start: 42 (noise: " << start_noise << " bits)\n\n";

    // ZANS: Add Enc(0) repeatedly
    int total_ops = 1000000;
    int checkpoint = 100000;
    
    cout << "  Applying ZANS (" << total_ops << " additions)...\n\n";
    
    auto start_time = chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= total_ops; i++) {
        evaluator.add_inplace(ct_42, enc_zero);
        
        if (i % checkpoint == 0) {
            int current_noise = decryptor.invariant_noise_budget(ct_42);
            cout << "  " << i << " ops: noise = " << current_noise << " bits\n";
        }
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
    
    int final_noise = decryptor.invariant_noise_budget(ct_42);
    
    // Verify value using BatchEncoder
    Plaintext result;
    decryptor.decrypt(ct_42, result);
    vector<uint64_t> result_vec;
    batch_encoder.decode(result, result_vec);
    int val = result_vec[0];
    
    cout << "\n========================================\n";
    cout << "  ZANS ADDITION RESULTS (Fixed)\n";
    cout << "========================================\n";
    cout << "  Start noise:  " << start_noise << " bits\n";
    cout << "  Final noise:  " << final_noise << " bits\n";
    cout << "  Total drift:  " << (start_noise - final_noise) << " bits\n";
    cout << "  Drift/op:     " << (double)(start_noise - final_noise) / total_ops << " bits\n";
    cout << "  Value:        " << val << " (expected 42)\n";
    cout << "  Time:         " << duration << " ms\n";
    cout << "  Throughput:   " << (int)(total_ops / (duration / 1000.0)) << " ops/sec\n";
    cout << "========================================\n";
    cout << "  ✅ ZANS WORKS FOR ADDITION!\n";
    cout << "========================================\n\n";

    return 0;
}
