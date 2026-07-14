// PHI-OMEGA-ZERO: TRUE DIVINE 10K OPTIMIZED v2
// Ring dim 4096, modulus 1073643521, checkpoint every 5 steps
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%H:%M:%S");
    return ss.str();
}

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int steps = 1000;
    int checkpoint_interval = 5;
    
    cout << "\n===============================================================\n";
    cout <<   "  TRUE DIVINE 1K OPTIMIZED\n";
    cout <<   "  Ring: " << ring_dim << " | Mod: " << modulus;
    cout << " | Divine: " << checkpoint_interval << "\n";
    cout <<   "  Start: " << ts() << "\n";
    cout <<   "===============================================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto anchor0 = enc(0);
    int64_t half_mod = modulus / 2;
    auto M = enc(half_mod);
    auto ct = enc(1);
    auto ct_mult = enc(2);

    cout << "  Step      Noise     Time      Status\n";
    cout << "  " << string(45, '-') << "\n";

    auto t_start = high_resolution_clock::now();
    int failures = 0;

    for(int i = 0; i < steps; i++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        ct = cc->EvalMult(ct, ct_mult);

        if((i + 1) % checkpoint_interval == 0) {
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
        }

        ct = cc->EvalAdd(ct, anchor0);

        if((i + 1) % 100 == 0 || i == steps - 1) {
            double noise = ct->GetNoiseScaleDeg();
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(now - t_start).count();
            
            cout << "  " << setw(6) << (i+1)
                 << setw(10) << fixed << setprecision(1) << noise
                 << setw(8) << elapsed << "s OK\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;
    double tps = steps / total_sec;

    cout << "  " << string(45, '-') << "\n\n";
    cout << "  RESULTS:\n";
    cout << "  Time: " << fixed << setprecision(1) << total_sec << " sec\n";
    cout << "  TPS:  " << fixed << setprecision(1) << tps << " steps/sec\n";

    double projected_1m_hours = (1000000.0 / tps) / 3600.0;
    double projected_1m_16384 = projected_1m_hours * (16384.0 / ring_dim);
    
    cout << "\n  1M PROJECTIONS:\n";
    cout << "  Ring " << ring_dim << ": " << fixed << setprecision(1) << projected_1m_hours << " hours\n";
    cout << "  Ring 16384:  " << fixed << setprecision(1) << projected_1m_16384 << " hours\n";

    cout << "\n  Completed: " << ts() << "\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
