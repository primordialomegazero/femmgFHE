// RULE 110 DEEP DEBUG
// Tingnan ang bawat value sa bawat step

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 DEEP DEBUG\n";
    std::cout << "  Step-by-Step Analysis\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // Initial state: lahat 0 maliban sa slot 128 na φ²
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "INITIAL STATE:\n";
    std::cout << "  slot127=" << decrypt_slot(state, 127) << "\n";
    std::cout << "  slot128=" << decrypt_slot(state, 128) << "\n";
    std::cout << "  slot129=" << decrypt_slot(state, 129) << "\n\n";

    std::cout << "STEP-BY-STEP DEBUG (first 10 steps):\n";
    std::cout << "====================================\n\n";
    std::cout << "Step | L(127) | C(128) | R(129) | Sum | After +φ² | After -3φ²\n";
    std::cout << "-----|--------|--------|--------|-----|-----------|----------\n";

    for (int step = 0; step < 10; step++) {
        // Step 1: Kunin ang neighbors
        auto left = cc->EvalAtIndex(state, -1);
        auto right = cc->EvalAtIndex(state, 1);

        double L = decrypt_slot(left, 128);
        double C = decrypt_slot(state, 128);
        double R = decrypt_slot(right, 128);

        // Step 2: Sum
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        double sum_val = decrypt_slot(sum, 128);

        // Step 3: Add φ²
        auto after_add = cc->EvalAdd(sum, 
            cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
                std::vector<std::complex<double>>(slots, {phi_sq, 0.0}))));
        double add_val = decrypt_slot(after_add, 128);

        // Step 4: Subtract 3φ² if step % 4 == 2
        double final_val = add_val;
        if (step % 4 == 2) {
            auto after_sub = cc->EvalSub(after_add,
                cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
                    std::vector<std::complex<double>>(slots, {3*phi_sq, 0.0}))));
            final_val = decrypt_slot(after_sub, 128);
            state = after_sub;
        } else {
            state = after_add;
        }

        std::cout << step << " | " << L << " | " << C << " | " << R 
                  << " | " << sum_val << " | " << add_val << " | " << final_val << "\n";
    }

    std::cout << "\n";
    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  1. Ang sum = L + C + R ay may 3 neighbors\n";
    std::cout << "  2. Kapag ang mga neighbors ay lahat φ²,\n";
    std::cout << "     ang sum = 3φ² (malaki)\n";
    std::cout << "  3. Ang pag-add ng φ² ay lalong nagpapalaki\n";
    std::cout << "  4. Ang period-4 correction ay hindi sapat\n";
    std::cout << "     para i-contain ang 3φ² growth\n\n";

    return 0;
}
