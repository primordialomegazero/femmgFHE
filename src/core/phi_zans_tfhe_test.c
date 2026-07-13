// PHI-OMEGA-ZERO: TFHE ZANS TEST
// "I AM THAT I AM"

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

int main() {
    printf("\n======================================================================\n");
    printf("  PHI-OMEGA-ZERO: TFHE ZANS TEST\n");
    printf("======================================================================\n\n");

    // Generate params
    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(100);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);

    // Encrypt test bits (1 = true)
    LweSample* ct = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(ct, 1, key);

    // Create Enc(0) anchor
    LweSample* anchor = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(anchor, 0, key);

    printf("  Running 50 ZANS additions...\n");
    
    clock_t t1 = clock();
    
    for(int i = 0; i < 50; i++) {
        // Add anchor (Enc(0)) to ct
        lweAddTo(ct, anchor, params->in_out_params);
    }
    
    clock_t t2 = clock();
    double elapsed = (double)(t2 - t1) / CLOCKS_PER_SEC;

    // Decrypt
    int result = bootsSymDecrypt(ct, key);
    
    printf("  Operations: 50\n");
    printf("  Result: %d (expected: 1)\n", result);
    printf("  Time: %.2fs\n", elapsed);
    printf("  Status: %s\n", result == 1 ? "PASSED" : "FAILED");
    
    printf("\n======================================================================\n");
    printf("  TFHE ZANS: %s\n", result == 1 ? "VERIFIED" : "FAILED");
    printf("======================================================================\n\n");

    delete_gate_bootstrapping_ciphertext(ct);
    delete_gate_bootstrapping_ciphertext(anchor);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);

    return 0;
}
