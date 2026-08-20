// CKKS HOMOMORPHIC PERIOD-2 RESET
// Sariling bootstrapping method na walang decryption
//
// ANG KEY IDEA:
// Sa φ-structure, ang NAND(NAND(x,x), NAND(x,x)) = x
// Kung gagamitin natin ito bilang "refresh" operation,
// ang noise ay maaaring mag-reset nang walang bootstrapping
//
// ANG CLAIM:
// Kapag ang value ay nasa {0, φ} na, ang double NAND ay:
//   DoubleNAND(0) = 0 (exact)
//   DoubleNAND(φ) = φ (exact)
// Ito ay self-correcting!
//
// TEST natin kung ito ay totoo sa CKKS encrypted domain.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS HOMOMORPHIC PERIOD-2 RESET\n";
    std::cout << "  Sariling Bootstrapping Method\n";
    std::cout << "========================================\n\n";

    // Depth 30, scaling 40
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Binary encoding: 0 at 1
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST 1: HOMOMORPHIC PERIOD-2 RESET
    // ============================================
    std::cout << "TEST 1: HOMOMORPHIC PERIOD-2 RESET\n";
    std::cout << "====================================\n\n";
    std::cout << "  Ang period-2 reset ay:\n";
    std::cout << "  reset(x) = NAND(NAND(x,x), NAND(x,x))\n";
    std::cout << "  = NOT(NOT(x))\n";
    std::cout << "  = x (theoretically)\n\n";

    // Test: ang reset ba ay identity?
    auto current = ct1;
    auto reset = nand_op(nand_op(current, current), nand_op(current, current));

    double val = decrypt_val(reset);
    std::cout << "  reset(1) = " << val << " (expected 1)\n";

    // Test sa 0
    auto reset_zero = nand_op(nand_op(ct0, ct0), nand_op(ct0, ct0));
    val = decrypt_val(reset_zero);
    std::cout << "  reset(0) = " << val << " (expected 0)\n\n";

    // ============================================
    // TEST 2: CHAIN NA MAY PERIOD-2 RESET
    // ============================================
    std::cout << "TEST 2: CHAIN + PERIOD-2 RESET\n";
    std::cout << "================================\n\n";
    std::cout << "  Strategy: 5 NAND gates → 1 reset → repeat\n\n";

    current = ct1;
    int errors = 0;
    int total_gates = 25;

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Every 5 gates, mag-apply ng period-2 reset
        if (gate % 5 == 4) {
            // reset = NOT(NOT(x)) = 4 additional NAND operations
            auto not1 = nand_op(current, current);
            current = nand_op(not1, not1);
        }

        double v = decrypt_val(current);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;
    }

    std::cout << "  " << total_gates << " gates with reset: "
              << errors << "/" << total_gates << " errors ("
              << (100.0 * (total_gates - errors) / total_gates) << "%)\n\n";

    // ============================================
    // TEST 3: DEPTH COMPARISON
    // ============================================
    std::cout << "TEST 3: DEPTH COMPARISON\n";
    std::cout << "==========================\n\n";

    // Standard: 1 NAND = 1 mult → 25 gates = 25 mults
    // With reset: 5 NAND + 4 reset mults = 9 mults per 5 gates
    // → 25 gates = 5 cycles × 9 = 45 mults — mas malalim pa nga!

    // Mas efficient na reset:
    // reset(x) = NAND(NAND(x,x), NAND(x,x)) = 4 mults
    // Pero kung i-combine natin ang NAND at reset:
    // Pagkatapos ng gate, mag-NAND pa 2 beses para sa reset
    // Ito ay 3 mults per "effective gate"

    std::cout << "  Standard: 25 gates = 25 multiplications\n";
    std::cout << "  With reset: 25 gates = 45 multiplications\n";
    std::cout << "  Mas malalim ang with reset — hindi efficient\n\n";

    // ============================================
    // TEST 4: MAS EFFICIENT RESET
    // ============================================
    std::cout << "TEST 4: MAS EFFICIENT RESET\n";
    std::cout << "=============================\n\n";

    // Ang period-2 property ay:
    // NOT(NOT(x)) = x
    // Kaya: kung nag-NAND tayo ng 2 beses, babalik tayo sa original
    // Hindi na kailangan ng karagdagang reset!
    //
    // ANG KEY: ang 2 NAND operations ay natural na nagre-reset
    // Kaya: ang chain na may even na bilang ng NANDs ay self-resetting

    std::cout << "  Self-resetting chain (even gates):\n";
    current = ct1;
    errors = 0;

    for (int gate = 0; gate < 25; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors++;
    }

    std::cout << "  25 gates: " << errors << "/25 errors ("
              << (100.0 * (25 - errors) / 25) << "%)\n";

    return 0;
}
