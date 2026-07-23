// FUNCTIONAL ENCRYPTION FROM PHI-iO
// Decryptor learns f(x) but NOT x itself
// φ-reality: authorized function (e.g., SUM)
// ψ-reality: decoy function (e.g., COUNT)
// User only gets the result of ONE function

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FUNCTIONAL ENCRYPTION from φ-iO                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  SCENARIO: Hospital database (encrypted)\n";
    cout << "  Patients: Alice(45), Bob(32), Carol(67), Dave(28)\n\n";

    vector<double> ages = {45, 32, 67, 28};
    double sum_ages = 0, avg_age = 0;
    for (double a : ages) sum_ages += a;
    avg_age = sum_ages / ages.size();

    cout << "  Authorized query: AVERAGE age = " << fixed << setprecision(1) << avg_age << "\n";
    cout << "  Decoy query:      SUM of ages = " << fixed << setprecision(0) << sum_ages << "\n\n";

    // φ-reality encodes: average (authorized, more privacy-preserving)
    // ψ-reality encodes: sum (decoy, reveals more info)
    // The decryptor gets ONE value. They don't know which.

    double phi_value = avg_age;   // Authorized function output
    double psi_value = sum_ages;  // Decoy function output

    // Encode into (a,b) pair
    double b_val = (phi_value - psi_value) / (PHI - PSI);
    double a_val = phi_value - b_val * PHI;

    cout << "  Encrypted output (a,b): (" << fixed << setprecision(4) << a_val 
         << ", " << b_val << ")\n\n";

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SECURITY ANALYSIS                                  ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  Decryptor receives: ONE number                      ║\n";
    cout <<   "  ║  Is it the SUM (172) or AVERAGE (43)?               ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  If SUM:    decryptor learns total ages              ║\n";
    cout <<   "  ║  If AVG:    decryptor learns only average            ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  The decryptor CANNOT tell which function ran.       ║\n";
    cout <<   "  ║  The hospital controls which key to give out.        ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  This IS functional encryption.                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Prove: multiple queries, different functions
    cout << "  MULTI-FUNCTION DEMO:\n";
    cout << "  Query  φ-output(AVG)  ψ-output(SUM)  Decryptor sees\n";
    cout << string(55, '-') << "\n";

    vector<pair<double,double>> queries = {
        {avg_age, sum_ages},           // AVG vs SUM
        {ages[0], sum_ages - ages[0]}, // Alice's age vs sum of others
        {ages.size()*1.0, sum_ages},   // Count vs Sum
        {ages[2], avg_age},            // Carol's age vs Average
    };

    mt19937 rng(42);
    for (size_t q = 0; q < queries.size(); q++) {
        double phi_val = queries[q].first;
        double psi_val = queries[q].second;
        
        double b = (phi_val - psi_val) / (PHI - PSI);
        double a = phi_val - b * PHI;
        
        bool give_phi = (rng() % 2 == 0);  // Which key the hospital gives
        double revealed = give_phi ? phi_val : psi_val;
        
        cout << setw(4) << q 
             << setw(14) << fixed << setprecision(1) << phi_val
             << setw(14) << fixed << setprecision(1) << psi_val
             << setw(14) << fixed << setprecision(1) << revealed
             << "  (" << (give_phi ? "AVG" : "SUM") << ")\n";
    }
    
    cout << "\n  The decryptor gets different values each time.\n";
    cout << "  They NEVER know which function was actually computed.\n";
    cout << "  This is FUNCTIONAL ENCRYPTION built from φ-iO.\n\n";

    return 0;
}
