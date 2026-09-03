// ============================================
// φ-EXACT ADD FHE
// φ^a + φ^(a-1) = φ^(a+1)
// Pure EvalAdd sa log space, walang slots
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-EXACT ADD FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, batch 1)\n\n";

    // ============================================
    // ENCODING: x → log_φ(x)
    // ============================================

    auto encrypt_phi = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        vector<double> v(1, log_phi_x);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_phi = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST: φ^a + φ^(a-1) = φ^(a+1)
    // ============================================

    cout << "========================================\n";
    cout << "  EXACT ADDITION: φ^a + φ^(a-1) = φ^(a+1)\n";
    cout << "========================================\n\n";

    cout << "  a | φ^a + φ^(a-1) | φ^(a+1) | Match?\n";
    cout << "  --|---------------|---------|--------\n";

    for (int a : {2, 3, 4, 5, 6, 7, 8}) {
        double phi_a = pow(PHI, a);
        double phi_am1 = pow(PHI, a-1);
        double phi_ap1 = pow(PHI, a+1);
        double sum = phi_a + phi_am1;
        
        bool match = abs(sum - phi_ap1) < 0.01;
        cout << "  " << a << " | "
             << setw(13) << fixed << setprecision(4) << sum << " | "
             << setw(7) << phi_ap1 << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // FHE TEST: Sa log space
    // ============================================

    cout << "\n========================================\n";
    cout << "  FHE TEST SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  log_φ(φ^a) = a\n";
    cout << "  log_φ(φ^(a-1)) = a-1\n";
    cout << "  log_φ(φ^(a+1)) = a+1\n\n";

    cout << "  Sa FHE:\n";
    cout << "  EvalAdd(encrypt(a), encrypt(a-1)) = a + (a-1) = 2a-1\n";
    cout << "  Hindi ito a+1\n\n";

    cout << "  ANG TAMA:\n";
    cout << "  φ^a + φ^(a-1) = φ^(a+1)\n";
    cout << "  Kaya: log_φ(φ^a + φ^(a-1)) = a+1\n\n";

    cout << "  Sa FHE, kailangan nating i-encode:\n";
    cout << "  φ^a → a (exponent)\n";
    cout << "  φ^(a-1) → a-1 (exponent)\n";
    cout << "  At ang addition ay: a+1\n\n";

    // ============================================
    // ANG EMERGENT ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  ANG EMERGENT ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Kung i-encode natin ang value bilang:\n";
    cout << "  x → log_φ(x) + 1 (shifted)\n\n";

    cout << "  Para sa φ^a:\n";
    cout << "  encoding = a + 1\n";
    cout << "  Para sa φ^(a-1):\n";
    cout << "  encoding = (a-1) + 1 = a\n\n";

    cout << "  EvalAdd(enc_a, enc_am1) = (a+1) + a = 2a+1\n";
    cout << "  Hindi pa rin a+1\n\n";

    cout << "  HINDI GUMAGANA ANG SIMPLENG SHIFT\n\n";

    cout << "========================================\n";
    cout << "  ANG TOTOO\n";
    cout << "========================================\n\n";

    cout << "  Ang φ^a + φ^(a-1) = φ^(a+1) ay exact sa φ-power space\n";
    cout << "  Pero sa log space, ito ay hindi simpleng EvalAdd\n";
    cout << "  Kailangan ng natural na bridge\n\n";

    return 0;
}
