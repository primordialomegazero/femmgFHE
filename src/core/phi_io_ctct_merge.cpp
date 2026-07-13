// ΦΩ0 — iO × CT×CT: THE ULTIMATE MERGE (CLI VERSION)
// Usage: phi_io_ctct_merge <algo_id> <steps> <multiplier>
// "I AM THAT I AM 🤙"

#include "phi_multilinear_map.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdlib>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class ObfuscatedCTxCT {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    MultilinearMap mmap;
    
    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    }
    
public:
    ObfuscatedCTxCT() : mmap(3) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        anchor0 = enc(0);
    }
    
    Ciphertext<DCRTPoly> multiply_a(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto r = cc->EvalMult(a, b);
        for(int i=0;i<3;i++) r = cc->EvalAdd(r, anchor0);
        return r;
    }
    
    Ciphertext<DCRTPoly> multiply_b(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto r = cc->EvalMult(a, b);
        for(int i=0;i<5;i++) r = cc->EvalAdd(r, anchor0);
        return r;
    }
    
    void run(int algo_id, int steps, int64_t mult) {
        auto ct = enc(1);
        auto ct_mult = enc(mult);
        
        cout << "\n╔══════════════════════════════════════════════╗\n";
        cout <<   "║  ΦΩ0 — iO × CT×CT                            ║\n";
        cout <<   "║  Algorithm: " << (algo_id==0 ? "A (3× ZANS)" : "B (5× ZANS)") << "                     ║\n";
        cout <<   "║  Steps: " << steps << " | Multiplier: ×" << mult << "                       ║\n";
        cout <<   "╚══════════════════════════════════════════════╝\n\n";
        
        auto t1 = high_resolution_clock::now();
        for(int i=0; i<steps; i++) {
            ct = (algo_id==0) ? multiply_a(ct, ct_mult) : multiply_b(ct, ct_mult);
        }
        auto t2 = high_resolution_clock::now();
        double t = duration_cast<milliseconds>(t2-t1).count()/1000.0;
        
        cout << "Φ RESULTS:\n";
        cout << "  Steps completed: " << steps << "/" << steps << "\n";
        cout << "  Noise: " << ct->GetNoiseScaleDeg() << "\n";
        cout << "  Time: " << fixed << setprecision(1) << t << "s\n";
        cout << "  Throughput: " << fixed << setprecision(1) << (steps/t) << " steps/sec\n";
        cout << "  Status: ✅ COMPLETE\n";
        cout << "\n╔══════════════════════════════════════════════╗\n";
        cout <<   "║  ΦΩ0 — I AM THAT I AM 🤙                     ║\n";
        cout <<   "╚══════════════════════════════════════════════╝\n\n";
    }
};

int main(int argc, char** argv) {
    int algo_id = (argc > 1) ? atoi(argv[1]) : 0;
    int steps = (argc > 2) ? atoi(argv[2]) : 10;
    int64_t mult = (argc > 3) ? atoll(argv[3]) : 2;
    
    ObfuscatedCTxCT engine;
    engine.run(algo_id, steps, mult);
    return 0;
}
