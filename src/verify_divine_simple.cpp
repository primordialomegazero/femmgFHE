#include "openfhe.h"  // Use relative include
#include <iostream>
using namespace lbcrypto;
using namespace std;

int main() {
    cout << "Testing OpenFHE..." << endl;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(LEVELEDSHE);
    cc->Enable(KEYSWITCH);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    cout << "OpenFHE initialized successfully!" << endl;
    return 0;
}
