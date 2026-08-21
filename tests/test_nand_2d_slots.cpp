// NAND 2D — TAMANG SLOT ENCODING
// Slot 0 = A, Slot 1 = B
// Ang NAND ay kayang ma-encode sa 2 slots

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND 2D — SLOT ENCODING\n";
    std::cout << "  A sa Slot 0, B sa Slot 1\n";
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

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // A at B bilang magkaibang slots
    auto make_ab = [&](double a_val, double b_val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {a_val, 0.0};  // A sa slot 0
        vec[1] = {b_val, 0.0};  // B sa slot 1
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // A=0, B=0
    auto nand_00 = make_ab(0.0, 0.0);
    // A=0, B=1
    auto nand_01 = make_ab(0.0, phi_sq);
    // A=1, B=0
    auto nand_10 = make_ab(phi_sq, 0.0);
    // A=1, B=1
    auto nand_11 = make_ab(phi_sq, phi_sq);

    std::cout << "2D SLOT VALUES:\n";
    std::cout << "===============\n\n";

    std::cout << "  (0,0): A=" << decrypt_slot(nand_00, 0) << ", B=" << decrypt_slot(nand_00, 1) << "\n";
    std::cout << "  (0,1): A=" << decrypt_slot(nand_01, 0) << ", B=" << decrypt_slot(nand_01, 1) << "\n";
    std::cout << "  (1,0): A=" << decrypt_slot(nand_10, 0) << ", B=" << decrypt_slot(nand_10, 1) << "\n";
    std::cout << "  (1,1): A=" << decrypt_slot(nand_11, 0) << ", B=" << decrypt_slot(nand_11, 1) << "\n\n";

    // Ang NAND sa 2 slots:
    // NAND = φ² kung A=0 o B=0
    // NAND = 0 kung A=φ² at B=φ²
    //
    // Sa slot arithmetic:
    // sum_slot = A + B
    // NAND = φ² kung sum_slot < 2φ²
    // NAND = 0 kung sum_slot >= 2φ²

    std::cout << "SUM VALUES:\n";
    std::cout << "===========\n\n";

    std::cout << "  (0,0): sum = " << decrypt_slot(nand_00, 0) + decrypt_slot(nand_00, 1) << "\n";
    std::cout << "  (0,1): sum = " << decrypt_slot(nand_01, 0) + decrypt_slot(nand_01, 1) << "\n";
    std::cout << "  (1,0): sum = " << decrypt_slot(nand_10, 0) + decrypt_slot(nand_10, 1) << "\n";
    std::cout << "  (1,1): sum = " << decrypt_slot(nand_11, 0) + decrypt_slot(nand_11, 1) << "\n\n";

    std::cout << "  NAND = φ² kung sum ∈ {0, φ²}\n";
    std::cout << "  NAND = 0 kung sum = 2φ²\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: 2D slot encoding complete\n";

    return 0;
}
