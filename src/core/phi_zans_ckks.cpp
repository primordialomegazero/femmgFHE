// PHI-OMEGA-ZERO: OpenFHE CKKS ZANS 10K TEST
// Cross-library + cross-scheme validation: CKKS approximate FHE
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
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    cout << "\n===============================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: CKKS ZANS 10K TEST\n";
    cout <<   "===============================================================\n";
    cout <<   "  HARDWARE: AMD Ryzen 5 2600 | LIBRARY: OpenFHE CKKS\n";
    cout <<   "  START: " << ts() << "\n";
    cout <<   "===============================================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(40);
    params.SetBatchSize(4096);
    params.SetRingDim(8192);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    vector<double> vals = {42.0};
    auto pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keys.publicKey, pt);

    vector<double> zero_vals = {0.0};
    auto pt_zero = cc->MakeCKKSPackedPlaintext(zero_vals);
    auto anchor0 = cc->Encrypt(keys.publicKey, pt_zero);

    int steps = 10000;
    cout << "  Running " << steps << " ZANS additions...\n";

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < steps; i++) {
        ct = cc->EvalAdd(ct, anchor0);
    }
    auto t2 = high_resolution_clock::now();
    double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    Plaintext pt_result;
    cc->Decrypt(keys.secretKey, ct, &pt_result);
    vector<double> result = pt_result->GetRealPackedValue();

    cout << "  Operations: " << steps << "\n";
    cout << "  Result: " << fixed << setprecision(6) << result[0] << " (expected: 42.0)\n";
    cout << "  Time: " << elapsed/1000.0 << "s\n";
    cout << "  Throughput: " << fixed << setprecision(0) << (steps / (elapsed/1000.0)) << " ops/s\n";
    bool ok = abs(result[0] - 42.0) < 0.01;
    cout << "  Status: " << (ok ? "PASSED" : "FAILED") << "\n\n";

    return 0;
}
