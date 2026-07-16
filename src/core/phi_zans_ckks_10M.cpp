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
    ofstream log("ckks_zans_10M_results.txt");
    auto msg = [&](const string& s) { cout << s << flush; log << s << flush; };

    msg("\n===============================================================\n");
    msg("  CKKS ZANS 10M TEST\n  10,000,000 ZANS Additions on OpenFHE CKKS\n");
    msg("  HARDWARE: AMD Ryzen 5 2600 | RING DIM: 8192\n");
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

    int total = 10000000, chk = 1000000;
    msg("  SETUP COMPLETE. Starting 10M ZANS chain...\n\n");
    msg("  Step         Elapsed        TPS        ETA\n");
    msg("  " + string(60, '-') + "\n");

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < total; i++) {
        ct = cc->EvalAdd(ct, anchor0);
        if((i+1) % chk == 0) {
            double elap = duration_cast<seconds>(high_resolution_clock::now() - t1).count();
            double tps = (i+1) / elap;
            double rem = (total-i-1) / tps;
            stringstream l;
            l << "  " << setw(10) << (i+1) << "  " << (int)elap/3600 << "h" << ((int)elap%3600)/60 << "m" << (int)elap%60 << "s"
              << setw(10) << fixed << setprecision(0) << tps
              << setw(10) << (int)rem/3600 << "h" << ((int)rem%3600)/60 << "m\n";
            msg(l.str());
        }
    }

    double total_sec = duration_cast<milliseconds>(high_resolution_clock::now() - t1).count() / 1000.0;
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    double res = pt->GetRealPackedValue()[0];

    msg("\n=======================================================================\n");
    msg("  CKKS ZANS 10M — COMPLETE\n  Steps: " + to_string(total) + " | Value: " + to_string(res) + " (exp: 42.0)\n");
    msg("  Time: " + to_string(total_sec/3600.0) + "h | Completed: " + ts() + "\n");
    msg("  Status: " + string(abs(res-42.0)<0.01 ? "PASSED" : "FAILED") + "\n\n");
    log.close();
}
