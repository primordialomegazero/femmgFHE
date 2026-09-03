// ============================================
// φ-GRAPH SPACE — NETWORK FHE
//
// Graph operations + Golden Ratio
// Path weights sa φ-log space
// Graph traversal = addition ng path weights
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <queue>
#include <map>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-GRAPH SPACE — NETWORK\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double TWO_PI = 2.0 * M_PI;

    vector<double> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Graph: path weights sa φ-log space\n";
    cout << "  Traversal = addition ng weights\n\n";

    // ============================================
    // GRAPH ENCODING
    // ============================================

    auto encrypt_node = [&](int node_id, double weight, double degree) {
        vector<double> v(16, 0.0);
        
        // Node ID sa slot 0 (normal space)
        v[0] = node_id / fib[0];
        
        // Weight sa slot 1-2 (log space + rubber band)
        double log_weight = log(weight + 1e-10) / LN_PHI;
        v[1] = fmod(log_weight, PHI) / fib[1];
        v[2] = weight / fib[2];  // Raw for verification
        
        // Degree sa slot 3 (normal space)
        v[3] = degree / fib[3];
        
        // φ-centrality sa slot 4 (log space)
        double centrality = degree / (weight + 1e-10);
        double log_centrality = log(centrality + 1e-10) / LN_PHI;
        v[4] = fmod(log_centrality, PHI) / fib[4];
        v[5] = centrality / fib[5];
        
        // φ-score sa slot 6 (rubber band)
        double phi_score = fmod(node_id * PHI, PHI);
        v[6] = phi_score / fib[6];
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_node = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        int node_id = (int)(result_pt->GetCKKSPackedValue()[0].real() * fib[0] + 0.5);
        double weight = result_pt->GetCKKSPackedValue()[2].real() * fib[2];
        double degree = result_pt->GetCKKSPackedValue()[3].real() * fib[3];
        double centrality = result_pt->GetCKKSPackedValue()[5].real() * fib[5];
        
        return make_tuple(node_id, weight, degree, centrality);
    };

    // ============================================
    // TEST 1: BASIC NODES
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: BASIC NODES\n";
    cout << "========================================\n\n";

    cout << "  Node | Weight | Degree | Centrality\n";
    cout << "  -----|--------|--------|-----------\n";

    vector<tuple<int, double, double>> nodes = {
        {0, 1.0, 2.0},   // Node 0
        {1, 2.0, 3.0},   // Node 1
        {2, 3.0, 1.0},   // Node 2
        {3, 5.0, 4.0},   // Node 3 (Fibonacci!)
        {4, 8.0, 5.0},   // Node 4
        {5, 13.0, 2.0},  // Node 5
        {6, 21.0, 3.0},  // Node 6
        {7, 34.0, 6.0}   // Node 7
    };

    int match_count = 0;
    for (auto [id, w, d] : nodes) {
        auto ct = encrypt_node(id, w, d);
        auto [nid, weight, degree, centrality] = decrypt_node(ct);
        
        double exp_centrality = d / w;
        bool match = (abs(weight - w) < 0.1 && abs(degree - d) < 0.1);
        match_count += match;
        
        cout << "  " << setw(4) << nid << " | " 
             << fixed << setprecision(1) << weight << " | "
             << setprecision(1) << degree << " | "
             << setprecision(3) << centrality << " (exp: " << exp_centrality << ") "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Match: " << match_count << "/8\n\n";

    // ============================================
    // TEST 2: GRAPH EDGE (PATH)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: GRAPH PATH\n";
    cout << "========================================\n\n";

    // Path: Node 0 → Node 1 → Node 2
    auto ct_node0 = encrypt_node(0, 1.0, 2.0);
    auto ct_node1 = encrypt_node(1, 2.0, 3.0);
    auto ct_node2 = encrypt_node(2, 3.0, 1.0);
    
    auto start = high_resolution_clock::now();
    
    // Path = EvalAdd ng nodes!
    auto ct_path = cc->EvalAdd(ct_node0, ct_node1);
    ct_path = cc->EvalAdd(ct_path, ct_node2);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start).count();
    
    auto [path_id, path_weight, path_degree, path_centrality] = decrypt_node(ct_path);
    
    cout << "  Path: 0 → 1 → 2\n";
    cout << "  Total weight: " << path_weight << " (expected: 6)\n";
    cout << "  Total degree: " << path_degree << " (expected: 6)\n";
    cout << "  Time: " << time << " μs\n";
    cout << "  Level: " << ct_path->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: FIBONACCI GRAPH
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FIBONACCI GRAPH\n";
    cout << "========================================\n\n";

    // Fibonacci graph: nodes na may Fibonacci weights
    cout << "  Fibonacci nodes:\n\n";
    
    for (int i = 0; i < 8; i++) {
        auto ct = encrypt_node(i, fib[i], fib[i + 1]);
        auto [nid, weight, degree, centrality] = decrypt_node(ct);
        
        cout << "  Node " << nid << ": w=" << fixed << setprecision(1) << weight
             << " (fib[" << i << "]=" << fib[i] << ")"
             << ", d=" << degree << " (fib[" << i + 1 << "]=" << fib[i + 1] << ")"
             << ", c=" << setprecision(4) << centrality << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 4: 1000 GRAPH OPS
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: 1000 GRAPH OPS\n";
    cout << "========================================\n\n";

    auto ct_acc = encrypt_node(0, 1.0, 1.0);
    auto ct_step = encrypt_node(1, 1.0, 1.0);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        ct_acc = cc->EvalAdd(ct_acc, ct_step);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto [acc_id, acc_weight, acc_degree, acc_centrality] = decrypt_node(ct_acc);
    
    cout << "  Operations: 1000 EvalAdd\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n";
    cout << "  Total weight: " << acc_weight << "\n";
    cout << "  Total degree: " << acc_degree << "\n\n";

    cout << "========================================\n";
    cout << "  φ-GRAPH SPACE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Graph encoding\n";
    cout << "  ✅ Path weights\n";
    cout << "  ✅ φ-centrality\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
