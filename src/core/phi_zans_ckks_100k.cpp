#include <openfhe.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now = system_clock::now(); auto t = system_clock::to_time_t(now); stringstream ss; ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S"); return ss.str(); }

int main() {
    ofstream log("ckks_zans_100k_results.txt");
    auto msg = [&](const string& s) { cout << s << flush; log << s << flush; };

    msg("\n===============================================================\n");
    msg("  CKKS ZANS 100K TEST\n");
    msg("  START: " + ts() + "\n===============================================================\n\n");

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20); params.SetScalingModSize(40);
    params.SetBatchSize(4096); params.SetRingDim(8192);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vector<double>{42.0}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vector<double>{0.0}));

    int steps = 100000;
    msg("  Running " + to_string(steps) + " ZANS additions...\n");
    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < steps; i++) ct = cc->EvalAdd(ct, anchor0);
    double elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() / 1000.0;

    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    double res = pt->GetRealPackedValue()[0];

    msg("  Steps: " + to_string(steps) + " | Result: " + to_string(res) + " | Time: " + to_string(elapsed/1000.0) + "s | Throughput: " + to_string((int)(steps/(elapsed/1000.0))) + " ops/s\n");
    msg("  Status: " + string(abs(res-42.0)<0.01 ? "PASSED" : "FAILED") + "\n\n");
    log.close();
}
