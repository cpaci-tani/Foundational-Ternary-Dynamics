/**
 * CosmicEngine scenario builders.
 *
 * Extracted from cosmic_engine.cpp (CE1). Contains the six setup_* functions:
 * spiral_galaxy, galaxy_cluster, cosmic_web, black_hole_closeup,
 * galaxy_merger, quasar. See cosmic_engine.h for declarations.
 *
 * Note: setup_quasar calls setup_black_hole_closeup — both live in this TU,
 * so no cross-TU include order concerns.
 */

#include "ftd/cosmic_engine.h"
#include <cmath>
#include <random>

namespace ftd {

// ============================================================================
// Scenario builders
// ============================================================================

void CosmicEngine::setup_spiral_galaxy(int n_dm, int n_gas, int n_stars,
                                       double total_mass, double disk_radius) {
    clear();
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::normal_distribution<double> normal(0.0, 1.0);

    double dm_mass = total_mass * cosmic::DM_FRACTION / n_dm;
    double gas_mass = total_mass * cosmic::BARYON_FRACTION * 0.6 / n_gas;
    double star_mass = total_mass * cosmic::BARYON_FRACTION * 0.4 / n_stars;
    double halo_radius = disk_radius * 5.0;

    // Central supermassive black hole (~0.1% of total mass)
    add_black_hole(total_mass * 0.001, {0, 0, 0});

    // Dark matter halo (NFW-like profile: r distributed as r^2 * (r/r_s)^-1 * (1+r/r_s)^-2)
    double r_s = halo_radius / 5.0; // scale radius
    for (int i = 0; i < n_dm; ++i) {
        // Rejection sampling for NFW profile
        double r, theta, phi;
        do {
            r = unit(rng) * halo_radius;
        } while (unit(rng) > r * r / ((r / r_s) * (1.0 + r / r_s) * (1.0 + r / r_s) * r_s * r_s * r_s));
        theta = std::acos(2.0 * unit(rng) - 1.0);
        phi = 2.0 * PI * unit(rng);

        Vec3 pos = {
            r * std::sin(theta) * std::cos(phi),
            r * std::sin(theta) * std::sin(phi),
            r * std::cos(theta)
        };

        // Circular velocity: v_c = sqrt(G * M(<r) / r) with some dispersion
        double M_enc = total_mass * cosmic::DM_FRACTION *
                       (std::log(1.0 + r / r_s) - r / (r_s + r)) /
                       (std::log(1.0 + halo_radius / r_s) - halo_radius / (r_s + halo_radius));
        double v_c = std::sqrt(G_N * M_enc / (r + 0.1));
        double sigma = v_c * 0.3; // velocity dispersion

        Vec3 vel = {
            sigma * normal(rng),
            sigma * normal(rng),
            sigma * normal(rng)
        };
        add_dark_matter(dm_mass, pos, vel);
    }

    // Gas disk (exponential disk profile)
    for (int i = 0; i < n_gas; ++i) {
        double r = -disk_radius * 0.3 * std::log(unit(rng) + 1e-10);
        if (r > disk_radius * 2.0) r = unit(rng) * disk_radius;
        double phi = 2.0 * PI * unit(rng);
        double z = normal(rng) * disk_radius * 0.02; // thin disk

        Vec3 pos = {r * std::cos(phi), r * std::sin(phi), z};

        // Circular velocity in the disk plane
        double v_c = std::sqrt(G_N * total_mass * r / (r * r + r_s * r_s));
        Vec3 vel = {-v_c * std::sin(phi), v_c * std::cos(phi), 0.0};

        add_gas(gas_mass, pos, vel, 1e4);
    }

    // Stars (thicker disk, exponential profile)
    for (int i = 0; i < n_stars; ++i) {
        double r = -disk_radius * 0.25 * std::log(unit(rng) + 1e-10);
        if (r > disk_radius * 1.5) r = unit(rng) * disk_radius;
        double phi = 2.0 * PI * unit(rng);
        double z = normal(rng) * disk_radius * 0.05;

        Vec3 pos = {r * std::cos(phi), r * std::sin(phi), z};

        double v_c = std::sqrt(G_N * total_mass * r / (r * r + r_s * r_s));
        Vec3 vel = {-v_c * std::sin(phi), v_c * std::cos(phi), 0.0};
        vel.x += normal(rng) * v_c * 0.1; // velocity dispersion
        vel.y += normal(rng) * v_c * 0.1;
        vel.z += normal(rng) * v_c * 0.05;

        add_star(star_mass, pos, vel);
    }

    box_size_ = halo_radius * 3.0;
    softening_ = box_size_ * cosmic::SOFTENING_SCALE;
    H0_ = std::sqrt(8.0 * PI * G_N * total_mass / (box_size_ * box_size_ * box_size_) / 3.0);
    adot_ = H0_ * a_;
}

void CosmicEngine::setup_galaxy_cluster(int n_galaxies, double cluster_radius) {
    clear();
    std::mt19937 rng(123);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::normal_distribution<double> normal(0.0, 1.0);

    double galaxy_mass = 1e11;

    for (int g = 0; g < n_galaxies; ++g) {
        // Random position in cluster
        double r = unit(rng) * cluster_radius;
        double theta = std::acos(2.0 * unit(rng) - 1.0);
        double phi = 2.0 * PI * unit(rng);
        Vec3 center = {
            r * std::sin(theta) * std::cos(phi),
            r * std::sin(theta) * std::sin(phi),
            r * std::cos(theta)
        };

        // Orbital velocity
        double M_enc = galaxy_mass * n_galaxies * (r * r * r) /
                       (cluster_radius * cluster_radius * cluster_radius);
        double v_c = std::sqrt(G_N * M_enc / (r + 1.0));
        Vec3 base_vel = {
            -v_c * std::sin(phi) + normal(rng) * v_c * 0.2,
            v_c * std::cos(phi) + normal(rng) * v_c * 0.2,
            normal(rng) * v_c * 0.1
        };

        // Small galaxy: 1000 DM + 500 gas + 300 stars
        double gal_radius = 30.0;
        int n_dm = 1000, n_gas = 500, n_stars = 300;

        // Central BH
        add_black_hole(galaxy_mass * 0.001, center, base_vel);

        // DM halo
        for (int i = 0; i < n_dm; ++i) {
            double lr = unit(rng) * gal_radius * 3.0;
            double lt = std::acos(2.0 * unit(rng) - 1.0);
            double lp = 2.0 * PI * unit(rng);
            Vec3 pos = center + Vec3{lr * std::sin(lt) * std::cos(lp),
                                     lr * std::sin(lt) * std::sin(lp),
                                     lr * std::cos(lt)};
            Vec3 vel = base_vel + Vec3{normal(rng) * 0.5, normal(rng) * 0.5, normal(rng) * 0.5};
            add_dark_matter(galaxy_mass * cosmic::DM_FRACTION / n_dm, pos, vel);
        }

        // Gas
        for (int i = 0; i < n_gas; ++i) {
            double lr = unit(rng) * gal_radius;
            double lp = 2.0 * PI * unit(rng);
            double lz = normal(rng) * gal_radius * 0.05;
            Vec3 pos = center + Vec3{lr * std::cos(lp), lr * std::sin(lp), lz};
            Vec3 vel = base_vel;
            add_gas(galaxy_mass * cosmic::BARYON_FRACTION * 0.6 / n_gas, pos, vel, 5e4);
        }

        // Stars
        for (int i = 0; i < n_stars; ++i) {
            double lr = unit(rng) * gal_radius * 0.8;
            double lp = 2.0 * PI * unit(rng);
            double lz = normal(rng) * gal_radius * 0.1;
            Vec3 pos = center + Vec3{lr * std::cos(lp), lr * std::sin(lp), lz};
            Vec3 vel = base_vel;
            add_star(galaxy_mass * cosmic::BARYON_FRACTION * 0.4 / n_stars, pos, vel);
        }
    }

    box_size_ = cluster_radius * 3.0;
    softening_ = box_size_ * cosmic::SOFTENING_SCALE;
    H0_ = 0.07; // ~70 km/s/Mpc in simulation units
    adot_ = H0_ * a_;
}

void CosmicEngine::setup_cosmic_web(int n_dm, double box_size) {
    clear();
    std::mt19937 rng(999);
    std::uniform_real_distribution<double> unit(-0.5, 0.5);
    std::normal_distribution<double> normal(0.0, 1.0);

    box_size_ = box_size;
    double dm_mass = 1e8; // per particle

    // Distribute DM with slight density perturbations (seed for structure)
    // Use sinusoidal perturbation spectrum to seed filaments
    for (int i = 0; i < n_dm; ++i) {
        Vec3 pos = {unit(rng) * box_size, unit(rng) * box_size, unit(rng) * box_size};

        // Add perturbation velocity (Zel'dovich approximation)
        double kx = 2.0 * PI / box_size;
        double pert_amp = 0.01 * box_size;
        Vec3 vel = {
            -pert_amp * kx * std::sin(kx * pos.x) * (1.0 + 0.3 * std::cos(kx * pos.y)),
            -pert_amp * kx * std::sin(kx * pos.y) * (1.0 + 0.3 * std::cos(kx * pos.z)),
            -pert_amp * kx * std::sin(kx * pos.z) * (1.0 + 0.3 * std::cos(kx * pos.x))
        };

        add_dark_matter(dm_mass, pos, vel);
    }

    // Add 10% gas particles following the DM distribution
    int n_gas = n_dm / 10;
    for (int i = 0; i < n_gas; ++i) {
        Vec3 pos = {unit(rng) * box_size, unit(rng) * box_size, unit(rng) * box_size};
        double kx = 2.0 * PI / box_size;
        double pert_amp = 0.01 * box_size;
        Vec3 vel = {
            -pert_amp * kx * std::sin(kx * pos.x),
            -pert_amp * kx * std::sin(kx * pos.y),
            -pert_amp * kx * std::sin(kx * pos.z)
        };
        add_gas(dm_mass * 0.17, pos, vel, 1e4); // 17% baryonic fraction
    }

    softening_ = box_size * cosmic::SOFTENING_SCALE;
    H0_ = 0.07;
    adot_ = H0_ * a_;
}

void CosmicEngine::setup_black_hole_closeup(double bh_mass, int n_gas) {
    clear();
    std::mt19937 rng(777);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::normal_distribution<double> normal(0.0, 1.0);

    add_black_hole(bh_mass, {0, 0, 0});

    // Accretion disk gas
    double r_s = 2.0 * G_N * bh_mass;
    double disk_inner = r_s * 3.0; // ISCO
    double disk_outer = r_s * 100.0;

    for (int i = 0; i < n_gas; ++i) {
        double r = disk_inner + unit(rng) * (disk_outer - disk_inner);
        double phi = 2.0 * PI * unit(rng);
        double z = normal(rng) * r_s * 0.5;

        Vec3 pos = {r * std::cos(phi), r * std::sin(phi), z};
        double v_kep = std::sqrt(G_N * bh_mass / r);
        Vec3 vel = {-v_kep * std::sin(phi), v_kep * std::cos(phi), 0.0};

        add_gas(bh_mass * 1e-8, pos, vel, 1e6);
    }

    box_size_ = disk_outer * 3.0;
    softening_ = r_s * 0.5;
    toggles.black_hole_accretion = true;
}

void CosmicEngine::setup_galaxy_merger(double mass1, double mass2,
                                       double separation) {
    clear();
    std::mt19937 rng(555);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::normal_distribution<double> normal(0.0, 1.0);

    double total = mass1 + mass2;
    double v_approach = std::sqrt(G_N * total / separation) * 0.5;

    // Galaxy 1 at (-separation/2, 0, 0)
    Vec3 c1 = {-separation / 2.0, 0, 0};
    Vec3 v1 = {v_approach, v_approach * 0.3, 0};

    add_black_hole(mass1 * 0.001, c1, v1);
    int n1 = 5000;
    double r1 = 30.0;
    for (int i = 0; i < n1; ++i) {
        double r = unit(rng) * r1;
        double phi = 2.0 * PI * unit(rng);
        double z = normal(rng) * r1 * 0.05;
        Vec3 pos = c1 + Vec3{r * std::cos(phi), r * std::sin(phi), z};
        double v_c = std::sqrt(G_N * mass1 * r / (r * r + 100.0));
        Vec3 vel = v1 + Vec3{-v_c * std::sin(phi), v_c * std::cos(phi), 0};
        if (i < n1 / 2)
            add_dark_matter(mass1 * cosmic::DM_FRACTION / (n1 / 2), pos, vel);
        else
            add_star(mass1 * cosmic::BARYON_FRACTION / (n1 / 2), pos, vel);
    }

    // Galaxy 2 at (+separation/2, 0, 0)
    Vec3 c2 = {separation / 2.0, 0, 0};
    Vec3 v2 = {-v_approach, -v_approach * 0.3, 0};

    add_black_hole(mass2 * 0.001, c2, v2);
    int n2 = 3000;
    double r2 = 25.0;
    for (int i = 0; i < n2; ++i) {
        double r = unit(rng) * r2;
        double phi = 2.0 * PI * unit(rng);
        double z = normal(rng) * r2 * 0.05;
        Vec3 pos = c2 + Vec3{r * std::cos(phi), r * std::sin(phi), z};
        double v_c = std::sqrt(G_N * mass2 * r / (r * r + 100.0));
        Vec3 vel = v2 + Vec3{-v_c * std::sin(phi), v_c * std::cos(phi), 0};
        if (i < n2 / 2)
            add_dark_matter(mass2 * cosmic::DM_FRACTION / (n2 / 2), pos, vel);
        else
            add_star(mass2 * cosmic::BARYON_FRACTION / (n2 / 2), pos, vel);
    }

    box_size_ = separation * 3.0;
    softening_ = box_size_ * cosmic::SOFTENING_SCALE;
    toggles.galaxy_mergers = true;
}

void CosmicEngine::setup_quasar(double mass, int n_gas) {
    clear();
    add_quasar(mass, {0, 0, 0});
    setup_black_hole_closeup(mass, n_gas); // Reuse accretion disk setup
    // Convert the BH to quasar type
    for (auto& b : bodies_) {
        if (b.type == CosmicBodyType::BLACK_HOLE && b.mass == mass) {
            b.type = CosmicBodyType::QUASAR;
            b.luminosity = mass * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED;
            break;
        }
    }
    toggles.relativistic_jets = true;
}

}  // namespace ftd
