// ============================================
// LUCAS ENCODING SA OPENFHE
// Value = L_a + L_b
// Square = (L_a + L_b)^2 via Lucas identities
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  LUCAS ENCODING SA OPENFHE\n";
    cout << "  Value = L_a + L_b\n";
    cout << "  Square via Lucas identities\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;
    vector<double> L = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364};

    // Test: Value = L_2 + L_4 = 3 + 7 = 10
    double a = 2;
    double b = 4;
    double value = L[(int)a] + L[(int)b];
    double expected_square = value * value;

    cout << "TEST: Value = L_" << (int)a << " + L_" << (int)b << "\n";
    cout << "Value = " << L[(int)a] << " + " << L[(int)b] << " = " << value << "\n";
    cout << "Expected square = " << expected_square << "\n\n";

    vector<double> plain_value(slots, value);
    auto ct_value = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_value));

    cout << "✅ Encrypted value = " << value << "\n\n";

    // Approach 1: Traditional ct × ct
    auto ct_square_trad = cc->EvalMult(ct_value, ct_value);

    // Approach 2: Lucas identity reconstruction
    double L2sq = L[(int)(2*a)] + 2 * pow(-1, (int)a);
    double L4sq = L[(int)(2*b)] + 2 * pow(-1, (int)b);
    double Lcross = L[(int)(a+b)] + pow(-1, (int)b) * L[(int)abs(a-b)];

    double reconstructed = L2sq + 2*Lcross + L4sq;

    cout << "Lucas identity reconstruction:\n";
    cout << "  L_" << (int)a << "^2 = " << L2sq << "\n";
    cout << "  L_" << (int)b << "^2 = " << L4sq << "\n";
    cout << "  L_" << (int)a << "*L_" << (int)b << " = " << Lcross << "\n";
    cout << "  Reconstructed square = " << reconstructed << "\n\n";

    // Decrypt traditional result
    Plaintext plain_result;
    cc->Decrypt(keyPair.secretKey, ct_square_trad, &plain_result);
    plain_result->SetLength(slots);
    auto result_complex = plain_result->GetCKKSPackedValue();

    cout << "VERIFICATION:\n";
    cout << "Slot | Traditional | Lucas Recon | Expected | Match?\n";
    cout << "-----|-------------|-------------|----------|-------\n";

    for (int i = 0; i < slots; i++) {
        double trad = result_complex[i].real();
        bool match = (abs(trad - expected_square) < 0.01) && (abs(reconstructed - expected_square) < 0.01);
        cout << setw(4) << i << " | " 
             << setw(11) << fixed << setprecision(4) << trad << " | "
             << setw(11) << reconstructed << " | "
             << setw(8) << expected_square << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  LUCAS ENCODING TEST COMPLETE\n";
    cout << "========================================\n";

    return 0;
}
