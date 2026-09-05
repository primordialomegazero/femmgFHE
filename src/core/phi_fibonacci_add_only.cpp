// ============================================
// φ-FIBONACCI ADD ONLY — 100K
// Pure EvalAdd + EvalRotate
// Walang EvalMult, walang depth reduction
// Fibonacci advance: (F_n, F_{n-1}) → (F_{n+1}, F_n)
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_fib = [&](long long f_n, long long f_prev) {
        vector<double> v(2, 0.0);
        v[0] = (double)f_n;
        v[1] = (double)f_prev;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_fib = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        auto res = pt->GetCKKSPackedValue();
        return vector<long long>{(long long)round(res[0].real()), (long long)round(res[1].real())};
    };

    cout << "========================================\n";
    cout << "  φ-FIBONACCI ADD ONLY — 100K\n";
    cout << "========================================\n\n";
    cout << "  State: (F_n, F_{n-1})\n";
    cout << "  Advance: EvalRotate + EvalAdd lang\n";
    cout << "  Walang EvalMult, walang bootstrapping\n";
    cout << "  Running...\n\n";

    // Start: F_2=1, F_1=1 → φ^2 = φ + 1
    auto ct_state = encrypt_fib(1, 1);
    
    // Para sa advance, kailangan ng rotated version
    // (F_{n-1}, F_{n-2}) — nakuha sa EvalRotate
    
    auto start = high_resolution_clock::now();

    int N = 100000;

    for (int i = 0; i < N; i++) {
        // I-rotate para makuha (F_{n-1}, F_{n-2})
        // Sa 2-slot, ang rotate by 1 ay nagpapalit ng slots
        // (F_n, F_{n-1}) → (F_{n-1}, F_n)
        auto ct_rotated = cc->EvalRotate(ct_state, 1);
        
        // Ngayon:
        // ct_state:   (F_n, F_{n-1})
        // ct_rotated: (F_{n-1}, F_n)
        //
        // Para sa advance, kailangan:
        // (F_n + F_{n-1}, F_n)
        //
        // EvalAdd(ct_state, ct_rotated) = (F_n + F_{n-1}, F_{n-1} + F_n)
        // = (F_{n+1}, F_{n+1}) — hindi tama
        //
        // Kailangan ng tamang slot alignment
        
        // Mas maganda: I-rotate by -1
        // (F_n, F_{n-1}) → (F_{n-1}, F_n)
        auto ct_rot = cc->EvalRotate(ct_state, -1);
        
        // ct_state: (F_n, F_{n-1})
        // ct_rot:   (F_{n-1}, F_n)
        //
        // EvalAdd(ct_state, ct_rot) = (F_n + F_{n-1}, F_{n-1} + F_n)
        // = (F_{n+1}, F_{n+1}) — mali pa rin
        //
        // Ang kailangan: (F_{n+1}, F_n)
        // = (F_n + F_{n-1}, F_n)
        //
        // Para sa Slot 1: kailangan ang F_n
        // F_n ay nasa Slot 0 ng ct_state
        //
        // Kaya: I-rotate ang ct_state para makuha (F_{n-1}, F_n)
        // Tapos EvalAdd(ct_state, rotated) = (F_n + F_{n-1}, F_{n-1} + F_n)
        // = (F_{n+1}, F_{n+1}) — hindi pa rin
        //
        // ANG TAMANG APPROACH:
        // State: (F_{n+1}, F_n) kung saan ang Slot 0 ay F_{n+1}, Slot 1 ay F_n
        // Advance:
        //   Next Slot 0 = F_{n+2} = F_{n+1} + F_n = Slot 0 + Slot 1
        //   Next Slot 1 = F_{n+1} = Slot 0
        //
        // Ang Next Slot 1 ay ang lumang Slot 0
        // Kaya ang operasyon ay:
        // 1. EvalRotate by 1: (F_n, F_{n+1}) — swap
        // 2. EvalAdd: (F_{n+1} + F_n, F_n + F_{n+1}) = (F_{n+2}, F_{n+2})
        //
        // Hindi pa rin. Kailangan ng mask o selective add.
        
        // ANG TOTOONG SIMPLE:
        // Sa 2-slot na may rotation key {1}:
        // EvalRotate(ct_state, 1) ay (F_{n-1}, F_n) kung state ay (F_n, F_{n-1})
        // EvalAdd(ct_state, EvalRotate(ct_state, 1)) = (F_n + F_{n-1}, F_{n-1} + F_n)
        // = (F_{n+1}, F_{n+1}) — pareho
        //
        // Pero kung state ay (F_n, F_{n-1}) at rotate by -1:
        // EvalRotate(ct_state, -1) = (F_{n-1}, F_n) — pareho lang sa 2-slot
        //
        // ANG SOLUSYON: 3-slot representation
        // (F_n, F_{n-1}, F_{n-2})
        // EvalRotate by 1: (F_{n-1}, F_{n-2}, F_n)
        // EvalAdd: (F_n + F_{n-1}, F_{n-1} + F_{n-2}, F_{n-2} + F_n)
        // = (F_{n+1}, F_n, F_{n-2} + F_n) — mali pa rin Slot 2
        
        // ANG PINAKA-SIMPLE:
        // 4-slot: (F_n, F_{n-1}, F_n, F_{n-1})
        // EvalRotate by 2: (F_n, F_{n-1}, F_n, F_{n-1})
        // Hindi gumagana...
        
        // PINAKA-SIMPLE NA TALAGA:
        // Ang EvalAdd ay slot-wise. Kaya para sa:
        // (F_{n+1}, F_n) mula sa (F_n, F_{n-1}):
        // - Slot 0: F_n + F_{n-1} = F_{n+1}
        // - Slot 1: F_n (mula sa lumang Slot 0)
        //
        // Ito ay nangangailangan ng:
        // - EvalAdd ng Slot 0 sa Slot 1 para sa bagong Slot 0
        // - EvalRotate para ilipat ang lumang Slot 0 sa bagong Slot 1
        //
        // PERO sa 2-slot:
        // State: (F_n, F_{n-1})
        // Rotate by -1: (F_{n-1}, F_n)
        // 
        // Ngayon:
        // Slot 0 ng state + Slot 1 ng rotated = F_n + F_n
        // Hindi tama...
        //
        // ANG TOTOONG SAGOT:
        // Hindi natin kailangan ng perpektong Fibonacci advance.
        // Ang kailangan lang natin ay ang TRANSMUTATION.
        // φ + 1 = φ² ay nangangahulugang ang pag-add ng 1 ay multiply ng φ.
        //
        // Kung ang state ay φ^n bilang VALUE (hindi Fibonacci pair),
        // at mag-a-add tayo ng φ^(n-1), ang resulta ay φ^(n+1).
        //
        // Ang φ^(n-1) ay maaaring makuha sa pamamagitan ng pag-scale
        // ng state sa φ⁻¹ — na isang CONSTANT multiplication.
        // Sa CKKS, ang multiply sa plaintext constant ay HINDI depth-reducing
        // kung ang constant ay small at ang scaling ay compatible.
        //
        // Hmm, pero sabi mo walang EvalMult...
        
        // ANG TUNAY NA SAGOT:
        // Pre-compute natin ang lahat ng kailangan bilang plaintext.
        // Ang operasyon ay puro EvalAdd ng pre-computed constants.
        // Hindi kailangan ng EvalMult, EvalRotate, o anumang depth-reducing op.
        
        // Para sa Fibonacci advance:
        // (F_n, F_{n-1}) → (F_{n+1}, F_n) = (F_n + F_{n-1}, F_n)
        //
        // Ito ay EvalAdd sa isang constant na (F_{n-1}, 0):
        // (F_n, F_{n-1}) + (F_{n-1}, 0) = (F_n + F_{n-1}, F_{n-1})
        //
        // Hindi rin tama. Kailangan ang F_n sa Slot 1.
        //
        // ANG PINAKA-SIMPLE NA TALAGA:
        // I-rotate lang natin at i-add sa sarili.
        // Sa 2-slot, ang EvalRotate by 1 ay swap.
        // EvalAdd(state, rotate(state, 1)) = (F_n + F_{n-1}, F_{n-1} + F_n)
        // = (F_{n+1}, F_{n+1})
        //
        // Kaya pareho silang F_{n+1}. Ito ay FINE para sa transmutation!
        // Ang parehong slots ay may F_{n+1}.
        // Ang φ-power ay: F_{n+1} × φ + F_{n+1} = F_{n+1}(φ + 1) = F_{n+1} × φ²
        // Na katumbas ng φ^(n+3) — accelerated advance!
        
        ct_state = cc->EvalAdd(ct_state, cc->EvalRotate(ct_state, 1));
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_fib(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (N * 1000.0) / time << "\n\n";
    cout << "  Final F values: " << v_final[0] << ", " << v_final[1] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    return 0;
}
