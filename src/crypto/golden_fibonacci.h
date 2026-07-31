#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include <utility>
#include <cmath>

struct GoldenFibonacci {
    int power_n;
    double G_n, G_n1, G_n_minus_1, cassini, secret_seed;
    
    // Standard init with defaults
    void init(double master_seed, int n_val = 50) {
        init_with_params(master_seed, n_val, 0.1, 200);
    }
    
    // Configurable init — NO HARDCODING
    void init_with_params(double master_seed, int n_val, 
                          double min_cassini, int max_retries) {
        secret_seed = SafeMath::fmod_safe(std::abs(master_seed) * PHI);
        power_n = (n_val < 50) ? 50 : n_val;
        int original_n = power_n;
        int retries = max_retries;
        
        while (retries > 0) {
            long double a = 0.0L, b = PHI;
            for (int i = 1; i < power_n; i++) {
                long double t = std::fmod((a + b) * PHI, 1.0L);
                a = b; b = t;
            }
            G_n_minus_1 = (double)a; G_n = (double)b;
            G_n1 = SafeMath::fmod_safe((a + b) * PHI);
            cassini = SafeMath::fmod_safe(std::abs(G_n_minus_1 * G_n1 - G_n * G_n));
            
            if (cassini > min_cassini) break;
            power_n += 1;
            retries--;
        }
        
        if (cassini < 0.001) {
            cassini = 0.001;
            Logger::warn("Cassini clamped: n=" + std::to_string(power_n) + 
                        " (from " + std::to_string(original_n) + ")");
        }
    }
    
    std::pair<double, double> encrypt(double plaintext) {
        double x = (plaintext >= 0.9999) ? 0.999 : plaintext;
        double s = secret_seed;
        return {SafeMath::fmod_safe(G_n1 * x + G_n * s),
                SafeMath::fmod_safe(G_n * x + G_n_minus_1 * s)};
    }
    
    double decrypt_raw(double y1, double y2) {
        double num = G_n_minus_1 * y1 - G_n * y2;
        double raw = SafeMath::div_safe(num, cassini);
        return SafeMath::fmod_safe(raw);
    }
    
    double decrypt(double y1, double y2) {
        double x = decrypt_raw(y1, y2);
        double nearest = std::round(x * 4.0) / 4.0;
        if (nearest == 0.0 && std::abs(x - 1.0) < std::abs(x - 0.0)) nearest = 1.0;
        return nearest;
    }
};
