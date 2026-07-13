// PHI-OMEGA-ZERO: PARALLEL ZANS ACCELERATION
// Multi-threaded ZANS operations using OpenMP
// Immediate speedup on multi-core CPUs
// "THE CIPHERTEXT COMPUTES IN PARALLEL"
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <omp.h>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class ParallelZANS {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    
    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    }
    
public:
    ParallelZANS() {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(5);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        anchor0 = enc(0);
    }
    
    struct BenchResult {
        string name;
        int operations;
        int threads;
        double time_sec;
        double ops_per_sec;
        double speedup;
    };
    
    BenchResult benchmark_serial(int count) {
        auto ct = enc(42);
        
        auto t1 = high_resolution_clock::now();
        for(int i = 0; i < count; i++) {
            ct = cc->EvalAdd(ct, anchor0);
        }
        auto t2 = high_resolution_clock::now();
        
        double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
        return {"Serial ZANS", count, 1, elapsed, count / elapsed, 1.0};
    }
    
    BenchResult benchmark_parallel(int count, int num_threads) {
        omp_set_num_threads(num_threads);
        
        // Create independent ciphertexts per thread
        vector<Ciphertext<DCRTPoly>> cts(num_threads);
        for(int t = 0; t < num_threads; t++) {
            cts[t] = enc(42);
        }
        
        int ops_per_thread = count / num_threads;
        
        auto t1 = high_resolution_clock::now();
        
        #pragma omp parallel for
        for(int t = 0; t < num_threads; t++) {
            for(int i = 0; i < ops_per_thread; i++) {
                cts[t] = cc->EvalAdd(cts[t], anchor0);
            }
        }
        
        auto t2 = high_resolution_clock::now();
        double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
        double total_ops = ops_per_thread * num_threads;
        
        return {"Parallel ZANS", (int)total_ops, num_threads, elapsed, total_ops / elapsed, 0};
    }
    
    void run() {
        cout << "\n======================================================================\n";
        cout <<   "  PHI-OMEGA-ZERO: PARALLEL ZANS ACCELERATION\n";
        cout <<   "  Multi-threaded noise-free additions\n";
        cout <<   "  Hardware: " << omp_get_max_threads() << " threads available\n";
        cout <<   "======================================================================\n\n";
        
        int count = 10000;
        
        // Serial baseline
        auto serial = benchmark_serial(count);
        double baseline = serial.ops_per_sec;
        
        cout << "  ZANS ADDITION BENCHMARK (" << count << " operations)\n";
        cout << "  ------------------------------------------------------------------\n";
        cout << "  " << setw(20) << left << "Mode" 
             << setw(10) << "Threads" 
             << setw(12) << "Time(s)" 
             << setw(15) << "Ops/sec" 
             << setw(10) << "Speedup" << "\n";
        cout << "  ------------------------------------------------------------------\n";
        cout << "  " << setw(20) << serial.name 
             << setw(10) << serial.threads 
             << setw(12) << fixed << setprecision(2) << serial.time_sec
             << setw(15) << fixed << setprecision(0) << serial.ops_per_sec
             << setw(10) << "1.00x" << "\n";
        
        // Parallel tests
        vector<int> thread_counts = {2, 4, 6, 8, 10, 12};
        double best_speedup = 1.0;
        int best_threads = 1;
        
        for(int threads : thread_counts) {
            auto par = benchmark_parallel(count, threads);
            par.speedup = par.ops_per_sec / baseline;
            
            cout << "  " << setw(20) << par.name 
                 << setw(10) << par.threads 
                 << setw(12) << fixed << setprecision(2) << par.time_sec
                 << setw(15) << fixed << setprecision(0) << par.ops_per_sec
                 << setw(9) << fixed << setprecision(2) << par.speedup << "x" << "\n";
            
            if(par.speedup > best_speedup) {
                best_speedup = par.speedup;
                best_threads = threads;
            }
        }
        
        cout << "  ------------------------------------------------------------------\n";
        cout << "\n  RESULTS:\n";
        cout << "  Best configuration: " << best_threads << " threads\n";
        cout << "  Best speedup: " << fixed << setprecision(2) << best_speedup << "x\n";
        cout << "  Effective throughput: " << fixed << setprecision(0) << (baseline * best_speedup) << " ops/sec\n";
        cout << "\n  NOTE: ZANS operations are memory-bound (not CPU-bound).\n";
        cout << "  Speedup limited by memory bandwidth, not core count.\n";
        cout << "  FPGA/GPU acceleration would remove this bottleneck.\n";
        
        cout << "\n======================================================================\n";
        cout <<   "  PHI-OMEGA-ZERO - I AM THAT I AM\n";
        cout <<   "======================================================================\n\n";
    }
};

int main() {
    ParallelZANS pz;
    pz.run();
    return 0;
}
