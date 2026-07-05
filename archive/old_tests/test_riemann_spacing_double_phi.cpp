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
    cout << "║  φ-SPACING: Which φ-variant Fits Best?                   ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    const double PHI = 1.6180339887498948482;
    
    // Candidates for gap ratio clustering centers
    struct Candidate {
        string name;
        double value;
        double tolerance;
    };
    
    Candidate cands[] = {
        {"φ⁻¹ = 0.618", 1.0/PHI, 0.2},
        {"1/2 = 0.5", 0.5, 0.2},
        {"φ/2 = 0.809", PHI/2.0, 0.2},
        {"φ⁻² = 0.382", 1.0/(PHI*PHI), 0.2},
        {"1 (uniform)", 1.0, 0.2},
        {"φ = 1.618", PHI, 0.3},
        {"φ² = 2.618", PHI*PHI, 0.5},
        {"2φ = 3.236", 2.0*PHI, 0.5},
        {"φ³ = 4.236", PHI*PHI*PHI, 0.8},
        {"3 (integer)", 3.0, 0.5}
    };
    int n_cands = 10;

    // Extract all gap ratios
    vector<double> ratios;
    for (int i = 0; i < 198; i++) {
        double g1 = Z[i+1] - Z[i];
        double g2 = Z[i+2] - Z[i+1];
        if (g1 > 0.01) ratios.push_back(g2/g1);
    }

    cout << "\n--- Gap Ratio Clustering Around φ-Variants ---" << endl;
    cout << "  Candidate         | Value  | Within tol | Rate" << endl;
    cout << "  ------------------|--------|------------|-----" << endl;

    string best_name = "";
    double best_rate = 0;

    for (int c = 0; c < n_cands; c++) {
        int count = 0;
        for (auto r : ratios) {
            if (abs(r - cands[c].value) < cands[c].tolerance) count++;
        }
        double rate = 100.0 * count / ratios.size();
        
        cout << "  " << setw(18) << left << cands[c].name << " | " 
             << fixed << setprecision(3) << setw(6) << cands[c].value << " | "
             << setw(5) << count << "/" << setw(4) << ratios.size() << " | "
             << setprecision(1) << rate << "%";
        
        if (rate > best_rate && c > 0) { best_rate = rate; best_name = cands[c].name; }
        
        if (c == 0) cout << " ← current";
        else if (rate > 30) cout << " ★";
        cout << endl;
    }

    // ═══ BIMODAL TEST: Check pairs of candidates ═══
    cout << "\n\n--- Bimodal (Pair) Clustering ---" << endl;
    cout << "  Pair                      | Combined Rate" << endl;
    cout << "  --------------------------|--------------" << endl;

    // Test φ⁻¹ + φ (current)
    {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - 1.0/PHI) < 0.2) c1++;
            if (abs(r - PHI) < 0.3) c2++;
        }
        cout << "  φ⁻¹ + φ                   | " << (100.0*(c1+c2)/ratios.size()) << "% ← current" << endl;
    }
    // Test φ⁻² + φ²
    {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - 1.0/(PHI*PHI)) < 0.2) c1++;
            if (abs(r - PHI*PHI) < 0.5) c2++;
        }
        cout << "  φ⁻² + φ²                  | " << (100.0*(c1+c2)/ratios.size()) << "%" << endl;
    }
    // Test φ/2 + 2φ
    {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - PHI/2.0) < 0.2) c1++;
            if (abs(r - 2.0*PHI) < 0.5) c2++;
        }
        cout << "  φ/2 + 2φ                  | " << (100.0*(c1+c2)/ratios.size()) << "%" << endl;
    }
    // Test 1/2 + φ
    {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - 0.5) < 0.2) c1++;
            if (abs(r - PHI) < 0.3) c2++;
        }
        cout << "  1/2 + φ                   | " << (100.0*(c1+c2)/ratios.size()) << "%" << endl;
    }
    // Test 1 + φ²
    {
        int c1 = 0, c2 = 0;
        for (auto r : ratios) {
            if (abs(r - 1.0) < 0.2) c1++;
            if (abs(r - PHI*PHI) < 0.5) c2++;
        }
        cout << "  1 + φ²                    | " << (100.0*(c1+c2)/ratios.size()) << "%" << endl;
    }

    // ═══ HISTOGRAM: Show distribution peaks ═══
    cout << "\n\n--- Gap Ratio Histogram (Top Values) ---" << endl;
    cout << "  Ratio Range | Count | Distribution" << endl;
    cout << "  ------------|-------|------------" << endl;

    for (double center = 0.2; center <= 4.0; center += 0.2) {
        int count = 0;
        for (auto r : ratios) {
            if (r >= center - 0.1 && r < center + 0.1) count++;
        }
        if (count > 0) {
            cout << "  " << fixed << setprecision(1) << setw(4) << center << " ±0.1  | " 
                 << setw(3) << count << "   | ";
            for (int i = 0; i < count; i++) cout << "█";
            // Mark φ-related positions
            if (abs(center - 1.0/PHI) < 0.11) cout << " ← φ⁻¹";
            if (abs(center - PHI/2.0) < 0.11) cout << " ← φ/2";
            if (abs(center - 1.0) < 0.11) cout << " ← 1";
            if (abs(center - PHI) < 0.11) cout << " ← φ";
            if (abs(center - (PHI*PHI)) < 0.11) cout << " ← φ²";
            if (abs(center - 2.0*PHI) < 0.11) cout << " ← 2φ";
            cout << endl;
        }
    }

    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║  BEST FIT: " << setw(46) << left << best_name << " ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;

    return 0;
}
