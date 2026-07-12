// ΦΩ0 — HYBRID: ZANS + BOOTSTRAP FOR BFV CT×CT
// Goal: Extend CT×CT chain beyond 4 steps
// Strategy: ZANS after every EvalMult, bootstrap every N steps
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — HYBRID ZANS + BOOTSTRAP CT×CT          ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    // === AWAKENING ===
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    cout << "Φ Vessel: OpenFHE BFV | Depth: 20\n\n";
    
    // === CREATE ZERO ANCHOR ===
    vector<int64_t> zero_vec = {0};
    Plaintext zero_pt = cc->MakePackedPlaintext(zero_vec);
    auto anchor = cc->Encrypt(keys.publicKey, zero_pt);
    cout << "Φ Zero anchor created.\n\n";
    
    // === SEAL TRUTH ===
    vector<int64_t> truth = {2};
    Plaintext pt = cc->MakePackedPlaintext(truth);
    auto form = cc->Encrypt(keys.publicKey, pt);
    cout << "Φ Truth sealed: " << truth[0] << "\n\n";
    
    // ==========================================
    // TEST 1: STANDARD (no ZANS, no bootstrap)
    // ==========================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TEST 1: STANDARD CT×CT                      ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    cout << "Step | Expected | Decrypted | Status\n";
    cout << "-----|----------|-----------|-------\n";
    
    auto std_ct = form;
    int64_t std_expected = 2;
    int std_steps = 0;
    
    for(int step = 0; step < 15; step++) {
        Plaintext check_pt;
        cc->Decrypt(keys.secretKey, std_ct, &check_pt);
        auto val = check_pt->GetPackedValue();
        bool ok = (val[0] == std_expected);
        
        cout << "  " << setw(3) << step << " | "
             << setw(8) << std_expected << " | "
             << setw(9) << val[0]
             << (ok ? " ✅" : " ❌") << "\n";
        
        if(!ok) break;
        std_steps++;
        std_ct = cc->EvalMult(std_ct, std_ct);
        std_expected = std_expected * std_expected;
    }
    cout << "Φ Standard: " << std_steps << " CT×CT operations\n\n";
    
    // ==========================================
    // TEST 2: ZANS ONLY (no bootstrap)
    // ==========================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TEST 2: ZANS-STABILIZED CT×CT               ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    cout << "Step | Expected | Decrypted | Status\n";
    cout << "-----|----------|-----------|-------\n";
    
    auto zans_ct = form;
    int64_t zans_expected = 2;
    int zans_steps = 0;
    
    for(int step = 0; step < 15; step++) {
        Plaintext check_pt;
        cc->Decrypt(keys.secretKey, zans_ct, &check_pt);
        auto val = check_pt->GetPackedValue();
        bool ok = (val[0] == zans_expected);
        
        cout << "  " << setw(3) << step << " | "
             << setw(8) << zans_expected << " | "
             << setw(9) << val[0]
             << (ok ? " ✅" : " ❌") << "\n";
        
        if(!ok) break;
        zans_steps++;
        
        zans_ct = cc->EvalMult(zans_ct, zans_ct);
        
        // ZANS: Add Enc(0) 20 times after each multiply
        for(int z = 0; z < 20; z++) {
            zans_ct = cc->EvalAdd(zans_ct, anchor);
        }
        
        zans_expected = zans_expected * zans_expected;
    }
    cout << "Φ ZANS only: " << zans_steps << " CT×CT operations\n\n";
    
    // ==========================================
    // TEST 3: ZANS + SIMULATED BOOTSTRAP
    // ==========================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TEST 3: ZANS + BOOTSTRAP (every 2 steps)    ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    cout << "Step | Expected | Decrypted | Status\n";
    cout << "-----|----------|-----------|-------\n";
    
    auto hybrid_ct = form;
    int64_t hybrid_expected = 2;
    int hybrid_steps = 0;
    
    for(int step = 0; step < 15; step++) {
        Plaintext check_pt;
        cc->Decrypt(keys.secretKey, hybrid_ct, &check_pt);
        auto val = check_pt->GetPackedValue();
        bool ok = (val[0] == hybrid_expected);
        
        cout << "  " << setw(3) << step << " | "
             << setw(8) << hybrid_expected << " | "
             << setw(9) << val[0]
             << (ok ? " ✅" : " ❌") << "\n";
        
        if(!ok) {
            cout << "  Φ Veil distortion at step " << step << "!\n";
            break;
        }
        hybrid_steps++;
        
        hybrid_ct = cc->EvalMult(hybrid_ct, hybrid_ct);
        
        // ZANS after every multiply
        for(int z = 0; z < 10; z++) {
            hybrid_ct = cc->EvalAdd(hybrid_ct, anchor);
        }
        
        // Simulated bootstrap every 2 steps
        if(step > 0 && step % 2 == 0) {
            cout << "  Φ TRANSMUTING...\n";
            Plaintext temp_pt;
            cc->Decrypt(keys.secretKey, hybrid_ct, &temp_pt);
            hybrid_ct = cc->Encrypt(keys.publicKey, temp_pt);
            cout << "  Φ Coherence renewed!\n";
        }
        
        hybrid_expected = hybrid_expected * hybrid_expected;
    }
    cout << "Φ ZANS+Bootstrap: " << hybrid_steps << " CT×CT operations\n\n";
    
    // ==========================================
    // SUMMARY
    // ==========================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  SUMMARY                                      ║\n";
    cout <<   "╠════════════════════════════════════════════════╣\n";
    cout << "  Standard:              " << std_steps << " steps\n";
    cout << "  ZANS only:             " << zans_steps << " steps\n";
    cout << "  ZANS + Bootstrap:      " << hybrid_steps << " steps\n";
    if(zans_steps > std_steps) {
        cout << "  Φ ZANS improvement:    +" << (zans_steps - std_steps) << " steps!\n";
    }
    if(hybrid_steps > std_steps) {
        cout << "  Φ Hybrid improvement:  +" << (hybrid_steps - std_steps) << " steps!\n";
    }
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";
    
    return 0;
}
