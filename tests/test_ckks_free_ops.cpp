// CKKS FREE OPERATIONS — 0 LEVELS
// May ADVANCEDSHE enabled para sa EvalSum

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS FREE OPERATIONS CHECK\n";
    std::cout << "  (0 Multiplicative Depth)\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);  // Para sa EvalSum

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);  // For EvalSum
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto check_level = [](const char* op, auto ct) {
        std::cout << "  " << op << ": level=" << ct->GetLevel() << "\n";
    };

    auto ct1 = make_ct(1.0);
    auto ct2 = make_ct(2.0);
    
    std::cout << "Initial:\n";
    check_level("ct1", ct1);
    check_level("ct2", ct2);
    
    std::cout << "\nFree Operations:\n";
    std::cout << "================\n";
    
    // Addition
    auto add = cc->EvalAdd(ct1, ct2);
    check_level("EvalAdd", add);
    
    // Subtraction
    auto sub = cc->EvalSub(ct1, ct2);
    check_level("EvalSub", sub);
    
    // Negation
    auto neg = cc->EvalNegate(ct1);
    check_level("EvalNegate", neg);
    
    // Rotation
    auto rot = cc->EvalRotate(ct1, 1);
    check_level("EvalRotate", rot);
    
    // Sum
    auto sum = cc->EvalSum(ct1, 4);  // Sum over 4 slots
    check_level("EvalSum", sum);
    
    // Inner Product
    auto inner = cc->EvalInnerProduct(ct1, ct2, 4);
    check_level("EvalInnerProduct", inner);
    
    std::cout << "\nMultiplicative Operations:\n";
    std::cout << "==========================\n";
    
    // Multiplication (consumes 1 level)
    auto mult = cc->EvalMult(ct1, ct2);
    check_level("EvalMult", mult);
    
    // Square (consumes 1 level)
    auto square = cc->EvalSquare(ct1);
    check_level("EvalSquare", square);

    return 0;
}
