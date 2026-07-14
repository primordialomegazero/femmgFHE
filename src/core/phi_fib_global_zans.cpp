// PHI-OMEGA-ZERO: FIBONACCI-INDEXED GLOBAL CONSCIOUSNESS ZANS
// Primes at Fibonacci positions vs Uniform sampling
// Tests if phi-spaced primes give better batch performance
// "FIBONACCI SELECTS. PRIMES CANCEL. ZANS STABILIZES."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>
#include <numeric>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

vector<int64_t> generate_primes(int count) {
    vector<int64_t> primes;
    vector<bool> is_prime(1000000, true);
    is_prime[0] = is_prime[1] = false;
    for(int64_t i = 2; i < 1000000 && (int)primes.size() < count; i++) {
        if(is_prime[i]) {
            primes.push_back(i);
            for(int64_t j = i*i; j < 1000000; j += i) is_prime[j] = false;
        }
    }
    return primes;
}

vector<int64_t> generate_fibonacci(int limit) {
    vector<int64_t> fib = {1, 2};
    while(fib.back() < limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    return fib;
}

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: FIBONACCI-INDEXED GLOBAL ZANS\n";
    cout <<   "  Phi-Spaced Prime Selection for Batch Consensus\n";
    cout <<   "======================================================================\n\n";

    // Generate 500 primes (large pool)
    auto all_primes = generate_primes(500);
    
    // Generate Fibonacci indices up to 500
    auto fib_indices = generate_fibonacci(500);
    
    // ============================================
    // METHOD A: Uniform — first N primes
    // ============================================
    vector<int64_t> uniform_primes;
    int uniform_count = 50;
    for(int i = 0; i < uniform_count; i++) {
        uniform_primes.push_back(all_primes[i]);
    }
    
    // ============================================
    // METHOD B: Fibonacci-indexed primes
    // ============================================
    vector<int64_t> fib_primes;
    for(auto idx : fib_indices) {
        if(idx > 0 && idx <= (int64_t)all_primes.size()) {
            fib_primes.push_back(all_primes[idx - 1]); // 0-indexed
        }
    }
    
    cout << "  PRIME SELECTION COMPARISON:\n";
    cout << "  " << string(70, '-') << "\n";
    cout << "  Uniform:   " << uniform_primes.size() << " primes (first " << uniform_count << ")\n";
    cout << "  First 5:   ";
    for(int i = 0; i < 5; i++) cout << uniform_primes[i] << " ";
    cout << "\n  Last 5:    ";
    for(int i = uniform_primes.size()-5; i < (int)uniform_primes.size(); i++) cout << uniform_primes[i] << " ";
    cout << "\n  Range:     " << uniform_primes.front() << " to " << uniform_primes.back();
    cout << "\n  Sum:       " << accumulate(uniform_primes.begin(), uniform_primes.end(), 0LL);
    cout << "\n\n";
    
    cout << "  Fibonacci: " << fib_primes.size() << " primes (at Fib positions)\n";
    cout << "  Indices:   ";
    for(auto idx : fib_indices) cout << idx << " ";
    cout << "\n  Primes:    ";
    for(int i = 0; i < min(8, (int)fib_primes.size()); i++) cout << fib_primes[i] << " ";
    if(fib_primes.size() > 8) cout << "...";
    cout << "\n  Range:     " << fib_primes.front() << " to " << fib_primes.back();
    cout << "\n  Sum:       " << accumulate(fib_primes.begin(), fib_primes.end(), 0LL);
    cout << "\n  Ratio Fib/Uniform count: " << fixed << setprecision(2) 
         << (double)fib_primes.size() / uniform_primes.size();
    cout << "\n  Ratio Fib/Uniform sum:   " << fixed << setprecision(2)
         << (double)accumulate(fib_primes.begin(), fib_primes.end(), 0LL) / 
            accumulate(uniform_primes.begin(), uniform_primes.end(), 0LL);
    cout << "\n\n";

    // ============================================
    // SETUP FHE
    // ============================================
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Build anchors
    auto build_anchor = [&](const vector<int64_t>& primes) {
        auto anchor = enc(0);
        int64_t pos_sum = 0, neg_sum = 0;
        for(auto p : primes) {
            anchor = cc->EvalAdd(anchor, enc(p));
            anchor = cc->EvalAdd(anchor, enc(-p));
            pos_sum += p;
            neg_sum += -p;
        }
        return make_pair(anchor, make_pair(pos_sum, neg_sum));
    };

    auto [uniform_anchor, uniform_sums] = build_anchor(uniform_primes);
    auto [fib_anchor, fib_sums] = build_anchor(fib_primes);
    
    auto plain_anchor = enc(0);
    int test_ops = 50000; // 50K ops for faster test

    // ============================================
    // BENCHMARK
    // ============================================
    cout << "  PERFORMANCE COMPARISON (" << test_ops << " additions each):\n";
    cout << "  " << string(75, '-') << "\n";
    cout << "  " << setw(28) << left << "Method"
         << setw(10) << "Ops"
         << setw(10) << "Time(s)"
         << setw(10) << "Ops/s"
         << setw(12) << "Noise"
         << setw(10) << "Value\n";
    cout << "  " << string(75, '-') << "\n";

    // Test 1: Standard ZANS (baseline)
    auto ct1 = enc(42);
    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < test_ops; i++) ct1 = cc->EvalAdd(ct1, plain_anchor);
    auto t2 = high_resolution_clock::now();
    double time1 = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    cout << "  " << setw(28) << left << "Standard ZANS (1 pair)"
         << setw(10) << test_ops
         << setw(10) << fixed << setprecision(1) << time1
         << setw(10) << fixed << setprecision(0) << (test_ops/time1)
         << setw(12) << ct1->GetNoiseScaleDeg()
         << setw(10) << dec(ct1) << "\n";

    // Test 2: Uniform 50-primes Global ZANS
    auto ct2 = enc(42);
    t1 = high_resolution_clock::now();
    for(int i = 0; i < test_ops; i++) ct2 = cc->EvalAdd(ct2, uniform_anchor);
    t2 = high_resolution_clock::now();
    double time2 = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
    double effective_ops2 = test_ops * uniform_primes.size();

    cout << "  " << setw(28) << left << "Uniform Global (" + to_string(uniform_primes.size()) + " pairs)"
         << setw(10) << test_ops
         << setw(10) << fixed << setprecision(1) << time2
         << setw(10) << fixed << setprecision(0) << (test_ops/time2)
         << setw(12) << ct2->GetNoiseScaleDeg()
         << setw(10) << dec(ct2) << "\n";

    // Test 3: Fibonacci-indexed Global ZANS
    auto ct3 = enc(42);
    t1 = high_resolution_clock::now();
    for(int i = 0; i < test_ops; i++) ct3 = cc->EvalAdd(ct3, fib_anchor);
    t2 = high_resolution_clock::now();
    double time3 = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
    double effective_ops3 = test_ops * fib_primes.size();

    cout << "  " << setw(28) << left << "Fibonacci Global (" + to_string(fib_primes.size()) + " pairs)"
         << setw(10) << test_ops
         << setw(10) << fixed << setprecision(1) << time3
         << setw(10) << fixed << setprecision(0) << (test_ops/time3)
         << setw(12) << ct3->GetNoiseScaleDeg()
         << setw(10) << dec(ct3) << "\n";

    cout << "  " << string(75, '-') << "\n\n";

    // ============================================
    // EFFICIENCY ANALYSIS
    // ============================================
    double effective_tps2 = effective_ops2 / time2;
    double effective_tps3 = effective_ops3 / time3;
    double pair_efficiency2 = effective_tps2 / uniform_primes.size();
    double pair_efficiency3 = effective_tps3 / fib_primes.size();

    cout << "  EFFECTIVE THROUGHPUT (accounting for batch size):\n";
    cout << "  " << string(55, '-') << "\n";
    cout << "  Uniform " << uniform_primes.size() << " pairs:   "
         << fixed << setprecision(0) << effective_tps2 << " effective ops/s";
    cout << "  | " << fixed << setprecision(1) << pair_efficiency2 << " per pair\n";
    cout << "  Fibonacci " << fib_primes.size() << " pairs: "
         << fixed << setprecision(0) << effective_tps3 << " effective ops/s";
    cout << "  | " << fixed << setprecision(1) << pair_efficiency3 << " per pair\n";
    cout << "  " << string(55, '-') << "\n";
    
    double improvement = ((pair_efficiency3 - pair_efficiency2) / pair_efficiency2) * 100;
    cout << "  Per-pair efficiency change: " << fixed << setprecision(1) << improvement << "%";
    if(improvement > 0) cout << " (Fibonacci BETTER)";
    else if(improvement < 0) cout << " (Uniform BETTER)";
    else cout << " (EQUAL)";
    cout << "\n\n";

    cout << "======================================================================\n";
    cout <<   "  FIBONACCI-INDEXED GLOBAL ZANS: COMPLETE\n";
    cout <<   "======================================================================\n\n";

    cout << "  I AM THAT I AM\n\n";

    return 0;
}
