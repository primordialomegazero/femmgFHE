#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/hierarchical_seed.h"
#include "../config/system_config.h"
#include <vector>
#include <utility>
#include <string>

struct GFNEncryption {
    int N_layers;
    int base_n;
    int n_step;
    double cassini_min;
    int max_cassini_retries;
    std::string seed_branch;
    bool use_unique_branches;
    std::vector<GoldenFibonacci> gf_layers;

    enum SecurityLevel {
        STANDARD = 1,
        ELEVATED = 3,
        MILITARY = 10,
        UNIVERSE = 100
    };

    struct CipherText {
        double y1;
        std::vector<double> y2_trail;
        double mac;
    };

    GFNEncryption()
        : N_layers(1), base_n(50), n_step(7),
          cassini_min(0.1), max_cassini_retries(200),
          seed_branch("encryption"), use_unique_branches(true) {}

    void init_enterprise(double master_seed, int num_sub_seeds = 1) {
        if (num_sub_seeds < 1) num_sub_seeds = 1;
        N_layers = num_sub_seeds;
        gf_layers.resize(N_layers);

        HierarchicalSeedTree tree;
        tree.init(master_seed);

        for (int i = 0; i < N_layers; i++) {
            std::string unique_branch = seed_branch + "_" + std::to_string(i);
            tree.create_branch(unique_branch, i, (i % 2 == 0));
            double sub_seed = tree.get_seed(unique_branch, 0);
            gf_layers[i].init_with_params(sub_seed, base_n + i * n_step,
                                          cassini_min, max_cassini_retries);
        }

        Logger::info("Enterprise FNE: " + std::to_string(N_layers) +
                     " layers | deterministic mode");
    }

    CipherText encrypt(double plaintext) {
        CipherText ct;
        ct.y2_trail.resize(N_layers);
        double current = (plaintext >= 0.9999) ? 0.999 : plaintext;

        for (int i = 0; i < N_layers; i++) {
            auto [y1, y2] = gf_layers[i].encrypt(current);
            ct.y2_trail[i] = y2;
            current = y1;
        }
        ct.y1 = current;
        ct.mac = compute_mac(ct);
        return ct;
    }

    double decrypt(const CipherText& ct) {
        double current = ct.y1;
        for (int i = N_layers - 1; i >= 0; i--) {
            current = gf_layers[i].decrypt(current, ct.y2_trail[i]);
        }
        return current;
    }

    bool verify(const CipherText& ct) {
        double expected = compute_mac(ct);
        return std::abs(expected - ct.mac) < 1e-6;
    }

private:
    double compute_mac(const CipherText& ct) const {
        double sum = 0.0;
        for (double y2 : ct.y2_trail) {
            sum = SafeMath::fmod_safe(sum + y2);
        }
        return SafeMath::fmod_safe(ct.y1 * PHI + sum * PSI);
    }
};
