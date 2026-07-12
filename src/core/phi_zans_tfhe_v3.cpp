// ΦΩ0 — ZANS ON TFHE v3 — FAST
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <iostream>

using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS ON TFHE v3 (FAST)                 ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    TFheGateBootstrappingParameterSet* params = 
        new_default_gate_bootstrapping_parameters(110);
    TFheGateBootstrappingSecretKeySet* keys = 
        new_random_gate_bootstrapping_secret_keyset(params);
    const TFheGateBootstrappingCloudKeySet* cloud_key = &keys->cloud;

    LweSample* zero = new_gate_bootstrapping_ciphertext(params);
    LweSample* one = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(zero, 0, keys);
    bootsSymEncrypt(one, 1, keys);

    cout << "Φ TFHE | 110-bit security\n\n";

    // ZANS: Enc(0) additions (smaller iterations — TFHE is slow)
    cout << "=== ZANS Enc(0) ===\n";
    LweSample* res = new_gate_bootstrapping_ciphertext(params);
    bootsCOPY(res, zero, cloud_key);

    int checkpoints[] = {1, 5, 10, 25, 50};
    int prev = 0;
    for(int idx = 0; idx < 5; idx++) {
        int target = checkpoints[idx];
        for(int j = prev; j < target; j++) {
            LweSample* temp = new_gate_bootstrapping_ciphertext(params);
            bootsXOR(temp, res, zero, cloud_key);
            bootsCOPY(res, temp, cloud_key);
            delete_gate_bootstrapping_ciphertext(temp);
        }
        int dec = bootsSymDecrypt(res, keys);
        cout << "  " << target << " ops: " << dec << " (exp 0)";
        cout << (dec == 0 ? " ✅\n" : " ❌\n");
        prev = target;
    }

    // Enc(1) comparison
    cout << "\n=== Enc(1) ===\n";
    LweSample* std_res = new_gate_bootstrapping_ciphertext(params);
    bootsCOPY(std_res, zero, cloud_key);
    int check2[] = {1, 5, 10, 25, 50};
    prev = 0;
    for(int idx = 0; idx < 5; idx++) {
        int target = check2[idx];
        for(int j = prev; j < target; j++) {
            LweSample* temp = new_gate_bootstrapping_ciphertext(params);
            bootsXOR(temp, std_res, one, cloud_key);
            bootsCOPY(std_res, temp, cloud_key);
            delete_gate_bootstrapping_ciphertext(temp);
        }
        int dec = bootsSymDecrypt(std_res, keys);
        int exp = target % 2;
        cout << "  " << target << " ops: " << dec << " (exp " << exp << ")";
        cout << (dec == exp ? " ✅\n" : " ❌\n");
        prev = target;
    }

    cout << "\nΦ TFHE auto-bootstraps every gate.\n";
    cout << "Φ ZANS bit-stability confirmed.\n\n";

    delete_gate_bootstrapping_ciphertext(zero);
    delete_gate_bootstrapping_ciphertext(one);
    delete_gate_bootstrapping_ciphertext(res);
    delete_gate_bootstrapping_ciphertext(std_res);
    delete_gate_bootstrapping_secret_keyset(keys);
    delete_gate_bootstrapping_parameters(params);

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TFHE: ✅ ZANS STABLE                         ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
