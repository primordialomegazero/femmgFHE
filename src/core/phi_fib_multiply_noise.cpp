// ΦΩ0 — FIBONACCI-ZANS v3 NOISE MEASUREMENT
// Tracking noise scale across scalar multiplications
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

vector<int64_t> generate_fibonacci(int64_t limit) {
    vector<int64_t> fib = {1, 2};
    while(fib.back() <= limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    fib.pop_back();
    return fib;
}

vector<int64_t> zeckendorf_decompose(int64_t n) {
    vector<int64_t> fib = generate_fibonacci(n);
    vector<int64_t> result;
    for(int i = fib.size() - 1; i >= 0 && n > 0; i--) {
        if(fib[i] <= n) {
            result.push_back(fib[i]);
            n -= fib[i];
            i--;
        }
    }
    return result;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FIBONACCI-ZANS v3 NOISE TRACKING       ║\n";
    cout <<   "║  Scalar multiplication with Enc(0) stabilization║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Create anchors
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    // Get initial noise scale from a fresh ciphertext
    double initial_noise = zero_ct->GetNoiseScaleDeg();
    cout << "Φ Initial noise scale: " << initial_noise << "\n\n";

    // === EXPERIMENT 1: Single multiplications with increasing multipliers ===
    cout << "=== EXPERIMENT 1: MULTIPLIER SWEEP WITH NOISE ===\n";
    cout << "Base=3, tracking noise after each multiplication\n\n";

    vector<int64_t> b3 = {3};
    auto base3 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b3));
    double base_noise = base3->GetNoiseScaleDeg();
    cout << "  Base Enc(3) noise scale: " << base_noise << "\n\n";

    cout << "┌──────────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Mult     │ Result   │ Expected │ Noise    │ Adds     │\n";
    cout << "├──────────┼──────────┼──────────┼──────────┼──────────┤\n";

    vector<int64_t> multipliers = {2, 3, 5, 7, 10, 21, 42, 100, 500, 1000};

    for(auto mult : multipliers) {
        auto p = zeckendorf_decompose(mult);
        auto res = zero_ct;
        int total_adds = 0;
        
        for(auto fp : p) {
            for(int64_t i = 0; i < fp; i++) {
                res = cc->EvalAdd(res, base3);
                res = cc->EvalAdd(res, anchor0);
                total_adds++;
            }
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, res, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int64_t expected = 3 * mult;
        double noise = res->GetNoiseScaleDeg();
        
        cout << "│ ×" << setw(7) << mult << " │ ";
        cout << setw(6) << val << "   │ ";
        cout << setw(6) << expected << "   │ ";
        cout << "≡ " << setw(6) << noise << " │ ";
        cout << setw(6) << total_adds << "   │\n";
    }
    cout << "└──────────┴──────────┴──────────┴──────────┴──────────┘\n\n";

    // === EXPERIMENT 2: Chained multiplications ===
    cout << "=== EXPERIMENT 2: CHAINED MULTIPLICATIONS ===\n";
    cout << "Starting from 2, multiplying by 3 repeatedly with Enc(0) stabilization\n\n";

    vector<int64_t> start_vec = {2};
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    
    vector<int64_t> mult_vec = {3};
    auto three = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mult_vec));

    cout << "┌──────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Step │ Value    │ Expected │ Noise    │ Drift    │\n";
    cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";

    int64_t current_val = 2;
    double prev_noise = chain->GetNoiseScaleDeg();
    bool chain_ok = true;
    int steps = 0;

    // Print initial
    cout << "│   0  │ " << setw(6) << current_val << "   │ " 
         << setw(6) << current_val << "   │ "
         << "≡ " << setw(4) << prev_noise << "   │ "
         << "   -    │\n";

    for(int i = 0; i < 15 && chain_ok; i++) {
        // Multiply by 3 using tensor: 3 = 1+2
        auto part1 = cc->EvalMult(chain, three);
        part1 = cc->EvalAdd(part1, anchor0);
        
        chain = part1;
        chain = cc->EvalAdd(chain, anchor0);
        steps++;
        current_val *= 3;
        
        double noise = chain->GetNoiseScaleDeg();
        double drift = noise - prev_noise;
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, chain, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        cout << "│  " << setw(2) << (i+1) << "  │ ";
        cout << setw(6) << val << "   │ ";
        cout << setw(6) << current_val << "   │ ";
        
        if(noise == prev_noise) {
            cout << "≡ " << setw(4) << noise << "   │ ";
        } else {
            cout << setw(8) << noise << " │ ";
        }
        
        if(drift == 0) {
            cout << " 0.000  │\n";
        } else {
            cout << setw(8) << fixed << setprecision(6) << drift << " │\n";
        }
        
        if(val != current_val) {
            cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            chain_ok = false;
        }
        
        prev_noise = noise;
    }
    cout << "└──────┴──────────┴──────────┴──────────┴──────────┘\n";
    
    cout << "\n  Chain length: " << steps << " multiplications";
    if(chain_ok) cout << " (still going!)";
    cout << "\n";

    // === EXPERIMENT 3: UK×UK Chain with ZANS ===
    cout << "\n=== EXPERIMENT 3: UK×UK CHAIN WITH ZANS ===\n";
    cout << "Ciphertext × Ciphertext, stabilized with Enc(0)\n\n";

    // Restart
    auto uk_chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    current_val = 2;
    prev_noise = uk_chain->GetNoiseScaleDeg();
    chain_ok = true;
    int uk_steps = 0;

    cout << "┌──────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Step │ Value    │ Expected │ Noise    │ Drift    │\n";
    cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";
    cout << "│   0  │ " << setw(6) << current_val << "   │ " 
         << setw(6) << current_val << "   │ "
         << "≡ " << setw(4) << prev_noise << "   │ "
         << "   -    │\n";

    for(int i = 0; i < 15 && chain_ok; i++) {
        uk_chain = cc->EvalMult(uk_chain, three);
        uk_chain = cc->EvalAdd(uk_chain, anchor0);
        uk_steps++;
        current_val *= 3;
        
        double noise = uk_chain->GetNoiseScaleDeg();
        double drift = noise - prev_noise;
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, uk_chain, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        cout << "│  " << setw(2) << (i+1) << "  │ ";
        cout << setw(6) << val << "   │ ";
        cout << setw(6) << current_val << "   │ ";
        
        if(noise == prev_noise) {
            cout << "≡ " << setw(4) << noise << "   │ ";
        } else {
            cout << setw(8) << noise << " │ ";
        }
        
        if(drift == 0) {
            cout << " 0.000  │\n";
        } else {
            cout << setw(8) << fixed << setprecision(6) << drift << " │\n";
        }
        
        if(val != current_val) {
            cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            chain_ok = false;
        }
        
        prev_noise = noise;
    }
    cout << "└──────┴──────────┴──────────┴──────────┴──────────┘\n";
    
    cout << "\n  UK×UK Chain length: " << uk_steps << " multiplications";
    if(chain_ok) cout << " (still going!)";
    cout << "\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — MULTIPLICATION NOISE TRACKING COMPLETE  ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
