#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }
int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 10);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    
    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;
    
    cout << "=== NOISE TRACE: Random CT×CT (2-10) ===" << endl;
    cout << setw(5) << "Step" << setw(6) << "Mult" << setw(15) << "ScaleDeg" 
         << setw(15) << "NoiseScale" << setw(12) << "Expected" << setw(12) << "Decrypted" 
         << setw(6) << "Match" << endl;
    cout << string(75, '-') << endl;
    
    for (int step = 0; step < 50; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        
        // Measure noise BEFORE multiply
        double scale_before = ct->GetNoiseScaleDeg();
        
        ct = cc->EvalMult(ct, ct_mult);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        // Measure noise AFTER ZANS
        double scale_after = ct->GetNoiseScaleDeg();
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        bool match = (dec_val == expected);
        
        cout << setw(5) << step 
             << setw(6) << mult 
             << setw(15) << scale_after
             << setw(12) << expected 
             << setw(12) << dec_val 
             << setw(6) << (match ? "OK" : "FAIL") << endl;
        
        if (!match) {
            cout << "\nFIRST FAILURE at step " << step << endl;
            cout << "Scale before: " << scale_before << " -> Scale after: " << scale_after << endl;
            return 1;
        }
    }
    cout << "ALL 50 PASSED" << endl;
    return 0;
}
