// ΦΩ0 — BINFHE ZANS BREAKTHROUGH v1
// Applying Zero-Anchor Noise Stabilization to BinFHE
// Hypothesis: Enc(0) stabilization = fewer bootstraps = DRAMATIC speedup
// "I AM THAT I AM"

#include <openfhe/binfhe/binfhecontext.h>
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

BinFHEContext* g_cc = nullptr;
LWEPrivateKey g_sk = nullptr;
LWECiphertext g_anchor0 = nullptr; // ZANS Enc(0) for stabilization

LWECiphertext clone_ct(const LWECiphertext& ct) {
    return std::make_shared<LWECiphertextImpl>(*ct);
}

// ============================================
// STANDARD GATES (with Bootstrap)
// ============================================
LWECiphertext NAND_STD(const LWECiphertext& a, const LWECiphertext& b) {
    return g_cc->Bootstrap(g_cc->EvalBinGate(NAND, a, b));
}

LWECiphertext AND_STD(const LWECiphertext& a, const LWECiphertext& b) {
    auto nand_ab = NAND_STD(a, b);
    return NAND_STD(nand_ab, clone_ct(nand_ab));
}

// ============================================
// ZANS-OPTIMIZED GATES (Minimal Bootstrap)
// ============================================

// Hypothesis: After EvalBinGate, apply ZANS stabilization instead of full Bootstrap
LWECiphertext NAND_ZANS(const LWECiphertext& a, const LWECiphertext& b) {
    auto result = g_cc->EvalBinGate(NAND, a, b);
    // ZANS stabilization: add Enc(0) instead of Bootstrap
    // This is the BREAKTHROUGH — if noise stays low, we skip Bootstrap!
    result = g_cc->EvalAdd(result, g_anchor0);
    return result;
}

LWECiphertext AND_ZANS(const LWECiphertext& a, const LWECiphertext& b) {
    auto nand_ab = NAND_ZANS(a, b);
    return NAND_ZANS(nand_ab, clone_ct(nand_ab));
}

// ============================================
// BATCH BOOTSTRAP (Bootstrap every N gates)
// ============================================
struct BatchGate {
    vector<LWECiphertext> gates;
    int bootstrapInterval;
    int gateCount;
    
    BatchGate(int interval) : bootstrapInterval(interval), gateCount(0) {}
    
    LWECiphertext NAND(const LWECiphertext& a, const LWECiphertext& b) {
        auto result = g_cc->EvalBinGate(NAND, a, b);
        gateCount++;
        
        // Bootstrap only every N gates
        if (gateCount % bootstrapInterval == 0) {
            result = g_cc->Bootstrap(result);
        } else {
            // ZANS stabilization
            result = g_cc->EvalAdd(result, g_anchor0);
        }
        
        return result;
    }
    
    LWECiphertext AND(const LWECiphertext& a, const LWECiphertext& b) {
        auto nand_ab = NAND(a, b);
        return NAND(nand_ab, clone_ct(nand_ab));
    }
};

// ============================================
// BENCHMARK: Standard vs ZANS vs Batch
// ============================================

struct BenchResult {
    string method;
    int numGates;
    double timeMs;
    double gatesPerSec;
    int bootstraps;
    bool correct;
};

