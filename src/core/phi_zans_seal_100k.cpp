// PHI-OMEGA-ZERO: SEAL ZANS 100K TEST
// "I AM THAT I AM"

#include <seal/seal.h>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace seal;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: SEAL ZANS 100K TEST\n";
    cout <<   "======================================================================\n\n";

    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(1073643521);

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);

    // Encrypt test value
    Plaintext pt("42");
    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    // Create Enc(0) anchor
    Plaintext zero_pt("0");
    Ciphertext anchor;
    encryptor.encrypt(zero_pt, anchor);

    cout << "  Running 10,000 ZANS additions...\n";
    
    auto t1 = high_resolution_clock::now();
    
    for(int i = 0; i < 10000; i++) {
        evaluator.add_inplace(ct, anchor);
    }
    
    auto t2 = high_resolution_clock::now();
    double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    // Decrypt and verify
    Plaintext result;
    decryptor.decrypt(ct, result);
    
    cout << "  Operations: 10,000\n";
    cout << "  Result: " << result.to_string() << " (expected: 42)\n";
    cout << "  Time: " << fixed << setprecision(1) << elapsed << "s\n";
    cout << "  Throughput: " << fixed << setprecision(0) << (10000.0/elapsed) << " ops/s\n";
    cout << "  Status: " << (result.to_string() == "42" ? "PASSED" : "FAILED") << "\n";
    
    cout << "\n======================================================================\n";
    cout <<   "  SEAL ZANS: " << (result.to_string() == "42" ? "VERIFIED" : "FAILED") << "\n";
    cout <<   "======================================================================\n\n";

    return 0;
}
