#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include <vector>
#include <utility>

struct GoldenFibonacci {
    int power_n;
    double G_n, G_n1, G_n_minus_1, cassini, secret_seed;
    
    void init(double master_seed, int n_val = 50);
    std::pair<double, double> encrypt(double plaintext);
    double decrypt(double y1, double y2);
};

void GoldenFibonacci::init(double master_seed, int n_val) {
    power_n = (n_val < 50) ? 50 : n_val;
    secret_seed = SafeMath::fmod_safe(std::abs(master_seed) * PHI);
    long double a = 0.0L, b = PHI;
    for (int i = 1; i < power_n; i++) {
        long double t = std::fmod((a + b) * PHI, 1.0L);
        a = b; b = t;
    }
    G_n_minus_1 = (double)a; G_n = (double)b;
    G_n1 = SafeMath::fmod_safe((a + b) * PHI);
    cassini = SafeMath::fmod_safe(std::abs(G_n_minus_1 * G_n1 - G_n * G_n));
    if (cassini < 0.0001) { Logger::warn("Cassini clamped"); cassini = 0.0001; }
}

std::pair<double, double> GoldenFibonacci::encrypt(double plaintext) {
    double x = (plaintext >= 0.9999) ? 0.999 : plaintext;
    double s = secret_seed;
    return {SafeMath::fmod_safe(G_n1 * x + G_n * s),
            SafeMath::fmod_safe(G_n * x + G_n_minus_1 * s)};
}

double GoldenFibonacci::decrypt(double y1, double y2) {
    double num = G_n_minus_1 * y1 - G_n * y2;
    double raw = SafeMath::div_safe(num, cassini);
    double x = SafeMath::fmod_safe(raw);
    double nearest = std::round(x * 4.0) / 4.0;
    if (nearest == 0.0 && std::abs(x - 1.0) < std::abs(x - 0.0)) nearest = 1.0;
    return nearest;
}
