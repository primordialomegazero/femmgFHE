#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

constexpr double Z[200] = {
    14.134725, 21.022040, 25.010857, 30.424876, 32.935061,
    37.586178, 40.918719, 43.327073, 48.005150, 49.773832,
    52.970321, 56.446248, 59.347044, 60.831779, 65.112544,
    67.079811, 69.546402, 72.067158, 75.704691, 77.144840,
    79.337375, 82.910381, 84.735493, 87.425275, 88.809111,
    92.491899, 94.651344, 95.870634, 98.831194, 101.317851,
    103.725538, 105.446623, 107.168611, 111.029536, 111.874659,
    114.320221, 116.226680, 118.015783, 121.370125, 122.946829,
    124.256819, 127.516684, 129.578704, 131.087689, 133.497737,
    134.756510, 138.116042, 139.736209, 141.123707, 143.111846,
    146.000982, 147.422765, 150.053520, 150.925258, 153.024694,
    156.112909, 157.597592, 158.849988, 161.188964, 163.030710,
    165.537069, 167.184440, 169.094515, 169.911976, 173.411537,
    174.754192, 176.441434, 178.377408, 179.916484, 182.207078,
    184.874468, 185.598784, 187.228923, 189.416159, 192.026656,
    193.079727, 195.265397, 196.876482, 198.015310, 201.264752,
    202.493595, 204.189672, 205.394697, 207.906259, 209.576510,
    211.690863, 213.347919, 214.547045, 216.169539, 219.067595,
    220.714919, 221.430706, 224.007000, 224.983325, 227.421444,
    229.337413, 231.250189, 231.987235, 233.693404, 236.524230,
    237.769820, 239.555431, 241.049326, 242.823035, 244.070898,
    247.136990, 248.101990, 249.573452, 251.014948, 253.069986,
    255.306157, 256.368655, 258.868442, 260.002854, 261.345499,
    263.599409, 265.557033, 266.614033, 268.313572, 270.880080,
    273.277849, 274.456108, 275.587212, 277.257284, 278.761221,
    280.802430, 282.455402, 283.211186, 284.787143, 287.226438,
    288.876346, 290.144512, 291.686628, 293.557821, 295.573255,
    297.077105, 298.584796, 299.819463, 301.651642, 303.234948,
    304.891502, 305.988876, 307.219632, 309.971110, 311.132143,
    313.332887, 314.504055, 315.569870, 317.356470, 318.870244,
    320.589288, 321.994879, 323.466400, 324.861537, 326.669058,
    328.063390, 329.260580, 331.270118, 333.640969, 334.842951,
    336.232924, 337.571245, 339.270593, 340.649295, 341.901292,
    344.144393, 345.340989, 346.979439, 348.681673, 349.882024,
    351.826990, 353.483443, 354.535100, 356.134545, 357.998742,
    359.832810, 361.113272, 362.623152, 364.882851, 366.394281
};

