// ============================================
// φ-HOMOMORPHIC ROTATION — Walang Pattern Leak
// Ang rotation ay nasa encrypted φ-power
// Hindi visible sa attacker — homomorphic
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double exponent) {
        // Ang state ay φ^exponent — homomorphic sa exponent
        vector<double> v(8, exponent);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-HOMOMORPHIC ROTATION — Walang Leak\n";
    cout << "========================================\n\n";
    cout << "  Ang rotation ay nasa encrypted φ-power\n";
    cout << "  Hindi visible sa attacker\n\n";

    // ============================================
    // TEST: Homomorphic na rotation
    // ============================================
    cout << "  TEST: Homomorphic rotation\n\n";

    // Initial: exponent = 3 (φ³)
    auto ct = encrypt_state(3.0);

    // Ang rotation ay ang pag-advance ng exponent
    // φ^n → φ^(n+1) → φ^(n+2) → ...
    // Ito ay homomorphic — EvalAdd(1) sa exponent

    // Ang Fibonacci mod 8 rotation ay maaaring i-encode
    // bilang exponent arithmetic — homomorphic

    int N = 1000;

    vector<double> delta(8, 1.0);  // +1 sa exponent bawat step
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, pt_delta);  // homomorphic rotation
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct);
    
    cout << "    Final exponent: " << v_final[0] << "\n";
    cout << "    Expected: " << 3.0 + N << "\n";
    cout << "    Match: " << (abs(v_final[0] - (3.0 + N)) < 0.1 ? "✅" : "❌") << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    cout << "  KEY: Ang exponent ay HOMOMORPHIC\n";
    cout << "  Ang attacker ay hindi nakakakita ng rotation pattern\n";
    cout << "  Dahil ang rotation ay nasa encrypted exponent\n";

    return 0;
}
