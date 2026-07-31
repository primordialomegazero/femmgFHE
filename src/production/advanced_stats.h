#pragma once
#include "../utils/logger.h"
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>

struct AdvancedStatisticalValidator {
    int total_samples;
    int completed_samples;
    double ks_threshold;

    std::vector<double> dist_A, dist_B;

    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_report_time;
    double last_ks_snapshot;
    int report_interval;

    double progressive_ks;
    std::vector<double> dist_A_snapshot, dist_B_snapshot;
    int snapshot_interval;

    void init(int samples = 100, double threshold = 0.05, int report_every = 10) {
        total_samples = samples;
        ks_threshold = threshold;
        completed_samples = 0;
        progressive_ks = 1.0;
        report_interval = report_every;
        snapshot_interval = 10;

        dist_A.reserve(samples);
        dist_B.reserve(samples);
        dist_A_snapshot.reserve(samples);
        dist_B_snapshot.reserve(samples);

        start_time = std::chrono::steady_clock::now();
        last_report_time = start_time;

        Logger::info("");
        Logger::info("  ╔══════════════════════════════════════════════╗");
        Logger::info("  ║  ADVANCED STATISTICAL iO VALIDATION         ║");
        Logger::info("  ║  Real-time progress + live KS estimation    ║");
        Logger::info("  ╚══════════════════════════════════════════════╝");
        Logger::info("");
        Logger::info("  Progress | Samples | KS Est  | Rate      | ETA       | Status");
        Logger::info("  ---------|---------|---------|-----------|-----------|-------");
    }

    void add_sample(double value_A, double value_B) {
        dist_A.push_back(value_A);
        dist_B.push_back(value_B);
        completed_samples++;

        dist_A_snapshot.push_back(value_A);
        dist_B_snapshot.push_back(value_B);

        if (completed_samples % snapshot_interval == 0) {
            progressive_ks = compute_ks(dist_A_snapshot, dist_B_snapshot);
        }

        if (completed_samples % report_interval == 0 || completed_samples == total_samples) {
            report_progress();
        }
    }

    void report_progress() {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        double since_last = std::chrono::duration<double>(now - last_report_time).count();
        last_report_time = now;

        double progress_pct = (double)completed_samples / total_samples * 100.0;
        double rate = (since_last > 0) ? (report_interval / since_last) : 0;

        double remaining_samples = total_samples - completed_samples;
        double eta_seconds = (rate > 0) ? (remaining_samples / rate) : 0;

        std::string status;
        if (progressive_ks < 0.01) status = "EXCELLENT";
        else if (progressive_ks < ks_threshold) status = "PASSING";
        else if (completed_samples < total_samples * 0.5) status = "COLLECTING";
        else status = "BORDERLINE";

        int bar_width = 20;
        int filled = (int)(progress_pct / 100.0 * bar_width);
        std::string bar = "[";
        for (int i = 0; i < bar_width; i++) bar += (i < filled) ? "=" : (i == filled) ? ">" : " ";
        bar += "]";

        std::stringstream ss;
        ss << "  " << bar << " | "
           << std::setw(5) << completed_samples << "/" << total_samples << " | "
           << std::setw(7) << std::fixed << std::setprecision(4) << progressive_ks << " | "
           << std::setw(7) << std::fixed << std::setprecision(1) << rate << "/s | "
           << std::setw(7) << (int)eta_seconds << "s | "
           << status;

        Logger::info(ss.str());
    }

    double compute_ks(const std::vector<double>& A, const std::vector<double>& B) {
        if (A.empty() || B.empty()) return 1.0;

        std::vector<double> sorted_A = A;
        std::vector<double> sorted_B = B;
        std::sort(sorted_A.begin(), sorted_A.end());
        std::sort(sorted_B.begin(), sorted_B.end());

        double max_diff = 0;
        size_t i = 0, j = 0;

        while (i < sorted_A.size() && j < sorted_B.size()) {
            double cdf_A = (double)i / sorted_A.size();
            double cdf_B = (double)j / sorted_B.size();
            double diff = std::abs(cdf_A - cdf_B);
            max_diff = std::max(max_diff, diff);

            // FIXED: Handle equal values correctly
            if (sorted_A[i] < sorted_B[j]) {
                i++;
            } else if (sorted_B[j] < sorted_A[i]) {
                j++;
            } else {
                // EQUAL VALUES — advance both pointers!
                i++;
                j++;
            }
        }

        return max_diff;
    }

    std::string final_verdict() {
        double ks = compute_ks(dist_A, dist_B);

        Logger::info("");
        Logger::info("  ╔══════════════════════════════════════════════╗");
        Logger::info("  ║  FINAL STATISTICAL REPORT                    ║");
        Logger::info("  ╠══════════════════════════════════════════════╣");

        std::stringstream ss;
        ss << "  ║  Samples:     " << std::setw(4) << completed_samples << "/" << total_samples;
        Logger::info(ss.str());

        ss.str(""); ss << "  ║  KS Statistic: " << std::fixed << std::setprecision(6) << ks;
        Logger::info(ss.str());

        ss.str(""); ss << "  ║  Threshold:   " << std::fixed << std::setprecision(4) << ks_threshold;
        Logger::info(ss.str());

        std::string verdict;
        if (ks < 0.01) {
            verdict = "iO-SECURE (Excellent)";
        } else if (ks < ks_threshold) {
            verdict = "iO-SECURE (Pass)";
        } else if (ks < 0.1) {
            verdict = "NEEDS MORE SAMPLES";
        } else {
            verdict = "DISTINGUISHABLE (Fail)";
        }

        ss.str(""); ss << "  ║  Verdict:     " << verdict;
        Logger::info(ss.str());
        Logger::info("  ╚══════════════════════════════════════════════╝");
        Logger::info("");

        return verdict;
    }

    bool passed() {
        return compute_ks(dist_A, dist_B) < ks_threshold;
    }
};
