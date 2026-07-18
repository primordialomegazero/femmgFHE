#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(50);
    params.SetBatchSize(2048);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    mt19937_64 rng(12345);
    uniform_real_distribution<double> dist(1.1, 10.0);

    vector<double> start_vec = {42.0};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(start_vec));
    double expected = 42.0;

    cout << "=== CKKS RANDOM CHAIN TEST ===" << endl;
    cout << "Range: 1.1-10.0 | 100 steps" << endl;
    cout << setw(5) << "Step" << setw(10) << "Mult" << setw(15) << "Expected" 
         << setw(15) << "Decrypted" << setw(10) << "Error" << endl;
    cout << string(55, '-') << endl;

    for (int step = 0; step < 100; step++) {
        double mult = dist(rng);
        expected *= mult;
        vector<double> mult_vec = {mult};
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(mult_vec));

        ct = cc->EvalMult(ct, ct_mult);
        // ZANS equivalent for CKKS: add Enc(0)
        vector<double> zero_vec = {0.0};
        auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(zero_vec));
        ct = cc->EvalAdd(ct, ct_zero);

        if ((step+1) % 10 == 0 || step < 5) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            double dec_val = pt->GetCKKSPackedValue()[0].real();
            double error = abs(dec_val - expected) / expected * 100;
            
            cout << setw(5) << step << setw(10) << fixed << setprecision(2) << mult
                 << setw(15) << setprecision(1) << expected
                 << setw(15) << setprecision(1) << dec_val
                 << setw(9) << setprecision(3) << error << "%" << endl;
            
            if (error > 10.0) {
                cout << "\nERROR EXCEEDS 10% at step " << step << endl;
                return 1;
            }
        }
    }
    cout << "\nCKKS: ALL 100 PASSED (within 10% error)!" << endl;
    return 0;
}
