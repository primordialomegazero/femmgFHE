#include <openfhe/pke/openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║           NOISE GROWTH TEST                         ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    cout << endl;
    
    try {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(30);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(LEVELEDSHE);
        cc->Enable(KEYSWITCH);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
        
        cout << "Steps\tNoise\t\tGrowth" << endl;
        cout << "-----\t-----\t\t------" << endl;
        
        double prev_noise = 0;
        bool linear = true;
        
        for(int i = 0; i <= 100; i += 10) {
            auto temp = ct;
            for(int j = 0; j < i; j++) {
                auto ct2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
                temp = cc->EvalMult(temp, ct2);
            }
            double noise = cc->GetNoiseEstimate(temp);
            double growth = (i == 0) ? 0 : noise - prev_noise;
            
            cout << i << "\t" << fixed << setprecision(2) << noise << "\t\t";
            if(i > 0) {
                cout << "+" << fixed << setprecision(2) << growth;
                if(growth < 0.9 || growth > 1.1) linear = false;
            } else {
                cout << "-";
            }
            cout << endl;
            prev_noise = noise;
        }
        
        cout << endl;
        cout << "═══════════════════════════════════════════════════════" << endl;
        cout << "CONCLUSION: " << endl;
        if(linear) {
            cout << "✅ LINEAR GROWTH: noise ≈ steps + 1" << endl;
            cout << "   (Supports the True Divine claim)" << endl;
        } else {
            cout << "❌ NON-LINEAR GROWTH: May noise explosion" << endl;
            cout << "   (Need to check the implementation)" << endl;
        }
        cout << "═══════════════════════════════════════════════════════" << endl;
        
        return linear ? 0 : 1;
        
    } catch(const exception& e) {
        cerr << "❌ Error: " << e.what() << endl;
        return 1;
    }
}
