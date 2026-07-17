// ΦΩ0 — FIBONACCI-DECOMPOSED TRUE DIVINE CT×CT TEST
// Decompose large multiplier via Zeckendorf, multiply small, ZANS each
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(50);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 10);
    FibonacciZANS fib(zans);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    
    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);  // Large range!
    
    int64_t start_val = 42;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
    int64_t expected = start_val;
    
    int total_steps = 50;
    
    cout << "\n=== FIBONACCI-DECOMPOSED TRUE DIVINE CT×CT ===" << endl;
    cout << "Range: 2-1000 | Steps: " << total_steps << endl;
    cout << "Method: Decompose multiplier via Zeckendorf, ZANS each sub-multiply" << endl;
    cout << string(70, '-') << endl;
    cout << setw(6) << "Step" << setw(10) << "Mult" << setw(6) << "Parts" << setw(16) << "Expected" << setw(16) << "Got" << setw(8) << "Match" << endl;
    cout << string(70, '-') << endl;
    
    int passed = 0;
    
    for(int step = 0; step < total_steps; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        
        // Zeckendorf decomposition of multiplier
        auto parts = fib.decompose(mult);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        
        // Pinky Swear before multiply
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        // Fibonacci-decomposed multiply: multiply by each Fibonacci part with ZANS
        for(auto fib_part : parts) {
            auto fib_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{fib_part}));
            ct = cc->EvalMult(ct, fib_ct);
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
        }
        
        // Divine + ZANS
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
        ct = pool.stabilize(ct);
        
        if((step + 1) % 10 == 0 || step == total_steps - 1) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool match = (dec_val == expected);
            if(match) passed++;
            
            cout << setw(6) << step
                 << setw(10) << mult
                 << setw(6) << parts.size()
                 << setw(16) << expected
                 << setw(16) << dec_val
                 << setw(8) << (match ? "OK" : "FAIL") << endl;
        }
    }
    
    cout << string(70, '-') << endl;
    cout << "Result: " << passed << "/" << ((total_steps/10)+1) << " checkpoints passed" << endl;
    cout << (passed == ((total_steps/10)+1) ? "FIBONACCI DECOMPOSITION WORKS!" : "SOME FAILURES") << endl;
    cout << endl;
    
    return 0;
}