BenchResult benchmarkStandard(int bits, const vector<LWECiphertext>& a, 
                               const vector<LWECiphertext>& b, int expected) {
    auto start = high_resolution_clock::now();
    int bootstraps = 0;
    
    // Standard 4-bit multiply using NAND_STD
    auto zero = g_cc->Encrypt(g_sk, 0);
    vector<vector<LWECiphertext>> partial(bits, vector<LWECiphertext>(2*bits));
    
    for(int i = 0; i < bits; i++)
        for(int j = 0; j < 2*bits; j++)
            partial[i][j] = clone_ct(zero);
    
    // AND gates
    for(int i = 0; i < bits; i++) {
        for(int j = 0; j < bits; j++) {
            partial[i][i+j] = AND_STD(a[j], b[i]);
            bootstraps += 4; // Each AND = 2 NAND, each NAND = 1 Bootstrap
        }
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<milliseconds>(end - start).count();
    
    // Verify (decrypt first bit only for speed)
    int result = 0;
    for(int k = 0; k < 2*bits; k++) {
        LWEPlaintext bit;
        g_cc->Decrypt(g_sk, partial[0][k], &bit);
        if(bit == 1) result |= (1 << k);
    }
    
    return {
        "Standard",
        bits * bits,
        timeMs,
        (bits * bits * 1000.0) / timeMs,
        bootstraps,
        result == expected
    };
}

BenchResult benchmarkZANS(int bits, const vector<LWECiphertext>& a,
                           const vector<LWECiphertext>& b, int expected) {
    auto start = high_resolution_clock::now();
    
    auto zero = g_cc->Encrypt(g_sk, 0);
    vector<vector<LWECiphertext>> partial(bits, vector<LWECiphertext>(2*bits));
    
    for(int i = 0; i < bits; i++)
        for(int j = 0; j < 2*bits; j++)
            partial[i][j] = clone_ct(zero);
    
    // ZANS gates (NO bootstraps!)
    for(int i = 0; i < bits; i++) {
        for(int j = 0; j < bits; j++) {
            partial[i][i+j] = AND_ZANS(a[j], b[i]);
        }
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<milliseconds>(end - start).count();
    
    // Verify
    int result = 0;
    for(int k = 0; k < 2*bits; k++) {
        LWEPlaintext bit;
        g_cc->Decrypt(g_sk, partial[0][k], &bit);
        if(bit == 1) result |= (1 << k);
    }
    
    return {
        "ZANS (No Bootstrap)",
        bits * bits,
        timeMs,
        (bits * bits * 1000.0) / timeMs,
        0, // ZERO bootstraps!
        result == expected
    };
}

BenchResult benchmarkBatch(int bits, const vector<LWECiphertext>& a,
                            const vector<LWECiphertext>& b, int expected, int interval) {
    auto start = high_resolution_clock::now();
    
    BatchGate batch(interval);
    auto zero = g_cc->Encrypt(g_sk, 0);
    vector<vector<LWECiphertext>> partial(bits, vector<LWECiphertext>(2*bits));
    
    for(int i = 0; i < bits; i++)
        for(int j = 0; j < 2*bits; j++)
            partial[i][j] = clone_ct(zero);
    
    for(int i = 0; i < bits; i++) {
        for(int j = 0; j < bits; j++) {
            partial[i][i+j] = batch.AND(a[j], b[i]);
        }
    }
    
    auto end = high_resolution_clock::now();
    double timeMs = duration_cast<milliseconds>(end - start).count();
    
    // Verify
    int result = 0;
    for(int k = 0; k < 2*bits; k++) {
        LWEPlaintext bit;
        g_cc->Decrypt(g_sk, partial[0][k], &bit);
        if(bit == 1) result |= (1 << k);
    }
    
    int numBootstraps = batch.gateCount / interval;
    
    return {
        "Batch (BS every " + to_string(interval) + ")",
        bits * bits,
        timeMs,
        (bits * bits * 1000.0) / timeMs,
        numBootstraps,
        result == expected
    };
}

// ============================================
// MAIN
// ============================================

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — BINFHE ZANS BREAKTHROUGH TEST          ║\n";
    cout <<   "║  Hypothesis: Enc(0) stabilization = speedup   ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Initialize BinFHE
    auto cc = BinFHEContext();
    cc.GenerateBinFHEContext(TOY, false);
    auto sk = cc.KeyGen();
    cc.BTKeyGen(sk);
    
    g_cc = &cc;
    g_sk = sk;
    
    // Generate ZANS anchor
    g_anchor0 = cc.Encrypt(sk, 0);
    
    // Test: 2-bit multiply (3 × 3 = 9)
    cout << "Φ Test: 3 × 3 (2-bit)\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    // Encode 3 = 11 in binary
    auto a0 = cc.Encrypt(sk, 1); // LSB
    auto a1 = cc.Encrypt(sk, 1); // MSB
    auto b0 = cc.Encrypt(sk, 1);
    auto b1 = cc.Encrypt(sk, 1);
    
    vector<LWECiphertext> a = {a0, a1};
    vector<LWECiphertext> b = {b0, b1};
    
    // Run benchmarks
    auto stdResult = benchmarkStandard(2, a, b, 9);
    auto zansResult = benchmarkZANS(2, a, b, 9);
    auto batch5Result = benchmarkBatch(2, a, b, 9, 5);
    auto batch10Result = benchmarkBatch(2, a, b, 9, 10);
    
    // Print results
    cout << "\nΦ RESULTS:\n";
    cout << "┌─────────────────────────┬────────┬──────────┬────────────┬────────────┬─────────┐\n";
    cout << "│ Method                  │ Gates  │ Time(ms) │ Gates/sec  │ Bootstraps │ Correct │\n";
    cout << "├─────────────────────────┼────────┼──────────┼────────────┼────────────┼─────────┤\n";
    
    auto printResult = [](const BenchResult& r) {
        cout << "│ " << left << setw(23) << r.method 
             << " │ " << setw(6) << r.numGates
             << " │ " << setw(8) << fixed << setprecision(2) << r.timeMs
             << " │ " << setw(10) << fixed << setprecision(0) << r.gatesPerSec
             << " │ " << setw(10) << r.bootstraps
             << " │ " << setw(7) << (r.correct ? "✅" : "❌")
             << " │\n";
    };
    
    printResult(stdResult);
    printResult(zansResult);
    printResult(batch5Result);
    printResult(batch10Result);
    
    cout << "└─────────────────────────┴────────┴──────────┴────────────┴────────────┴─────────┘\n";
    
    // ZANS Breakthrough Verdict
    double speedup = stdResult.timeMs / max(zansResult.timeMs, 0.001);
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    if (zansResult.correct && zansResult.timeMs < stdResult.timeMs) {
        cout << "║  🔥 ZANS BREAKTHROUGH CONFIRMED!              ║\n";
        cout << "║  ZANS is " << fixed << setprecision(1) << speedup << "× FASTER than Standard      ║\n";
        cout << "║  ZERO bootstraps, result CORRECT              ║\n";
    } else if (zansResult.correct) {
        cout << "║  ⚡ ZANS works but similar speed              ║\n";
        cout << "║  Need larger test to see advantage            ║\n";
    } else {
        cout << "║  ❌ ZANS corrupted at this scale              ║\n";
        cout << "║  BinFHE noise different from BFV              ║\n";
    }
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
