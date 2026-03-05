#pragma once
/**
 * Ensemble Runner — Statistical mechanics over independent RenderBridge runs.
 *
 * Physics justification: FTD's stochastic genesis means single runs carry
 * shot noise. Ensemble averaging is the standard technique to extract
 * expectation values from stochastic systems. Without it, no statistical
 * claim (Born rule, Bell correlations, mass spectrum) can be validated.
 *
 * Usage:
 *   EnsembleRunner er(lattice_size, num_runs);
 *   er.set_setup([](RenderBridge& rb) { rb.inject_particle(...); });
 *   er.run(num_ticks);
 *   auto stats = er.energy_stats();  // mean, stderr, min, max
 */

#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "render_bridge.h"

namespace ftd {

// Statistical moments for a scalar observable
struct Stats {
    double mean = 0.0;
    double variance = 0.0;
    double stderr_ = 0.0;  // standard error of the mean
    double skewness = 0.0;
    double kurtosis = 0.0;
    double min_val = 0.0;
    double max_val = 0.0;
    int n_samples = 0;

    static Stats from_samples(const std::vector<double>& samples) {
        Stats s;
        s.n_samples = static_cast<int>(samples.size());
        if (s.n_samples == 0) return s;

        // Mean
        s.mean = std::accumulate(samples.begin(), samples.end(), 0.0) / s.n_samples;

        // Min/Max
        s.min_val = *std::min_element(samples.begin(), samples.end());
        s.max_val = *std::max_element(samples.begin(), samples.end());

        if (s.n_samples < 2) {
            s.variance = 0.0;
            s.stderr_ = 0.0;
            return s;
        }

        // Variance (Bessel-corrected)
        double sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
        for (double x : samples) {
            double d = x - s.mean;
            sum2 += d * d;
            sum3 += d * d * d;
            sum4 += d * d * d * d;
        }
        s.variance = sum2 / (s.n_samples - 1);
        s.stderr_ = std::sqrt(s.variance / s.n_samples);

        // Skewness and kurtosis (standardized)
        double sigma = std::sqrt(s.variance);
        if (sigma > 1e-30) {
            s.skewness = (sum3 / s.n_samples) / (sigma * sigma * sigma);
            s.kurtosis = (sum4 / s.n_samples) / (sigma * sigma * sigma * sigma) - 3.0;
        }
        return s;
    }
};

// Per-run snapshot of EnergyAudit fields
struct RunResult {
    double field_energy = 0.0;
    double wave_energy = 0.0;
    double particle_ke = 0.0;
    double total_energy = 0.0;
    double gauss_violation = 0.0;
    double coulomb_pe = 0.0;
    int charge_total = 0;
    int manifested_count = 0;
    // User-defined scalar observable
    double custom_observable = 0.0;
};

class EnsembleRunner {
public:
    EnsembleRunner(int lattice_size, int num_runs)
        : lattice_size_(lattice_size), num_runs_(num_runs) {}

    // Set the initial configuration for each run.
    // Called once per run before ticking.
    void set_setup(std::function<void(RenderBridge&)> setup_fn) {
        setup_fn_ = std::move(setup_fn);
    }

    // Set toggle configuration (applied to each run).
    void set_toggles(std::function<void(TermToggles&)> toggle_fn) {
        toggle_fn_ = std::move(toggle_fn);
    }

    // Optional: set a custom observable to measure after each run.
    // Receives the RenderBridge after all ticks; returns a scalar.
    void set_observable(std::function<double(const RenderBridge&)> obs_fn) {
        obs_fn_ = std::move(obs_fn);
    }

    // Run the ensemble: each run gets a unique seed (base_seed + run_index).
    void run(int num_ticks, int base_seed = 42) {
        results_.clear();
        results_.reserve(num_runs_);

        for (int r = 0; r < num_runs_; ++r) {
            RenderBridge rb(lattice_size_);

            // Re-seed RNG for this run (each run is independent)
            rb.seed_rng(static_cast<unsigned int>(base_seed + r));

            // Apply toggles
            if (toggle_fn_) toggle_fn_(rb.toggles);

            // Apply user setup
            if (setup_fn_) setup_fn_(rb);

            // Run ticks
            rb.run(num_ticks);

            // Collect results
            RunResult rr;
            auto audit = rb.energy_audit();
            rr.field_energy = audit.field_energy;
            rr.wave_energy = audit.wave_energy;
            rr.particle_ke = audit.particle_ke;
            rr.total_energy = audit.total_energy;
            rr.gauss_violation = audit.gauss_violation;
            rr.coulomb_pe = audit.coulomb_pe;
            rr.charge_total = audit.charge_total;
            rr.manifested_count = audit.manifested_count;

            if (obs_fn_) rr.custom_observable = obs_fn_(rb);

            results_.push_back(rr);
        }
    }

    // Access raw results
    const std::vector<RunResult>& results() const { return results_; }
    int num_runs() const { return num_runs_; }

    // Statistical summaries over ensemble
    Stats energy_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(r.total_energy);
        return Stats::from_samples(vals);
    }

    Stats field_energy_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(r.field_energy);
        return Stats::from_samples(vals);
    }

    Stats gauss_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(r.gauss_violation);
        return Stats::from_samples(vals);
    }

    Stats charge_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(static_cast<double>(r.charge_total));
        return Stats::from_samples(vals);
    }

    Stats manifested_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(static_cast<double>(r.manifested_count));
        return Stats::from_samples(vals);
    }

    Stats custom_stats() const {
        std::vector<double> vals;
        for (const auto& r : results_) vals.push_back(r.custom_observable);
        return Stats::from_samples(vals);
    }

private:
    int lattice_size_;
    int num_runs_;
    std::function<void(RenderBridge&)> setup_fn_;
    std::function<void(TermToggles&)> toggle_fn_;
    std::function<double(const RenderBridge&)> obs_fn_;
    std::vector<RunResult> results_;
};

}  // namespace ftd
