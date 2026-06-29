#ifndef RIEMANN_STABILIZER_H
#define RIEMANN_STABILIZER_H

#include <cmath>
#include <complex>

constexpr double PHI_ZETA      = 1.6180339887498948482;
constexpr double PHI_INV_ZETA  = 0.6180339887498948482;
constexpr double LAMBDA_ZETA   = 0.4812;
constexpr double CRITICAL_LINE = 0.5;
constexpr int    ZETA_TERMS    = 50;

namespace femmg {
namespace riemann {

class ZetaStabilizer {
private:
    static std::complex<double> zeta_series(std::complex<double> s, int terms = ZETA_TERMS) {
        std::complex<double> sum(0.0, 0.0);
        if (s.real() > 1.0) {
            for (int n = 1; n <= terms; n++) {
                double mag = std::pow(n, -s.real());
                double phase = -s.imag() * std::log(n);
                sum += std::complex<double>(mag * std::cos(phase), mag * std::sin(phase));
            }
        } else {
            std::complex<double> eta(0.0, 0.0);
            for (int n = 1; n <= terms; n++) {
                double sign = (n % 2 == 0) ? -1.0 : 1.0;
                double mag = sign * std::pow(n, -s.real());
                double phase = -s.imag() * std::log(n);
                eta += std::complex<double>(mag * std::cos(phase), mag * std::sin(phase));
            }
            double factor = 1.0 - std::pow(2.0, 1.0 - s.real());
            if (std::fabs(factor) < 0.0001) factor = 0.0001;
            sum = eta / factor;
        }
        return sum;
    }

public:
    // Riemann-enhanced noise stabilization
    // Standard: T(x) = x·φ⁻¹ + N₀·(1-φ⁻¹)
    // Riemann:  T(x) = x·φ⁻¹ + N₀·(1-φ⁻¹) + ζ(0.5+i·x·φ)·(N₀-x)·0.01
    static double stabilize(double noise, double target = 40.0) {
        double banach = noise * PHI_INV_ZETA + target * (1.0 - PHI_INV_ZETA);
        double t = noise * PHI_ZETA;
        double zeta_mag = std::abs(zeta_series(std::complex<double>(CRITICAL_LINE, t)));
        double zeta_pull = std::min(1.0, zeta_mag * 0.1);
        double riemann_boost = zeta_pull * (target - noise) * 0.01;
        return banach + riemann_boost;
    }
    
    static double deep_stabilize(double noise, int iterations = 7) {
        double result = noise;
        for (int i = 0; i < iterations; i++) result = stabilize(result);
        return result;
    }
    
    struct ZetaHealth {
        double noise, zeta_mag, banach, riemann_boost, stabilized;
    };
    
    static ZetaHealth check(double noise) {
        ZetaHealth h;
        h.noise = noise;
        double t = noise * PHI_ZETA;
        h.zeta_mag = std::abs(zeta_series(std::complex<double>(CRITICAL_LINE, t)));
        h.banach = noise * PHI_INV_ZETA + 40.0 * (1.0 - PHI_INV_ZETA);
        h.riemann_boost = std::min(1.0, h.zeta_mag * 0.1) * (40.0 - noise) * 0.01;
        h.stabilized = stabilize(noise);
        return h;
    }
};

} // namespace riemann
} // namespace femmg
#endif
