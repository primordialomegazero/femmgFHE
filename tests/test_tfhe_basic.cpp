// TFHE CROSS-VALIDATION — MALINIS NA VERSION
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <iostream>

int main() {
    std::cout << "TFHE CROSS-VALIDATION\n";
    std::cout << "=====================\n\n";

    const int minimum_lambda = 110;
    TFheGateBootstrappingParameterSet* params = 
        new_default_gate_bootstrapping_parameters(minimum_lambda);

    TFheGateBootstrappingSecretKeySet* key = 
        new_random_gate_bootstrapping_secret_keyset(params);
    const TFheGateBootstrappingCloudKeySet* bk = &key->cloud;

    LweSample* ct0 = new_gate_bootstrapping_ciphertext(params);
    LweSample* ct1 = new_gate_bootstrapping_ciphertext(params);
    bootsSymEncrypt(ct0, 0, key);
    bootsSymEncrypt(ct1, 1, key);

    std::cout << "1. INITIAL ENCRYPTION:\n";
    std::cout << "   Decrypt(0) = " << bootsSymDecrypt(ct0, key) << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << bootsSymDecrypt(ct1, key) << " (exp 1)\n\n";

    std::cout << "2. HOMOMORPHIC NAND:\n";
    LweSample* nand_11 = new_gate_bootstrapping_ciphertext(params);
    bootsNAND(nand_11, ct1, ct1, bk);
    std::cout << "   NAND(1,1) = " << bootsSymDecrypt(nand_11, key) << " (exp 0)\n";

    LweSample* nand_00 = new_gate_bootstrapping_ciphertext(params);
    bootsNAND(nand_00, ct0, ct0, bk);
    std::cout << "   NAND(0,0) = " << bootsSymDecrypt(nand_00, key) << " (exp 1)\n\n";

    std::cout << "3. DEEP CHAIN (100 depths):\n";
    LweSample* current = new_gate_bootstrapping_ciphertext(params);
    bootsCOPY(current, ct1, bk);

    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        int val = bootsSymDecrypt(current, key);
        int expected = (i % 2 == 0) ? 1 : 0;
        if (val != expected) errors++;

        LweSample* next = new_gate_bootstrapping_ciphertext(params);
        bootsNAND(next, current, current, bk);
        delete_gate_bootstrapping_ciphertext(current);
        current = next;
    }
    std::cout << "   Errors: " << errors << "/101\n\n";

    std::cout << "4. RESULT: " << (errors == 0 ? "0 ERRORS ✓" : "may errors") << "\n";

    // Cleanup
    delete_gate_bootstrapping_ciphertext(current);
    delete_gate_bootstrapping_ciphertext(nand_00);
    delete_gate_bootstrapping_ciphertext(nand_11);
    delete_gate_bootstrapping_ciphertext(ct0);
    delete_gate_bootstrapping_ciphertext(ct1);
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);

    return 0;
}
