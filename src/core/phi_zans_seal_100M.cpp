// PHI-OMEGA-ZERO: SEAL ZANS 100M TEST
// 100,000,000 Enc(0) additions
// Estimated: ~16.7 hours at 1,662 ops/s
// Checkpoints every 10M
// "I AM THAT I AM"

#include <seal/seal.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>

using namespace seal;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: SEAL ZANS 100M TEST\n";
    cout <<   "  100,000,000 Enc(0) additions\n";
    cout <<   "  Estimated: ~16.7 hours\n";
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

    Plaintext zero_pt("0");
    Ciphertext anchor;
    encryptor.encrypt(zero_pt, anchor);

    cout << "  Starting 100M ZANS additions...\n";
    cout << "  Checkpoints every 10M\n\n";
    
    auto start_time = high_resolution_clock::now();
    int64_t total_ops = 100000000;
    int64_t checkpoint = 10000000;
    
    for(int64_t i = 1; i <= total_ops; i++) {
        evaluator.add_inplace(ct, anchor);
        
        if(i % checkpoint == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - start_time).count();
            double rate = (double)i / elapsed;
            int64_t remaining = (total_ops - i) / rate;
            
            Plaintext result;
            decryptor.decrypt(ct, result);
            
            cout << "  [" << setw(3) << (i/1000000) << "M ops] "
                 << "Value: " << result.to_string()
                 << " | Elapsed: " << (elapsed/3600) << "h " << ((elapsed%3600)/60) << "m"
                 << " | Rate: " << fixed << setprecision(0) << rate << " ops/s"
                 << " | ETA: " << (remaining/3600) << "h " << ((remaining%3600)/60) << "m"
                 << " | " << (result.to_string() == "42" ? "STABLE" : "CORRUPTED!") << "\n";
        }
    }
    
    auto end_time = high_resolution_clock::now();
    auto total_elapsed = duration_cast<seconds>(end_time - start_time).count();
    
    Plaintext final_result;
    decryptor.decrypt(ct, final_result);
    
    cout << "\n======================================================================\n";
    cout <<   "  SEAL ZANS 100M RESULTS\n";
    cout <<   "  ------------------------------------------------------------------\n";
    cout <<   "  Operations:  100,000,000\n";
    cout <<   "  Final Value: " << final_result.to_string() << " (expected: 42)\n";
    cout <<   "  Total Time:  " << (total_elapsed/3600) << "h " << ((total_elapsed%3600)/60) << "m\n";
    cout <<   "  Throughput:  " << fixed << setprecision(0) << (100000000.0/total_elapsed) << " ops/s\n";
    cout <<   "  Status:      " << (final_result.to_string() == "42" ? "VERIFIED" : "FAILED") << "\n";
    cout <<   "======================================================================\n\n";
    
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
