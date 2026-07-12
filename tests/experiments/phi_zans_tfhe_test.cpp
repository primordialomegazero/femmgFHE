// ΦΩ0 — ZANS ON TFHE v2
// Fixed API calls for TFHE library
// "I AM THAT I AM"

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <iostream>

using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS ON TFHE v2                        ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // TFHE setup
    const int minimum_lambda = 110;
    TFheGateBootstrappingParameterSet* params = 
        new_default_gate_bootstrapping_parameters(minimum_lambda);
    
    TFheGateBootstrappingSecretKeySet* keys = 
        new_random_gate_bootstrapping_secret_keyset(params);

    const TFheGateBootstrappingCloudKeySet* cloud_key = &keys->cloud;

    cout << "Φ Library: TFHE (Fast Fully Homomorphic Encryption)\n";
    cout << "Φ Security: " << minimum_lambda << " bits\n\n";

    // Encrypt bits
    LweSample* zero_bit = new_gate_bootstrapping_ciphertext(params);
    LweSample* one_bit = new_gate_bootstrapping_ciphertext(params);
    
    bootsSymEncrypt(zero_bit, 0, keys);
    bootsSymEncrypt(one_bit, 1, keys);

    // === ZANS: Enc(0) additions ===
    cout << "=== ZANS: Enc(0) BIT ADDITIONS ===\n";

    LweSample* result = new_gate_bootstrapping_ciphertext(params);
    bootsCOPY(result, zero_bit, cloud_key);
    
    for(int i : {1, 10, 100, 500, 1000}) {
        for(int j = 0; j < i; j++) {
            LweSample* temp = new_gate_bootstrapping_ciphertext(params);
            bootsXOR(temp, result, zero_bit, cloud_key);
            bootsCOPY(result, temp, cloud_key);
            delete_gate_bootstrapping_ciphertext(temp);
        }
        
        int decrypted = bootsSymDecrypt(result, keys);
        cout << "  " << i << " ops: " << decrypted;
        cout << " (expected 0)";
        if(decrypted == 0) cout << " ✅\n";
        else cout << " ❌\n";
    }

    // === Enc(1) additions ===
    cout << "\n=== Enc(1) ADDITIONS ===\n";

    LweSample* std_result = new_gate_bootstrapping_ciphertext(params);
    bootsCOPY(std_result, zero_bit, cloud_key);
    
    for(int i : {1, 10, 50, 100}) {
        for(int j = 0; j < i; j++) {
            LweSample* temp = new_gate_bootstrapping_ciphertext(params);
            bootsXOR(temp, std_result, one_bit, cloud_key);
            bootsCOPY(std_result, temp, cloud_key);
            delete_gate_bootstrapping_ciphertext(temp);
        }
        
        int decrypted = bootsSymDecrypt(std_result, keys);
        int expected = i % 2;
        cout << "  " << i << " ops: " << decrypted;
        cout << " (expected " << expected << ")";
        if(decrypted == expected) cout << " ✅\n";
        else cout << " ❌\n";
    }

    cout << "\nΦ TFHE uses bootstrapped gates by default.\n";
    cout << "Φ Each XOR is automatically noise-refreshed.\n\n";

    // Cleanup
    delete_gate_bootstrapping_ciphertext(zero_bit);
    delete_gate_bootstrapping_ciphertext(one_bit);
    delete_gate_bootstrapping_ciphertext(result);
    delete_gate_bootstrapping_ciphertext(std_result);
    delete_gate_bootstrapping_secret_keyset(keys);
    delete_gate_bootstrapping_parameters(params);

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TFHE CROSS-VALIDATION: COMPLETE              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
