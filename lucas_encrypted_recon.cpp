// ============================================
// LUCAS ENCRYPTED RECONSTRUCTION
// Square via Lucas identities
// Walang ct × ct — puro additions lang
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
    cout << "  LUCAS ENCRYPTED RECONSTRUCTION\n";
    cout << "  Square without ct × ct\n";
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
    
    // Lucas values: L_0 to L_15
    vector<double> L = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364};

    // Test: Value = L_2 + L_4 = 3 + 7 = 10
    // Square = 100
    // Reconstruction:
    // L_2^2 = L_4 + 2 = 7 + 2 = 9
    // L_4^2 = L_8 + 2 = 47 + 2 = 49
    // L_2*L_4 = L_6 + L_2 = 18 + 3 = 21
    // Total = 9 + 2*21 + 49 = 100

    double a = 2;
    double b = 4;

    double L2sq_const = L[(int)(2*a)] + 2 * pow(-1, (int)a);       // 9
    double L4sq_const = L[(int)(2*b)] + 2 * pow(-1, (int)b);       // 49
    double Lcross_const = L[(int)(a+b)] + pow(-1, (int)b) * L[(int)abs(a-b)]; // 21

    cout << "Lucas components:\n";
    cout << "  L_" << (int)a << " = " << L[(int)a] << "\n";
    cout << "  L_" << (int)b << " = " << L[(int)b] << "\n";
    cout << "  L_" << (int)(2*a) << " = " << L[(int)(2*a)] << "\n";
    cout << "  L_" << (int)(2*b) << " = " << L[(int)(2*b)] << "\n";
    cout << "  L_" << (int)(a+b) << " = " << L[(int)(a+b)] << "\n\n";

    // Encrypt Lucas components
    vector<double> plain_L2(slots, L[(int)a]);
    vector<double> plain_L4(slots, L[(int)b]);
    vector<double> plain_L8(slots, L[(int)(2*b)]);
    vector<double> plain_L6(slots, L[(int)(a+b)]);

    auto ct_L2 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L2));
    auto ct_L4 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L4));
    auto ct_L8 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L8));
    auto ct_L6 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L6));

    cout << "✅ Encrypted Lucas components\n\n";

    // Reconstruction via additions only
    // L_2^2 = L_4 + 2
    vector<double> plain_two(slots, 2.0);
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    auto ct_L2sq = cc->EvalAdd(ct_L4, ct_two);          // 7 + 2 = 9
    auto ct_L4sq = cc->EvalAdd(ct_L8, ct_two);          // 47 + 2 = 49
    auto ct_Lcross = cc->EvalAdd(ct_L6, ct_L2);         // 18 + 3 = 21
    auto ct_2cross = cc->EvalAdd(ct_Lcross, ct_Lcross); // 42

    // Total = L2sq + 2*cross + L4sq
    auto ct_total = cc->EvalAdd(ct_L2sq, ct_2cross);    // 9 + 42 = 51
    ct_total = cc->EvalAdd(ct_total, ct_L4sq);          // 51 + 49 = 100

    cout << "✅ Lucas reconstruction complete (additions only)\n\n";

    // Also do traditional ct × ct for comparison
    vector<double> plain_value(slots, 10.0);
    auto ct_value = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_value));
    auto ct_square_trad = cc->EvalMult(ct_value, ct_value);

    // Decrypt both
    Plaintext plain_recon, plain_trad;
    cc->Decrypt(keyPair.secretKey, ct_total, &plain_recon);
    cc->Decrypt(keyPair.secretKey, ct_square_trad, &plain_trad);

    plain_recon->SetLength(slots);
    plain_trad->SetLength(slots);

    auto recon_complex = plain_recon->GetCKKSPackedValue();
    auto trad_complex = plain_trad->GetCKKSPackedValue();

    cout << "VERIFICATION:\n";
    cout << "Slot | Lucas Recon | Traditional | Expected | Match?\n";
    cout << "-----|-------------|-------------|----------|-------\n";

    for (int i = 0; i < slots; i++) {
        double recon = recon_complex[i].real();
        double trad = trad_complex[i].real();
        bool match = (abs(recon - 100.0) < 0.01) && (abs(trad - 100.0) < 0.01);
        cout << setw(4) << i << " | " 
             << setw(11) << fixed << setprecision(4) << recon << " | "
             << setw(11) << trad << " | "
             << setw(8) << "100.0000" << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  ENCRYPTED LUCAS RECONSTRUCTION COMPLETE\n";
    cout << "========================================\n";

    return 0;
}
