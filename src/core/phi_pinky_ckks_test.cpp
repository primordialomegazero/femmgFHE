#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetScalingModSize(50);
    params.SetBatchSize(2048);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    double M_val = 1000.0;
    
    vector<double> test_vals = {1.0, 10.0, 100.0, 500.0, 999.0, 1001.0, 5000.0};
    vector<double> M_vec = {M_val};
    
    auto M = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(M_vec));
    
    cout << "=== PINKY SWEAR CKKS TEST ===" << endl;
    cout << "Reference M = " << M_val << endl;
    cout << "If Pinky Swear works: overflow != 0 when value > M" << endl;
    cout << string(60, '-') << endl;
    
    for (double v : test_vals) {
        vector<double> v_vec = {v};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v_vec));
        
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow_ct = cc->EvalSub(ct, back);
        
        Plaintext pt_over;
        cc->Decrypt(keys.secretKey, overflow_ct, &pt_over);
        pt_over->SetLength(1);
        complex<double> over_complex = pt_over->GetCKKSPackedValue()[0];
        double over_val = over_complex.real();
        
        bool significant = (abs(over_val) > 0.01);
        string signal = significant ? "OVERFLOW DETECTED" : "clean";
        
        cout << "v=" << setw(8) << v 
             << " -> overflow=" << setw(12) << fixed << setprecision(6) << over_val
             << " [" << signal << "]" << endl;
    }
    
    cout << string(60, '-') << endl;
    return 0;
}
