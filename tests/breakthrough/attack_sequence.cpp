// ATTACK 3: Sequence Pattern Analysis
// Does the sequence of operations reveal the bit?
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE F_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE F_mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}
double F_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double F_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = F_val(cc, kp, s.a), b = F_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
PE F_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE F_swap(PE x) { auto t=x.a; x.a=x.b; x.b=t; return x; }

struct FractalNode { PE expand; PE contract; int depth; };

// MODIFIED: We record the sequence of operations!
struct AttackFractalNode {
    PE expand; PE contract; int depth;
    int total_forward;   // Total φ operations
    int total_reverse;   // Total ψ operations
    int total_swaps;     // Total swaps
    int total_cross;     // Total cross operations
};

int F_dual_rotate_record(CryptoContext<DCRTPoly>& cc, PE& E, PE& C,
                          int& fwd, int& rev, int& swp, int& crs) {
    int action = rand()%6;
    switch(action) {
        case 0: E = F_mulY(cc, E);     C = F_mulY(cc, C);     fwd+=2; break;
        case 1: E = F_mulY_inv(cc, E); C = F_mulY_inv(cc, C); rev+=2; break;
        case 2: E = F_mulY(cc, E);     C = F_mulY_inv(cc, C); fwd++; rev++; crs++; break;
        case 3: E = F_mulY_inv(cc, E); C = F_mulY(cc, C);     rev++; fwd++; crs++; break;
        case 4: E = F_swap(E);         swp++; break;
        case 5: C = F_swap(C);         swp++; break;
    }
    return action;
}

AttackFractalNode F_recursive_dual_record(CryptoContext<DCRTPoly>& cc, const PE& input, int depth) {
    if (depth <= 0) return {input, input, 0, 0, 0, 0, 0};
    
    PE E = input, C = input;
    int fwd=0, rev=0, swp=0, crs=0;
    int rotations = 3 + rand()%4;
    
    for (int i = 0; i < rotations; i++) {
        F_dual_rotate_record(cc, E, C, fwd, rev, swp, crs);
    }
    
    AttackFractalNode subE = F_recursive_dual_record(cc, E, depth - 1);
    AttackFractalNode subC = F_recursive_dual_record(cc, C, depth - 1);
    
    return {subE.expand, subC.contract, depth,
            fwd + subE.total_forward + subC.total_forward,
            rev + subE.total_reverse + subC.total_reverse,
            swp + subE.total_swaps + subC.total_swaps,
            crs + subE.total_cross + subC.total_cross};
}

int main() {
    srand(time(0));
    
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ATTACK 3: Operation Sequence Analysis              ║\n";
    std::cout << "  ║  Do operation counts reveal the bit?                ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20); p.SetScalingModSize(30); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    const int SAMPLES = 200;
    
    std::vector<double> fwd_0, fwd_1, rev_0, rev_1, swp_0, swp_1, crs_0, crs_1;
    
    for (int t = 0; t < SAMPLES; t++) {
        int secret = rand() % 2;
        PE ct = F_enc(cc, kp, secret);
        AttackFractalNode result = F_recursive_dual_record(cc, ct, 2);
        
        if (secret == 0) {
            fwd_0.push_back(result.total_forward);
            rev_0.push_back(result.total_reverse);
            swp_0.push_back(result.total_swaps);
            crs_0.push_back(result.total_cross);
        } else {
            fwd_1.push_back(result.total_forward);
            rev_1.push_back(result.total_reverse);
            swp_1.push_back(result.total_swaps);
            crs_1.push_back(result.total_cross);
        }
    }
    
    auto avg = [](std::vector<double>& v) {
        double sum = 0; for (double x : v) sum += x; return sum / v.size();
    };
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │ OPERATION COUNTS (Depth 2, " << SAMPLES << " samples)                │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │              Bit=0 Avg    Bit=1 Avg    Difference    │\n";
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    
    double af0=avg(fwd_0), af1=avg(fwd_1);
    double ar0=avg(rev_0), ar1=avg(rev_1);
    double as0=avg(swp_0), as1=avg(swp_1);
    double ac0=avg(crs_0), ac1=avg(crs_1);
    
    std::cout << "  │ Forward (φ): " << std::fixed << std::setprecision(1) << std::setw(8) << af0
              << "      " << std::setw(8) << af1 << "      " << std::setw(8) << std::abs(af0-af1) << "    │\n";
    std::cout << "  │ Reverse (ψ): " << std::setw(8) << ar0
              << "      " << std::setw(8) << ar1 << "      " << std::setw(8) << std::abs(ar0-ar1) << "    │\n";
    std::cout << "  │ Swaps:       " << std::setw(8) << as0
              << "      " << std::setw(8) << as1 << "      " << std::setw(8) << std::abs(as0-as1) << "    │\n";
    std::cout << "  │ Cross:       " << std::setw(8) << ac0
              << "      " << std::setw(8) << ac1 << "      " << std::setw(8) << std::abs(ac0-ac1) << "    │\n";
    
    double max_diff = std::max({std::abs(af0-af1), std::abs(ar0-ar1), 
                                 std::abs(as0-as1), std::abs(ac0-ac1)});
    double avg_mag = (af0+af1+ar0+ar1+as0+as1+ac0+ac1) / 8.0;
    double relative_diff = 100.0 * max_diff / avg_mag;
    
    std::cout << "  ├──────────────────────────────────────────────────────┤\n";
    std::cout << "  │ Max relative difference: " << std::fixed << std::setprecision(2) << relative_diff << "%                      │\n";
    
    if (relative_diff < 10.0)
        std::cout << "  │ ✓ ATTACK FAILED — Operation counts are uniform      │\n";
    else if (relative_diff < 25.0)
        std::cout << "  │ ⚠ MINOR LEAK — Small count difference               │\n";
    else
        std::cout << "  │ ❌ VULNERABLE — Operation counts leak information!  │\n";
    
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    return 0;
}
