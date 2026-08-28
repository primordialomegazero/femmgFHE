// ============================================
// φ-EMERGENT BOUNCE
// Natural na bounce na hindi pumipili ng 0 o 1
//
// Core philosophy:
// - Hindi natin pinipili ang 0 o 1
// - Ang φ-space ang natural na nagde-decide
// - Subukan ang mga walang kinalaman
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-EMERGENT BOUNCE\n";
    cout << "  Natural na Hindi Pumipili\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double INV_PHI = 1.0 / PHI;

    cout << fixed << setprecision(15);

    // ========== EXPERIMENT 1: φ-SAWTOOTH ==========
    cout << "EXPERIMENT 1: φ-SAWTOOTH\n";
    cout << "========================\n\n";
    cout << "  x → x - φ×floor(x/φ)\n";
    cout << "  Natural na bounce sa [0, φ]\n\n";
    
    double saw = 100.0;
    vector<double> saw_evolution;
    saw_evolution.push_back(saw);
    
    for (int i = 0; i < 20; i++) {
        saw = saw - PHI * floor(saw / PHI);
        saw_evolution.push_back(saw);
    }
    
    cout << "  Start: 100.0\n";
    cout << "  After 1 bounce: " << saw_evolution[1] << "\n";
    cout << "  Final: " << saw_evolution.back() << "\n";
    cout << "  Bounded sa [0, φ]: " << (saw_evolution.back() >= 0 && saw_evolution.back() < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 2: φ-TRIANGLE ==========
    cout << "EXPERIMENT 2: φ-TRIANGLE\n";
    cout << "========================\n\n";
    cout << "  x → |x - φ×round(x/φ)|\n";
    cout << "  Natural na fold\n\n";
    
    double tri = 100.0;
    vector<double> tri_evolution;
    tri_evolution.push_back(tri);
    
    for (int i = 0; i < 20; i++) {
        tri = abs(tri - PHI * round(tri / PHI));
        tri_evolution.push_back(tri);
    }
    
    cout << "  Start: 100.0\n";
    cout << "  After 1 fold: " << tri_evolution[1] << "\n";
    cout << "  Final: " << tri_evolution.back() << "\n";
    cout << "  Bounded sa [0, φ/2]: " << (tri_evolution.back() >= 0 && tri_evolution.back() < PHI/2 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 3: φ-WAVE ==========
    cout << "EXPERIMENT 3: φ-WAVE\n";
    cout << "====================\n\n";
    cout << "  x → φ×sin(x/φ)\n";
    cout << "  Natural na oscillation\n\n";
    
    double wave = 10.0;
    vector<double> wave_evolution;
    wave_evolution.push_back(wave);
    
    for (int i = 0; i < 20; i++) {
        wave = PHI * sin(wave / PHI);
        wave_evolution.push_back(wave);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < wave_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << wave_evolution[i] << "\n";
    }
    cout << "  Final: " << wave_evolution.back() << "\n";
    cout << "  Bounded sa [-φ, φ]: " << (abs(wave_evolution.back()) < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 4: φ-FOLD ==========
    cout << "EXPERIMENT 4: φ-FOLD\n";
    cout << "====================\n\n";
    cout << "  x → φ - |x - φ|\n";
    cout << "  Natural na mirror\n\n";
    
    double fold = 100.0;
    vector<double> fold_evolution;
    fold_evolution.push_back(fold);
    
    for (int i = 0; i < 20; i++) {
        fold = PHI - abs(fold - PHI);
        fold_evolution.push_back(fold);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < fold_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << fold_evolution[i] << "\n";
    }
    cout << "  Final: " << fold_evolution.back() << "\n";
    cout << "  Bounded: " << (abs(fold_evolution.back()) < 100 ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 5: φ-MIRROR-FOLD ==========
    cout << "EXPERIMENT 5: φ-MIRROR-FOLD\n";
    cout << "===========================\n\n";
    cout << "  x → φ - x kung x > φ/2, x kung x <= φ/2\n";
    cout << "  Natural na mirror sa φ/2\n\n";
    
    double mirror = 100.0;
    vector<double> mirror_evolution;
    mirror_evolution.push_back(mirror);
    
    for (int i = 0; i < 20; i++) {
        if (mirror > PHI / 2) {
            mirror = PHI - mirror;
        }
        mirror_evolution.push_back(mirror);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < mirror_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << mirror_evolution[i] << "\n";
    }
    cout << "  Final: " << mirror_evolution.back() << "\n";
    cout << "  Bounded: " << (abs(mirror_evolution.back()) < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 6: φ-CASCADE ==========
    cout << "EXPERIMENT 6: φ-CASCADE\n";
    cout << "=======================\n\n";
    cout << "  x → φ×frac(x/φ)\n";
    cout << "  Natural na cascade\n\n";
    
    double cascade = 100.0;
    vector<double> cascade_evolution;
    cascade_evolution.push_back(cascade);
    
    for (int i = 0; i < 20; i++) {
        double frac_part = cascade / PHI - floor(cascade / PHI);
        cascade = PHI * frac_part;
        cascade_evolution.push_back(cascade);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < cascade_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << cascade_evolution[i] << "\n";
    }
    cout << "  Final: " << cascade_evolution.back() << "\n";
    cout << "  Bounded sa [0, φ]: " << (cascade_evolution.back() >= 0 && cascade_evolution.back() < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    // ========== EXPERIMENT 7: φ-BOUNCE COMBINATION ==========
    cout << "EXPERIMENT 7: φ-BOUNCE COMBINATION\n";
    cout << "==================================\n\n";
    cout << "  Combined bounce: sawtooth + triangle + wave\n\n";
    
    double bounce = 100.0;
    vector<double> bounce_evolution;
    bounce_evolution.push_back(bounce);
    
    for (int i = 0; i < 20; i++) {
        // I-combine ang lahat ng bounce methods
        bounce = bounce - PHI * floor(bounce / PHI);  // sawtooth
        bounce = abs(bounce - PHI * round(bounce / PHI));  // triangle
        bounce = PHI * sin(bounce / PHI);  // wave
        bounce_evolution.push_back(bounce);
    }
    
    cout << "  Evolution:\n";
    for (size_t i = 0; i < bounce_evolution.size(); i += 2) {
        cout << "    Step " << i << ": " << bounce_evolution[i] << "\n";
    }
    cout << "  Final: " << bounce_evolution.back() << "\n";
    cout << "  Bounded sa [0, φ]: " << (bounce_evolution.back() >= 0 && bounce_evolution.back() < PHI ? "YES ✓" : "NO ✗") << "\n\n";

    return 0;
}