int main() {
    cout << "╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  TRIPLE φ & FLAME EMPRESS FREQUENCY TEST                 ║" << endl;
    cout << "║  3φ = 4.854  |  φ³ = 4.236  |  2φ+1 = 4.236             ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    const double PHI = 1.6180339887498948482;
    
    // ALL φ-variants — including triple and Flame Empress frequencies
    struct Freq {
        string name;
        double value;
        string meaning;
    };
    
    Freq freqs[] = {
        {"φ⁻³", 1.0/(PHI*PHI*PHI), "Triple inverse"},
        {"φ⁻²", 1.0/(PHI*PHI), "Double inverse"},
        {"φ⁻¹", 1.0/PHI, "Inverse (Flame Empress)"},
        {"1/2", 0.5, "Half"},
        {"φ/2", PHI/2.0, "Half φ ★"},
        {"1", 1.0, "Unity"},
        {"φ", PHI, "Golden (Source)"},
        {"φ²", PHI*PHI, "Double φ"},
        {"2φ", 2.0*PHI, "2× φ"},
        {"√5", sqrt(5.0), "φ+φ⁻¹ (Sum)"},
        {"φ³", PHI*PHI*PHI, "Triple φ"},
        {"3φ", 3.0*PHI, "3× φ"},
        {"φ²+φ", PHI*PHI+PHI, "Double+Single"},
        {"2φ+1", 2.0*PHI+1.0, "Flame Empress × 2 + Source"}
    };
    int nf = 14;

    // Extract gap ratios
    vector<double> ratios;
    for (int i = 0; i < 198; i++) {
        double g1 = Z[i+1] - Z[i], g2 = Z[i+2] - Z[i+1];
        if (g1 > 0.01) ratios.push_back(g2/g1);
    }

    // ═══ FULL HISTOGRAM with ALL φ-markers ═══
    cout << "\n--- Gap Ratio Distribution (Full Range) ---" << endl;
    cout << "  Range   | Count | Distribution" << endl;
    cout << "  --------|-------|------------" << endl;

    int max_count = 0;
    int counts[50] = {0};
    for (double center = 0.2; center < 5.0; center += 0.1) {
        int idx = (int)(center*10) - 2;
        for (auto r : ratios)
            if (r >= center-0.05 && r < center+0.05) counts[idx]++;
        if (counts[idx] > max_count) max_count = counts[idx];
    }

    for (double center = 0.2; center < 5.0; center += 0.1) {
        int idx = (int)(center*10) - 2;
        if (counts[idx] > 0) {
            cout << "  " << fixed << setprecision(1) << setw(5) << center << " | "
                 << setw(4) << counts[idx] << "  | ";
            int bar = counts[idx] * 40 / max_count;
            for (int i = 0; i < bar; i++) cout << "█";
            
            // Mark ALL φ-related positions
            for (int f = 0; f < nf; f++) {
                if (abs(center - freqs[f].value) < 0.06)
                    cout << " ← " << freqs[f].name << " (" << freqs[f].meaning << ")";
            }
            cout << endl;
        }
    }

    // ═══ CLUSTERING TEST: All φ-variants ═══
    cout << "\n\n--- Clustering Around Each φ-Variant ---" << endl;
    cout << "  Frequency        | Value  | Within tol | Rate  | Strength" << endl;
    cout << "  -----------------|--------|------------|-------|---------" << endl;

    struct Result { string name; double rate; double value; };
    vector<Result> results;

    for (int f = 0; f < nf; f++) {
        double tol = 0.15;  // Tighter tolerance for precision
        int count = 0;
        for (auto r : ratios)
            if (abs(r - freqs[f].value) < tol) count++;
        double rate = 100.0 * count / ratios.size();
        results.push_back({freqs[f].name, rate, freqs[f].value});
    }

    // Sort by rate
    sort(results.begin(), results.end(), [](auto& a, auto& b) { return a.rate > b.rate; });

    for (auto& r : results) {
        cout << "  " << setw(17) << left << r.name << " | " << fixed << setprecision(3) << setw(6) << r.value
             << " | " << setw(5) << (int)(r.rate * ratios.size() / 100) << "/" << setw(4) << ratios.size()
             << " | " << setprecision(1) << setw(4) << r.rate << "% | ";
        int stars = (int)(r.rate / 5);
        for (int i = 0; i < stars; i++) cout << "★";
        cout << endl;
    }

    // ═══ BIMODAL ANALYSIS: Find best pair ═══
    cout << "\n\n--- Best Bimodal Pairs ---" << endl;
    cout << "  Pair                    | Combined Rate" << endl;
    cout << "  ------------------------|--------------" << endl;

    // Test meaningful pairs
    struct Pair { string a, b, label; double va, vb, ta, tb; };
    Pair pairs[] = {
        {"φ⁻¹", "φ", "Source+FlameEmpress", 1.0/PHI, PHI, 0.2, 0.3},
        {"φ/2", "2φ", "Half+Double φ", PHI/2.0, 2.0*PHI, 0.2, 0.5},
        {"φ⁻¹", "φ²", "Inverse+Double", 1.0/PHI, PHI*PHI, 0.2, 0.5},
        {"φ/2", "φ", "Half+Golden", PHI/2.0, PHI, 0.2, 0.3},
        {"1/2", "2φ", "Half+Double", 0.5, 2.0*PHI, 0.2, 0.5},
        {"φ⁻¹", "3φ", "Inverse+Triple", 1.0/PHI, 3.0*PHI, 0.2, 0.8},
        {"φ", "3φ", "Source+Triple", PHI, 3.0*PHI, 0.3, 0.8},
        {"φ/2", "3φ", "Half+Triple φ", PHI/2.0, 3.0*PHI, 0.2, 0.8},
        {"φ⁻¹", "√5", "FlameEmpress+Sum", 1.0/PHI, sqrt(5.0), 0.2, 0.5},
        {"φ", "2φ+1", "Source+FlameEmpress×2+Source", PHI, 2.0*PHI+1.0, 0.3, 0.8}
    };
    int np = 10;

    struct PairResult { string label; double rate; };
    vector<PairResult> prs;

    for (int p = 0; p < np; p++) {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - pairs[p].va) < pairs[p].ta) c1++;
            if (abs(r - pairs[p].vb) < pairs[p].tb) c2++;
        }
        double rate = 100.0 * (c1 + c2) / ratios.size();
        prs.push_back({pairs[p].label, rate});
    }

    sort(prs.begin(), prs.end(), [](auto& a, auto& b) { return a.rate > b.rate; });

    for (auto& pr : prs) {
        cout << "  " << setw(24) << left << pr.label << " | " << fixed << setprecision(1) << pr.rate << "%";
        if (pr.rate > 45) cout << " ★★★";
        else if (pr.rate > 40) cout << " ★★";
        else if (pr.rate > 35) cout << " ★";
        cout << endl;
    }

    // ═══ VERDICT ═══
    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  FREQUENCY VERDICT                                       ║" << endl;
    if (!results.empty()) {
        cout << "║  #1 Strongest: " << setw(12) << left << results[0].name 
             << " at " << fixed << setprecision(1) << results[0].rate << "%";
        for (int i = 41 - results[0].name.length() - 6; i > 0; i--) cout << " ";
        cout << "║" << endl;
        cout << "║  #2:           " << setw(12) << left << results[1].name 
             << " at " << results[1].rate << "%";
        for (int i = 41 - results[1].name.length() - 6; i > 0; i--) cout << " ";
        cout << "║" << endl;
        cout << "║  #3:           " << setw(12) << left << results[2].name 
             << " at " << results[2].rate << "%";
        for (int i = 41 - results[2].name.length() - 6; i > 0; i--) cout << " ";
        cout << "║" << endl;
    }
    if (!prs.empty()) {
        cout << "║                                                          ║" << endl;
        cout << "║  Best Pair: " << setw(44) << left << prs[0].label << "║" << endl;
        cout << "║  Rate: " << fixed << setprecision(1) << prs[0].rate << "%";
        for (int i = 44 - 8; i > 0; i--) cout << " ";
        cout << "║" << endl;
    }
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    return 0;
}
