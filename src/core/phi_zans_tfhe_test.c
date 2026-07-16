// PHI-OMEGA-ZERO: TFHE ZANS TEST
// Cross-library validation of ZANS on TFHE (binary gates)
// "I AM THAT I AM"

#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <stdio.h>
#include <time.h>

const char* ts() {
    static char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buf;
}

int main() {
    printf("\n===============================================================\n");
    printf("  PHI-OMEGA-ZERO: TFHE ZANS TEST\n");
    printf("===============================================================\n");
    printf("  HARDWARE: AMD Ryzen 5 2600 | LIBRARY: TFHE\n");
    printf("  START: %s\n", ts());
    printf("===============================================================\n\n");

    // TFHE setup: 110-bit security
    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(110);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);

    // Encrypt bits: a=1, b=1
    LweSample* a = new_gate_bootstrapping_ciphertext(params);
    LweSample* b = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(a, 1, key);
    bootsSymEncrypt(b, 1, key);

    // Encrypted zero for ZANS
    LweSample* zero = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(zero, 0, key);

    LweSample* result = new_gate_bootstrapping_ciphertext(params);
    
    int steps = 100;
    printf("  Running %d NAND gates with ZANS...\n", steps);

    clock_t t1 = clock();

    // Start with a NAND b = 0 (since 1 NAND 1 = 0)
    bootsNAND(result, a, b, &key->cloud);

    // Chain: result = NAND(result, zero)
    for(int i = 0; i < steps; i++) {
        bootsNAND(result, result, zero, &key->cloud);
    }

    clock_t t2 = clock();
    double elapsed = (double)(t2 - t1) / CLOCKS_PER_SEC;

    // Decrypt
    int final_bit = bootsSymDecrypt(result, key);

    printf("  Operations: %d NAND gates\n", steps);
    printf("  Initial: 1 NAND 1 = 0\n");
    printf("  Final NAND(x,0): %d (expected: 1)\n", final_bit);
    printf("  Time: %.1fs\n", elapsed);
    printf("  Throughput: %.0f gates/s\n", steps / elapsed);
    printf("  Status: %s\n\n", final_bit == 1 ? "PASSED" : "CHECK");

    delete_gate_bootstrapping_ciphertext(a);
    delete_gate_bootstrapping_ciphertext(b);
    delete_gate_bootstrapping_ciphertext(zero);
    delete_gate_bootstrapping_ciphertext(result);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);

    return 0;
}
