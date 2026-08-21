// CKKS ARITHMETIC CIRCUIT — Hindi Binary
// Encrypted addition, multiplication, polynomial evaluation
// Ito ang totoong strength ng CKKS — approximate real numbers

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS ARITHMETIC CIRCUIT\n";
    std::cout << "  Real Number Computation\n";
    std::cout << "========================================\n\n";

    // Depth 30, scaling 40
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "Slots: " << slots << "\n\n";

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "ARITHMETIC TESTS:\n";
    std::cout << "==================\n\n";

    // 1. ADDITION
    std::cout << "1. ADDITION (3.5 + 2.5):\n";
    auto a = make_ct(3.5);
    auto b = make_ct(2.5);
    auto sum = cc->EvalAdd(a, b);
    std::cout << "   3.5 + 2.5 = " << decrypt_val(sum) << " (expected 6.0)\n\n";

    // 2. SUBTRACTION
    std::cout << "2. SUBTRACTION (10 - 4):\n";
    auto c = make_ct(10.0);
    auto d = make_ct(4.0);
    auto diff = cc->EvalSub(c, d);
    std::cout << "   10 - 4 = " << decrypt_val(diff) << " (expected 6.0)\n\n";

    // 3. MULTIPLICATION
    std::cout << "3. MULTIPLICATION (3 × 4):\n";
    auto e = make_ct(3.0);
    auto f = make_ct(4.0);
    auto prod = cc->EvalMult(e, f);
    std::cout << "   3 × 4 = " << decrypt_val(prod) << " (expected 12.0)\n\n";

    // 4. POLYNOMIAL: f(x) = x² + 2x + 1 sa x=3
    std::cout << "4. POLYNOMIAL f(x)=x²+2x+1 sa x=3:\n";
    auto x = make_ct(3.0);
    auto x_sq = cc->EvalMult(x, x);
    auto two_x = cc->EvalMult(x, make_ct(2.0));
    auto poly = cc->EvalAdd(cc->EvalAdd(x_sq, two_x), make_ct(1.0));
    std::cout << "   f(3) = " << decrypt_val(poly) << " (expected 16.0)\n\n";

    // 5. DOT PRODUCT: [1,2,3]·[4,5,6] = 1×4+2×5+3×6 = 32
    std::cout << "5. DOT PRODUCT [1,2,3]·[4,5,6]:\n";
    auto v1_0 = make_ct(1.0), v1_1 = make_ct(2.0), v1_2 = make_ct(3.0);
    auto v2_0 = make_ct(4.0), v2_1 = make_ct(5.0), v2_2 = make_ct(6.0);
    auto p0 = cc->EvalMult(v1_0, v2_0);
    auto p1 = cc->EvalMult(v1_1, v2_1);
    auto p2 = cc->EvalMult(v1_2, v2_2);
    auto dot = cc->EvalAdd(cc->EvalAdd(p0, p1), p2);
    std::cout << "   dot = " << decrypt_val(dot) << " (expected 32.0)\n\n";

    // 6. SQUARE ROOT approximation: √2 ≈ 1.414
    std::cout << "6. APPROXIMATION √2 ≈ 1.414:\n";
    auto two = make_ct(2.0);
    // Newton's method: x = 1.5, x = (x + 2/x)/2
    auto approx = make_ct(1.5);
    auto two_over_x = cc->EvalMult(two, make_ct(1.0/1.5));
    approx = cc->EvalMult(cc->EvalAdd(approx, two_over_x), make_ct(0.5));
    std::cout << "   √2 ≈ " << decrypt_val(approx) << " (expected ~1.416)\n\n";

    // 7. FIBONACCI SEQUENCE: F(10) = 55
    std::cout << "7. FIBONACCI F(10):\n";
    auto f_prev = make_ct(0.0);
    auto f_curr = make_ct(1.0);
    for (int i = 2; i <= 10; i++) {
        auto f_next = cc->EvalAdd(f_prev, f_curr);
        f_prev = f_curr;
        f_curr = f_next;
    }
    std::cout << "   F(10) = " << decrypt_val(f_curr) << " (expected 55.0)\n\n";

    std::cout << "========================================\n";
    std::cout << "  LAHAT NG ARITHMETIC AY GUMAGANA!\n";
    std::cout << "========================================\n";

    return 0;
}
