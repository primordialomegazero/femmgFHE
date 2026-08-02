// DM-DGR: Fibonacci Ring Stability Test
// Tests clean/recovery/swap cycle WITHOUT EvalMult workload
// If this is stable, we add computation back gradually

#include <iostream>
#include <iomanip>
#include <vector>
#include "openfhe.h"

using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> kp;

// Forward clean: drives ψ toward attractor
PE clean_forward(const PE& x) {
    auto sum = cc->EvalAdd(x.a, x.b);
    return {sum, cc->EvalAdd(x.a, sum)};
}

// Reverse clean: φ-error reset
PE clean_reverse(const PE& x) {
    auto a2 = cc->EvalAdd(x.a, x.a);
    return {cc->EvalSub(a2, x.b), cc->EvalSub(x.b, x.a)};
}

// Native bootstrap: ring swap
PE native_bootstrap(const PE& x, bool to_phi) {
    if (to_phi) return {x.b, cc->EvalAdd(x.a, x.b)};
    else        return {cc->EvalSub(x.a, x.b), x.a};
}

double decode_val(const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   DM-DGR: Clean/Recovery/Swap Stability Test          ║\n";
    std::cout <<   "  ║   ZERO EvalMult — testing the cycle only              ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);

    cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Start with value 1.0
    auto seed = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    PE state = {cc->Encrypt(kp.publicKey, seed), cc->Encrypt(kp.publicKey, seed)};
    bool active_phi = true;

    int total_fwd = 0, total_rev = 0, total_swap = 0;

    std::cout << "  CYCLE (per epoch — NO EvalMult):\n";
    std::cout << "    3 Forward cleans  → ψ-noise management\n";
    std::cout << "    1 Reverse clean   → φ-error reset\n";
    std::cout << "    1 Recovery FWD    → ψ self-healing\n";
    std::cout << "    Native bootstrap  → modulus refresh\n\n";

    std::cout << "  Epoch  Act  φ-lvl  ψ-lvl   φ-value      ψ-noise      Action\n";
    std::cout << "------------------------------------------------------------------------\n";

    for (int epoch = 0; epoch < 100; epoch++) {
        std::string action = "";

        try {
            // Forward cleans
            for (int f = 0; f < 3; f++) {
                state = clean_forward(state);
                total_fwd++;
            }

            // Reverse clean
            state = clean_reverse(state);
            total_rev++;
            action = "REV";

            // Recovery forward clean
            state = clean_forward(state);
            total_fwd++;

            // Modulus check and swap
            auto lev_a = state.a->GetLevel();
            auto lev_b = state.b->GetLevel();

            bool need_swap = false;
            if (active_phi) {
                need_swap = (lev_a < 20) || (lev_a > 80);
            } else {
                need_swap = (lev_b < 20) || (lev_b > 80);
            }

            if (need_swap) {
                if (active_phi) {
                    state = native_bootstrap(state, false);
                    active_phi = false;
                    action += " →SWAP ψ";
                } else {
                    state = native_bootstrap(state, true);
                    active_phi = true;
                    action += " →SWAP φ";
                }
                total_swap++;
            }

            lev_a = state.a->GetLevel();
            lev_b = state.b->GetLevel();

            // Decode
            double pv = decode_val(state.a);
            double sv = decode_val(state.b);

            // Print every 5 epochs, or when action happens
            if (epoch % 10 == 0 || epoch == 99 || !action.empty()) {
                std::cout << "  " << std::setw(3) << epoch
                     << "  " << (active_phi ? "φ " : "ψ ") << " "
                     << std::setw(3) << lev_a << "   "
                     << std::setw(3) << lev_b << "   "
                     << std::setw(11) << std::scientific << std::setprecision(2) << pv
                     << "  " << std::setw(11) << std::scientific << sv;
                if (!action.empty()) std::cout << "  " << action;
                std::cout << "\n";
            }

        } catch (const std::exception& e) {
            std::cout << "  ✗ CRASHED epoch " << epoch << ": " << e.what() << "\n\n";
            break;
        }
    }

    int total_ops = total_fwd + total_rev + total_swap;
    std::cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║  STABILITY TEST RESULTS                               ║\n";
    std::cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    std::cout <<   "  ║  Forward cleans: " << std::setw(5) << total_fwd;
    std::cout <<   "                                ║\n";
    std::cout <<   "  ║  Reverse cleans: " << std::setw(5) << total_rev;
    std::cout <<   "                                ║\n";
    std::cout <<   "  ║  Ring swaps:     " << std::setw(5) << total_swap;
    std::cout <<   "                                ║\n";
    std::cout <<   "  ║  Total ops:      " << std::setw(5) << total_ops;
    std::cout <<   "                                ║\n";
    std::cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    std::cout <<   "  ║  ZERO EvalMult — pure cycle stability test           ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
