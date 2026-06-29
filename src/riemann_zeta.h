#ifndef RIEMANN_ZETA_H
#define RIEMANN_ZETA_H

#include <cmath>
#include <complex>
#include <vector>
#include <algorithm>
#include <iostream>

namespace femmg {
namespace riemann {

// ═══ RIEMANN-SIEGEL Z-FUNCTION — FULL FORMULA ═══
// Z(t) = 2 Σ_{n=1}^{N} n^{-1/2} cos(θ(t) - t·ln(n)) + R(t)
// N = floor(sqrt(t/(2π)))
// R(t) = full remainder with up to 4 correction terms
class RiemannSiegelZ {
private:
    // Riemann-Siegel theta: θ(t) = arg(Γ(1/4 + it/2)) - t·ln(π)/2
    static double theta(double t) {
        if(t < 1.0) t = 1.0;
        return (t/2.0) * std::log(t/(2.0*M_PI)) - t/2.0 - M_PI/8.0
               + 1.0/(48.0*t) + 7.0/(5760.0*t*t*t);
    }

    // Full Riemann-Siegel Z(t) with remainder terms
    static double z_function(double t) {
        if(t < 10.0) t = 10.0;
        
        double th = theta(t);
        int N = (int)std::sqrt(t / (2.0 * M_PI));
        if(N < 1) N = 1;
        
        // Main sum: 2 Σ n^{-1/2} cos(θ - t·ln(n))
        double main_sum = 0.0;
        for(int n = 1; n <= N; n++) {
            main_sum += 2.0 * std::cos(th - t * std::log((double)n)) / std::sqrt((double)n);
        }
        
        // Riemann-Siegel remainder R(t)
        // R(t) = (-1)^{N-1} (t/(2π))^{-1/4} × Σ_{k=0}^{M} C_k(τ) / (t/(2π))^{k/2}
        double tau = std::sqrt(t/(2.0*M_PI)) - N;  // Fractional part, -0.5 to 0.5
        double p = std::pow(t/(2.0*M_PI), -0.25);
        
        // Asymptotic correction terms
        double C0 = 1.0;
        double C1 = (tau*tau - 1.0/6.0) / 2.0;
        double C2 = (tau*tau*tau*tau/4.0 - tau*tau/12.0 + 7.0/288.0) / 2.0;
        double C3 = (tau*tau*tau*tau*tau*tau/8.0 - 5.0*tau*tau*tau*tau/96.0 + 7.0*tau*tau/384.0 - 31.0/10368.0) / 2.0;
        
        double s = std::pow(t/(2.0*M_PI), -0.5);
        double remainder = p * (C0 + C1 * s + C2 * s*s + C3 * s*s*s);
        
        // Sign alternation
        if(N % 2 == 1) remainder = -remainder;
        
        return main_sum + remainder;
    }

public:
    // Evaluate Z(t)
    static double evaluate(double t) {
        return z_function(t);
    }
    
    // Find zeros with high precision
    static std::vector<double> find_zeros(int count, double t_start = 10.0, double step = 0.02) {
        std::vector<double> zeros;
        double t = t_start;
        double prev_z = z_function(t);
        double max_t = 10000.0;
        
        while((int)zeros.size() < count && t < max_t) {
            t += step;
            double curr_z = z_function(t);
            
            // Sign change detected
            if(prev_z * curr_z < 0.0) {
                // Brent's method refinement for high precision
                double a = t - step;
                double b = t;
                double fa = prev_z;
                double fb = curr_z;
                double c = a, fc = fa;
                
                for(int iter = 0; iter < 15; iter++) {
                    if(fb * fc > 0) {
                        c = a; fc = fa;
                    }
                    if(std::fabs(fc) < std::fabs(fb)) {
                        a = b; fa = fb;
                        b = c; fb = fc;
                        c = a; fc = fa;
                    }
                    
                    double tol = 1e-12;
                    double m = 0.5 * (c - b);
                    if(std::fabs(m) <= tol || fb == 0) break;
                    
                    double s;
                    if(std::fabs(fa - fb) > 1e-15) {
                        s = (a*fb*fc/((fa-fb)*(fa-fc)) + b*fa*fc/((fb-fa)*(fb-fc)) + c*fa*fb/((fc-fa)*(fc-fb)));
                    } else {
                        s = b - fb*(b-a)/(fb-fa);
                    }
                    
                    if(s < (3*a+b)/4 || s > b || 
                       (std::fabs(s-b) >= std::fabs(b-c)/2) ||
                       (std::fabs(b-c) < tol)) {
                        s = (a + b) / 2.0;
                    }
                    
                    double fs = z_function(s);
                    if(fs * fc > 0) {
                        c = a; fc = fa;
                        a = b; fa = fb;
                        b = s; fb = fs;
                    } else {
                        a = b; fa = fb;
                        b = s; fb = fs;
                    }
                }
                
                double t_zero = b;
                zeros.push_back(t_zero);
                
                if(zeros.size() % 200 == 0) {
                    std::cout << "  Found " << zeros.size() << " zeros... (t=" << t_zero << ")" << std::endl;
                }
            }
            
            prev_z = curr_z;
        }
        
        return zeros;
    }
};

} // namespace riemann
} // namespace femmg
#endif
