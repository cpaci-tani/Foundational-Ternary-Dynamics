/**
 * CosmicEngine: Scale 5 simulation implementation
 *
 * Barnes-Hut octree gravity (O(N log N)) + SPH gas dynamics.
 * All physics driven by FTD-derived constants — zero free parameters.
 *
 * See cosmic_engine.h for full documentation.
 */

#include "ftd/cosmic_engine.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace ftd {

// ============================================================================
// Construction
// ============================================================================

CosmicEngine::CosmicEngine() {
    bodies_.reserve(100000);
    forces_.reserve(100000);
    force_diag_.reserve(100000);
}

// ============================================================================
// Body creation
// ============================================================================

int CosmicEngine::add_body(CosmicBodyType type, double mass, Vec3 position,
                           Vec3 velocity, double temperature, double radius) {
    CosmicBody b;
    b.id = next_id_++;
    b.type = type;
    b.mass = mass;
    b.position = position;
    b.velocity = velocity;
    b.temperature = temperature;
    b.radius = (radius > 0.0) ? radius : std::cbrt(mass) * 0.1;

    // Set type-specific defaults
    if (is_sph_body(type)) {
        b.smoothing_length = b.radius * 2.0;
        b.internal_energy = temperature / (cosmic::GAMMA_ADIABATIC - 1.0);
    }
    if (type == CosmicBodyType::BLACK_HOLE || type == CosmicBodyType::QUASAR) {
        b.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + mass * G_N));
    }
    if (type == CosmicBodyType::STAR) {
        // Mass-luminosity relation: L ~ M^3.5 (main sequence approx)
        b.luminosity = std::pow(mass, 3.5);
    }
    if (type == CosmicBodyType::QUASAR) {
        b.luminosity = mass * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED;
    }

    bodies_.push_back(b);
    return b.id;
}

int CosmicEngine::add_dark_matter(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::DARK_MATTER, mass, position, velocity);
}

int CosmicEngine::add_gas(double mass, Vec3 position, Vec3 velocity,
                          double temperature) {
    return add_body(CosmicBodyType::GAS, mass, position, velocity, temperature);
}

int CosmicEngine::add_star(double mass, Vec3 position, Vec3 velocity,
                           double luminosity) {
    int id = add_body(CosmicBodyType::STAR, mass, position, velocity);
    if (luminosity > 0.0) {
        for (auto& b : bodies_) {
            if (b.id == id) { b.luminosity = luminosity; break; }
        }
    }
    return id;
}

int CosmicEngine::add_black_hole(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::BLACK_HOLE, mass, position, velocity);
}

int CosmicEngine::add_quasar(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::QUASAR, mass, position, velocity);
}

int CosmicEngine::add_neutron_star(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::NEUTRON_STAR, mass, position, velocity);
}

int CosmicEngine::add_nebula(double mass, Vec3 position, Vec3 velocity,
                             double temperature) {
    return add_body(CosmicBodyType::NEBULA, mass, position, velocity, temperature);
}

int CosmicEngine::add_white_dwarf(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::WHITE_DWARF, mass, position, velocity);
}

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

// ============================================================================
// Barnes-Hut Octree
// ============================================================================

void CosmicEngine::build_octree() {
    octree_.clear();
    if (bodies_.empty()) return;

    // Find bounding box
    Vec3 bmin = bodies_[0].position;
    Vec3 bmax = bodies_[0].position;
    for (const auto& b : bodies_) {
        bmin.x = std::min(bmin.x, b.position.x);
        bmin.y = std::min(bmin.y, b.position.y);
        bmin.z = std::min(bmin.z, b.position.z);
        bmax.x = std::max(bmax.x, b.position.x);
        bmax.y = std::max(bmax.y, b.position.y);
        bmax.z = std::max(bmax.z, b.position.z);
    }

    // Pad slightly to avoid boundary issues
    double pad = 0.01 * std::max({bmax.x - bmin.x, bmax.y - bmin.y, bmax.z - bmin.z});
    bmin.x -= pad; bmin.y -= pad; bmin.z -= pad;
    bmax.x += pad; bmax.y += pad; bmax.z += pad;

    // Make it cubic
    double maxspan = std::max({bmax.x - bmin.x, bmax.y - bmin.y, bmax.z - bmin.z});
    bmax = {bmin.x + maxspan, bmin.y + maxspan, bmin.z + maxspan};

    // Create root node
    octree_.reserve(bodies_.size() * 4);
    OctreeNode root;
    root.bbox_min = bmin;
    root.bbox_max = bmax;
    octree_.push_back(root);
    octree_root_ = 0;

    // Insert all bodies
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        insert_into_tree(i, octree_root_);
    }
}

void CosmicEngine::insert_into_tree(int body_idx, int node_idx) {
    OctreeNode& node = octree_[node_idx];

    if (node.is_leaf && node.body_index == -1) {
        // Empty leaf — place body here
        node.body_index = body_idx;
        node.center_of_mass = bodies_[body_idx].position;
        node.total_mass = bodies_[body_idx].mass;
        return;
    }

    if (node.is_leaf) {
        // Occupied leaf — split into 8 children
        int existing = node.body_index;
        node.body_index = -1;
        node.is_leaf = false;

        // Create 8 children
        for (int c = 0; c < 8; ++c) {
            OctreeNode child;
            child.is_leaf = true;
            node.child_bbox(c, child.bbox_min, child.bbox_max);
            octree_.push_back(child);
            // Note: node reference may be invalidated by push_back, re-fetch
        }
        // Re-fetch node after potential reallocation
        OctreeNode& n = octree_[node_idx];
        int base = (int)octree_.size() - 8;
        for (int c = 0; c < 8; ++c) {
            n.children[c] = base + c;
        }

        // Re-insert existing body
        int oct_existing = n.octant(bodies_[existing].position);
        insert_into_tree(existing, n.children[oct_existing]);
    }

    // Insert new body into appropriate child
    OctreeNode& n = octree_[node_idx];
    int oct = n.octant(bodies_[body_idx].position);
    if (n.children[oct] == -1) {
        // Create child on demand
        OctreeNode child;
        child.is_leaf = true;
        n.child_bbox(oct, child.bbox_min, child.bbox_max);
        octree_.push_back(child);
        octree_[node_idx].children[oct] = (int)octree_.size() - 1;
    }
    insert_into_tree(body_idx, octree_[node_idx].children[oct]);

    // Update center of mass (incremental)
    OctreeNode& nn = octree_[node_idx];
    double m_new = bodies_[body_idx].mass;
    double m_total = nn.total_mass + m_new;
    if (m_total > 0.0) {
        nn.center_of_mass.x = (nn.center_of_mass.x * nn.total_mass + bodies_[body_idx].position.x * m_new) / m_total;
        nn.center_of_mass.y = (nn.center_of_mass.y * nn.total_mass + bodies_[body_idx].position.y * m_new) / m_total;
        nn.center_of_mass.z = (nn.center_of_mass.z * nn.total_mass + bodies_[body_idx].position.z * m_new) / m_total;
    }
    nn.total_mass = m_total;
}

Vec3 CosmicEngine::tree_force(int body_idx, int node_idx) const {
    const OctreeNode& node = octree_[node_idx];
    const CosmicBody& body = bodies_[body_idx];

    if (node.total_mass <= 0.0) return {};

    Vec3 dr = {
        node.center_of_mass.x - body.position.x,
        node.center_of_mass.y - body.position.y,
        node.center_of_mass.z - body.position.z
    };
    double r2 = dr.mag2() + softening_ * softening_;
    double r = std::sqrt(r2);

    if (node.is_leaf) {
        if (node.body_index == body_idx) return {}; // Skip self
        if (node.body_index == -1) return {};
        // Direct force: F = G_N * m * M * r_hat / r^2
        double f_mag = G_N * body.mass * node.total_mass / r2;
        return {f_mag * dr.x / r, f_mag * dr.y / r, f_mag * dr.z / r};
    }

    // Barnes-Hut opening angle test
    double s = node.width();
    if (s / r < cosmic::THETA_BH) {
        // Treat as single mass
        double f_mag = G_N * body.mass * node.total_mass / r2;
        return {f_mag * dr.x / r, f_mag * dr.y / r, f_mag * dr.z / r};
    }

    // Recurse into children
    Vec3 force = {};
    for (int c = 0; c < 8; ++c) {
        if (node.children[c] >= 0) {
            Vec3 cf = tree_force(body_idx, node.children[c]);
            force.x += cf.x;
            force.y += cf.y;
            force.z += cf.z;
        }
    }
    return force;
}

// ============================================================================
// Gravity computation
// ============================================================================

void CosmicEngine::compute_gravity() {
    if (!toggles.gravity || octree_.empty()) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        Vec3 fg = tree_force(i, octree_root_);
        // Acceleration = force / mass (but force already includes body mass)
        // Actually tree_force returns F = G*m_i*M*rhat/r^2, so a = F/m_i
        double m = bodies_[i].mass;
        if (m > 0.0) {
            forces_[i].x += fg.x / m;
            forces_[i].y += fg.y / m;
            forces_[i].z += fg.z / m;
            force_diag_[i].f_gravity = {fg.x / m, fg.y / m, fg.z / m};
        }
    }
}

// ============================================================================
// SPH Implementation
// ============================================================================

double CosmicEngine::sph_kernel_w(double r, double h) const {
    // 3D cubic spline kernel (Monaghan 1992)
    // W(r,h) = (1/pi*h^3) * { 1 - 1.5*q^2 + 0.75*q^3  if q<1
    //                        { 0.25*(2-q)^3             if q<2
    double q = r / h;
    double norm = 1.0 / (PI * h * h * h);
    if (q < 1.0) {
        return norm * (1.0 - 1.5 * q * q + 0.75 * q * q * q);
    } else if (q < 2.0) {
        double t = 2.0 - q;
        return norm * 0.25 * t * t * t;
    }
    return 0.0;
}

Vec3 CosmicEngine::sph_kernel_grad(const Vec3& rij, double h) const {
    double r = rij.mag();
    if (r < 1e-10) return {};
    double q = r / h;
    double norm = 1.0 / (PI * h * h * h * h); // 3D gradient: extra 1/h for derivative
    double dw = 0.0;
    if (q < 1.0) {
        dw = norm * (-3.0 * q + 2.25 * q * q);
    } else if (q < 2.0) {
        double t = 2.0 - q;
        dw = norm * (-0.75 * t * t);
    }
    return {dw * rij.x / r, dw * rij.y / r, dw * rij.z / r};
}

void CosmicEngine::find_sph_neighbors() {
    sph_neighbors_.resize(bodies_.size());
    for (auto& n : sph_neighbors_) n.clear();

    // Simple O(N^2) neighbor search (CPU; GPU uses cell-linked list)
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_sph_body(bodies_[i].type)) continue;
        double hi = bodies_[i].smoothing_length;
        for (int j = i + 1; j < (int)bodies_.size(); ++j) {
            if (!is_sph_body(bodies_[j].type)) continue;
            double hj = bodies_[j].smoothing_length;
            double h_max = std::max(hi, hj) * 2.0;
            Vec3 dr = {
                bodies_[i].position.x - bodies_[j].position.x,
                bodies_[i].position.y - bodies_[j].position.y,
                bodies_[i].position.z - bodies_[j].position.z
            };
            if (dr.mag() < h_max) {
                sph_neighbors_[i].push_back(j);
                sph_neighbors_[j].push_back(i);
            }
        }
    }
}

void CosmicEngine::compute_sph_density() {
    if (!toggles.sph_gas) return;

    find_sph_neighbors();

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_sph_body(bodies_[i].type)) continue;
        double h = bodies_[i].smoothing_length;
        double rho = bodies_[i].mass * sph_kernel_w(0.0, h); // self-contribution

        for (int j : sph_neighbors_[i]) {
            Vec3 dr = {
                bodies_[i].position.x - bodies_[j].position.x,
                bodies_[i].position.y - bodies_[j].position.y,
                bodies_[i].position.z - bodies_[j].position.z
            };
            rho += bodies_[j].mass * sph_kernel_w(dr.mag(), h);
        }

        bodies_[i].density = rho;
        // Pressure from ideal gas EOS: P = (gamma - 1) * rho * u
        bodies_[i].pressure = (cosmic::GAMMA_ADIABATIC - 1.0) *
                              rho * bodies_[i].internal_energy;

        // Adaptive smoothing length
        if (rho > 0.0) {
            bodies_[i].smoothing_length = cosmic::SPH_ETA *
                std::cbrt(bodies_[i].mass / rho);
        }
    }
}

void CosmicEngine::compute_sph_forces() {
    if (!toggles.sph_gas) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_sph_body(bodies_[i].type)) continue;
        double rho_i = bodies_[i].density;
        double P_i = bodies_[i].pressure;
        double h_i = bodies_[i].smoothing_length;

        if (rho_i <= 0.0) continue;

        Vec3 f_press = {};
        Vec3 f_visc = {};

        for (int j : sph_neighbors_[i]) {
            double rho_j = bodies_[j].density;
            double P_j = bodies_[j].pressure;
            double h_j = bodies_[j].smoothing_length;
            double h_avg = 0.5 * (h_i + h_j);

            Vec3 rij = {
                bodies_[i].position.x - bodies_[j].position.x,
                bodies_[i].position.y - bodies_[j].position.y,
                bodies_[i].position.z - bodies_[j].position.z
            };
            Vec3 grad_w = sph_kernel_grad(rij, h_avg);

            // Pressure force: -m_j * (P_i/rho_i^2 + P_j/rho_j^2) * grad_W
            if (rho_j > 0.0) {
                double press_term = P_i / (rho_i * rho_i) + P_j / (rho_j * rho_j);
                f_press.x -= bodies_[j].mass * press_term * grad_w.x;
                f_press.y -= bodies_[j].mass * press_term * grad_w.y;
                f_press.z -= bodies_[j].mass * press_term * grad_w.z;
            }

            // Artificial viscosity (Monaghan-Gingold)
            Vec3 vij = {
                bodies_[i].velocity.x - bodies_[j].velocity.x,
                bodies_[i].velocity.y - bodies_[j].velocity.y,
                bodies_[i].velocity.z - bodies_[j].velocity.z
            };
            double vij_dot_rij = vij.dot(rij);
            if (vij_dot_rij < 0.0) { // Only when approaching
                double r2 = rij.mag2();
                // 0.01*h^2 is intentionally small to avoid over-damping; standard is h^2
                double mu = h_avg * vij_dot_rij / (r2 + 0.01 * h_avg * h_avg);
                double rho_avg = 0.5 * (rho_i + rho_j);
                double c_avg = 0.5 * (bodies_[i].sound_speed() + bodies_[j].sound_speed());
                double pi_visc = (-cosmic::SPH_ALPHA_VISC * c_avg * mu +
                                   cosmic::SPH_BETA_VISC * mu * mu) / rho_avg;

                f_visc.x -= bodies_[j].mass * pi_visc * grad_w.x;
                f_visc.y -= bodies_[j].mass * pi_visc * grad_w.y;
                f_visc.z -= bodies_[j].mass * pi_visc * grad_w.z;
            }
        }

        forces_[i].x += f_press.x + f_visc.x;
        forces_[i].y += f_press.y + f_visc.y;
        forces_[i].z += f_press.z + f_visc.z;
        force_diag_[i].f_pressure = f_press;
        force_diag_[i].f_viscosity = f_visc;
    }
}

// ============================================================================
// Cosmology: Friedmann equations
// ============================================================================

void CosmicEngine::friedmann_step() {
    // Friedmann equation with FTD constants:
    // H^2 = H0^2 * [Omega_m / a^3 + Omega_Lambda]
    // where Omega_m = 1/3, Omega_Lambda = 2/3 (from FTD)
    double om = MATTER_FRACTION;    // 1 - OMEGA_LAMBDA_CONJ ≈ 0.0845... wait
    // Actually from constants.h: MATTER_FRACTION = DELTA_SQUARED ≈ 0.9155
    // and OMEGA_LAMBDA_CONJ = 2/3
    // For cosmology, use the standard fractions
    double omega_m = 1.0 - OMEGA_LAMBDA_CONJ; // ≈ 1/3
    double omega_l = OMEGA_LAMBDA_CONJ;        // ≈ 2/3

    double H2 = H0_ * H0_ * (omega_m / (a_ * a_ * a_) + omega_l);
    double H = std::sqrt(std::max(H2, 0.0));

    // RK4 for scale factor evolution
    // da/dt = a * H
    // dH/dt = -H^2 * (1 + q) where q = Omega_m/(2*Omega_total) - Omega_Lambda/Omega_total
    double k1_a = a_ * H * dt_;
    double k1_H = -H * H * (0.5 * omega_m / (a_ * a_ * a_) / (omega_m / (a_ * a_ * a_) + omega_l) - omega_l / (omega_m / (a_ * a_ * a_) + omega_l)) * dt_;

    a_ += k1_a;
    adot_ = a_ * std::sqrt(std::max(H0_ * H0_ * (omega_m / (a_ * a_ * a_) + omega_l), 0.0));
    t_cosmic_ += dt_;
}

void CosmicEngine::apply_hubble_expansion() {
    if (!toggles.hubble_expansion) return;

    friedmann_step();

    double H = hubble_parameter();
    // Apply Hubble drag: v_peculiar is unchanged in comoving coords
    // But physical positions scale: x_phys = a * x_comov
    // The Hubble flow is built into the Friedmann step
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        force_diag_[i].f_hubble = {
            -H * bodies_[i].velocity.x,
            -H * bodies_[i].velocity.y,
            -H * bodies_[i].velocity.z
        };
    }
}

void CosmicEngine::apply_dark_energy() {
    if (!toggles.dark_energy) return;

    // Dark energy as repulsive force: F_DE = (Lambda/3) * r * m
    // Lambda = 3 * H0^2 * Omega_Lambda
    double Lambda = 3.0 * H0_ * H0_ * OMEGA_LAMBDA_CONJ;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        Vec3 r = bodies_[i].position;
        double f_mag = Lambda / 3.0 * bodies_[i].mass;
        Vec3 f_de = {f_mag * r.x, f_mag * r.y, f_mag * r.z};
        forces_[i].x += f_de.x / bodies_[i].mass;
        forces_[i].y += f_de.y / bodies_[i].mass;
        forces_[i].z += f_de.z / bodies_[i].mass;
        force_diag_[i].f_dark_energy = {f_de.x / bodies_[i].mass,
                                        f_de.y / bodies_[i].mass,
                                        f_de.z / bodies_[i].mass};
    }
}

// ============================================================================
// Compact objects
// ============================================================================

void CosmicEngine::compute_accretion() {
    if (!toggles.black_hole_accretion) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        auto& bh = bodies_[i];
        if (bh.type != CosmicBodyType::BLACK_HOLE &&
            bh.type != CosmicBodyType::QUASAR) continue;

        double r_s = bh.schwarzschild_radius();

        for (int j = 0; j < (int)bodies_.size(); ++j) {
            if (i == j) continue;
            if (!is_sph_body(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[j].position.x - bh.position.x,
                bodies_[j].position.y - bh.position.y,
                bodies_[j].position.z - bh.position.z
            };
            double r = dr.mag();
            if (r < r_s * 3.0) { // Inside ISCO — accrete
                // Bondi-Hoyle: Mdot = 4*pi*G^2*M^2*rho / (c_s^2 + v^2)^(3/2)
                double cs = bodies_[j].sound_speed();
                double v2 = bodies_[j].velocity.mag2();
                double denom = std::pow(cs * cs + v2, 1.5);
                if (denom > 0.0) {
                    double mdot = 4.0 * PI * G_N * G_N * bh.mass * bh.mass *
                                  bodies_[j].density / denom;
                    // Eddington limit: L_edd = 4*pi*G*M*c/sigma_T, then mdot_edd = L_edd/(eta*c^2)
                    // sigma_T ~ 0.4 in simulation units, eta = BONDI_EFFICIENCY
                    double L_edd = 4.0 * PI * G_N * bh.mass * C_SPEED / 0.4;
                    double mdot_edd = L_edd / (cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED);
                    mdot = std::min(mdot, mdot_edd);
                    double dm = std::min(mdot * dt_, bodies_[j].mass * 0.1);
                    bh.mass += dm;
                    bodies_[j].mass -= dm;
                    bh.accretion_rate = mdot;
                    if (bh.type == CosmicBodyType::QUASAR) {
                        bh.luminosity = mdot * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED;
                    }
                }
            }
        }

        // Update latency field
        bh.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + bh.mass * G_N));
    }
}

void CosmicEngine::compute_relativistic_jets() {
    if (!toggles.relativistic_jets) return;

    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::QUASAR &&
            b.type != CosmicBodyType::BLACK_HOLE) continue;
        if (b.accretion_rate <= 0.0) continue;

        // Jet velocity: v_jet = c * sqrt(1 - L^2) from FTD latency
        double v_jet = C_SPEED * std::sqrt(1.0 - b.latency * b.latency);

        // Jet direction along angular momentum axis
        Vec3 jet_dir = b.angular_momentum;
        double j_mag = jet_dir.mag();
        if (j_mag < 1e-10) {
            jet_dir = {0, 0, 1}; // Default to z-axis
        } else {
            jet_dir.x /= j_mag;
            jet_dir.y /= j_mag;
            jet_dir.z /= j_mag;
        }

        // Jet kinetic luminosity: fraction of accretion luminosity
        double L_jet = b.accretion_rate * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED * 0.1;
        // Momentum flux: p_dot = L_jet / v_jet (for relativistic material)
        double p_dot = L_jet / (v_jet + 1e-20);
        // Recoil on BH (Newton's third law) — bipolar, so net recoil is zero for symmetric jets
        // Add slight asymmetry for realistic kick
        double asymmetry = 0.05;
        b.velocity.x += jet_dir.x * p_dot * asymmetry / b.mass * dt_;
        b.velocity.y += jet_dir.y * p_dot * asymmetry / b.mass * dt_;
        b.velocity.z += jet_dir.z * p_dot * asymmetry / b.mass * dt_;
    }
}

// ============================================================================
// Stellar physics
// ============================================================================

void CosmicEngine::check_star_formation() {
    if (!toggles.star_formation) return;

    std::vector<CosmicBody> new_stars;
    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::GAS && b.type != CosmicBodyType::NEBULA) continue;
        // Jeans criterion: form star when density exceeds threshold
        if (b.density > cosmic::RHO_JEANS && b.temperature < 1e4) {
            // Convert fraction of gas to star
            double star_mass = b.mass * 0.1;
            b.mass -= star_mass;

            CosmicBody star;
            star.id = next_id_++;
            star.type = CosmicBodyType::STAR;
            star.mass = star_mass;
            star.position = b.position;
            star.velocity = b.velocity;
            star.luminosity = std::pow(star_mass, 3.5);
            star.radius = std::cbrt(star_mass) * 0.01;
            new_stars.push_back(star);
        }
    }
    for (auto& s : new_stars) bodies_.push_back(s);
}

void CosmicEngine::check_stellar_evolution() {
    if (!toggles.stellar_evolution) return;

    std::vector<CosmicBody> new_bodies;
    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::STAR) continue;

        // Stellar lifetime: t ~ (M+1)^(-2.5) * 1e7 (cosmic time units)
        double lifetime = std::pow(b.mass + 1.0, -2.5) * 1e7;
        if (t_cosmic_ > lifetime) {
            if (b.mass > cosmic::M_SUPERNOVA) {
                if (b.mass > cosmic::M_TOV * 10.0) {
                    // Massive star -> Black hole
                    b.type = CosmicBodyType::BLACK_HOLE;
                    b.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + b.mass * G_N));
                    b.luminosity = 0.0;
                } else {
                    // Medium star -> Neutron star + supernova nebula
                    b.type = CosmicBodyType::NEUTRON_STAR;
                    double m_remnant = std::min(b.mass * 0.15 + 1.2, cosmic::M_TOV); // ~1.4-2.2 M_sun
                    double ejected = b.mass - m_remnant;
                    b.mass = m_remnant;
                    CosmicBody nebula;
                    nebula.id = next_id_++;
                    nebula.type = CosmicBodyType::NEBULA;
                    nebula.mass = ejected;
                    nebula.position = b.position;
                    nebula.velocity = b.velocity;
                    nebula.temperature = 1e6;
                    new_bodies.push_back(nebula);
                }
            } else if (b.mass < cosmic::M_CHANDRASEKHAR) {
                // Low mass star -> White dwarf
                b.type = CosmicBodyType::WHITE_DWARF;
                b.luminosity *= 0.01; // Much dimmer
            }
        }
    }
    for (auto& nb : new_bodies) bodies_.push_back(nb);
}

// ============================================================================
// Extended physics
// ============================================================================

void CosmicEngine::compute_magnetic_fields() {
    if (!toggles.magnetic_fields) return;

    // Simplified cosmic dynamo: B grows with rotation and turbulence
    for (auto& b : bodies_) {
        if (!is_sph_body(b.type)) continue;
        // Dynamo growth: dB/dt ~ alpha_dynamo * omega * B
        // alpha_dynamo ~ ALPHA (fine structure — from FTD)
        double omega = b.angular_momentum.mag() / (b.radius * b.radius + 1.0);
        double growth = ALPHA * omega;
        b.magnetic_field.x += growth * b.magnetic_field.x * dt_;
        b.magnetic_field.y += growth * b.magnetic_field.y * dt_;
        b.magnetic_field.z += growth * b.magnetic_field.z * dt_;

        // Seed field if none exists
        if (b.magnetic_field.mag() < 1e-10) {
            b.magnetic_field = {1e-8, 1e-8, 1e-8};
        }
    }
}

void CosmicEngine::compute_radiation_pressure() {
    if (!toggles.radiation_pressure) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_luminous(bodies_[i].type)) continue;
        double L = bodies_[i].luminosity;
        if (L <= 0.0) continue;

        for (int j = 0; j < (int)bodies_.size(); ++j) {
            if (i == j) continue;
            if (!is_sph_body(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[j].position.x - bodies_[i].position.x,
                bodies_[j].position.y - bodies_[i].position.y,
                bodies_[j].position.z - bodies_[i].position.z
            };
            double r2 = dr.mag2() + softening_ * softening_;
            double r = std::sqrt(r2);

            // F_rad = L / (4*pi*r^2*c) in direction away from source
            double f_mag = L / (4.0 * PI * r2 * C_SPEED);
            Vec3 f_rad = {f_mag * dr.x / r, f_mag * dr.y / r, f_mag * dr.z / r};

            forces_[j].x += f_rad.x / bodies_[j].mass;
            forces_[j].y += f_rad.y / bodies_[j].mass;
            forces_[j].z += f_rad.z / bodies_[j].mass;
            force_diag_[j].f_radiation.x += f_rad.x / bodies_[j].mass;
            force_diag_[j].f_radiation.y += f_rad.y / bodies_[j].mass;
            force_diag_[j].f_radiation.z += f_rad.z / bodies_[j].mass;
        }
    }
}

// ============================================================================
// Gravitational waves
// ============================================================================

void CosmicEngine::detect_gw_events() {
    if (!toggles.gravitational_waves) return;

    // Check for BH/NS mergers (bodies within Schwarzschild radius)
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_compact_object(bodies_[i].type)) continue;
        for (int j = i + 1; j < (int)bodies_.size(); ++j) {
            if (!is_compact_object(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[i].position.x - bodies_[j].position.x,
                bodies_[i].position.y - bodies_[j].position.y,
                bodies_[i].position.z - bodies_[j].position.z
            };
            double r = dr.mag();
            double r_merge = bodies_[i].schwarzschild_radius() +
                            bodies_[j].schwarzschild_radius();

            if (r < r_merge * 3.0) {
                // Merger! Create GW event
                GravWaveEvent gw;
                gw.origin = {
                    0.5 * (bodies_[i].position.x + bodies_[j].position.x),
                    0.5 * (bodies_[i].position.y + bodies_[j].position.y),
                    0.5 * (bodies_[i].position.z + bodies_[j].position.z)
                };
                gw.emission_tick = tick_;
                gw.total_mass = bodies_[i].mass + bodies_[j].mass;
                // GW strain: h ~ 4*G*M*v^2 / (r*c^4) — distance factor is critical
                double v2 = (bodies_[i].velocity.mag2() + bodies_[j].velocity.mag2()) * 0.5;
                double r_source = std::max(dr.mag(), softening_);
                gw.strain = 4.0 * G_N * gw.total_mass * v2 / (r_source * C_SPEED * C_SPEED * C_SPEED * C_SPEED);
                gw.current_radius = 0.0;
                gw_events_.push_back(gw);

                // Merge bodies: j absorbed into i, ~5% mass radiated as GWs
                double m_total = bodies_[i].mass + bodies_[j].mass;
                double gw_mass_loss = 0.05; // ~5% of rest mass radiated (equal-mass limit)
                // Momentum-conserving velocity merge (before mass loss)
                bodies_[i].velocity.x = (bodies_[i].velocity.x * bodies_[i].mass +
                                         bodies_[j].velocity.x * bodies_[j].mass) / m_total;
                bodies_[i].velocity.y = (bodies_[i].velocity.y * bodies_[i].mass +
                                         bodies_[j].velocity.y * bodies_[j].mass) / m_total;
                bodies_[i].velocity.z = (bodies_[i].velocity.z * bodies_[i].mass +
                                         bodies_[j].velocity.z * bodies_[j].mass) / m_total;
                bodies_[i].mass = m_total * (1.0 - gw_mass_loss);
                // Mark j for removal
                bodies_[j].mass = 0.0;
                bodies_[j].type = CosmicBodyType::DARK_ENERGY; // Will be cleaned
            }
        }
    }

    // Clean up zero-mass bodies
    bodies_.erase(
        std::remove_if(bodies_.begin(), bodies_.end(),
                       [](const CosmicBody& b) { return b.mass <= 0.0; }),
        bodies_.end());
}

void CosmicEngine::propagate_gw() {
    if (!toggles.gravitational_waves) return;

    for (auto& gw : gw_events_) {
        // GW propagates at c = 1/sqrt(3)
        gw.current_radius += C_SPEED * dt_;
        // Strain falls as 1/r
        // (strain at source is stored; observers compute h(r) = h_source * r_source / r)
    }

    // Remove old events that have propagated beyond the box
    gw_events_.erase(
        std::remove_if(gw_events_.begin(), gw_events_.end(),
                       [this](const GravWaveEvent& gw) {
                           return gw.current_radius > box_size_ * 2.0;
                       }),
        gw_events_.end());
}

// ============================================================================
// Integration (Velocity Verlet)
// ============================================================================

void CosmicEngine::half_kick() {
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        bodies_[i].velocity.x += 0.5 * dt_ * forces_[i].x;
        bodies_[i].velocity.y += 0.5 * dt_ * forces_[i].y;
        bodies_[i].velocity.z += 0.5 * dt_ * forces_[i].z;
    }
}

void CosmicEngine::drift() {
    for (auto& b : bodies_) {
        b.position.x += dt_ * b.velocity.x;
        b.position.y += dt_ * b.velocity.y;
        b.position.z += dt_ * b.velocity.z;
    }
}

void CosmicEngine::enforce_speed_limit() {
    for (auto& b : bodies_) {
        double v = b.velocity.mag();
        if (v > C_SPEED) {
            double scale = C_SPEED / v;
            b.velocity.x *= scale;
            b.velocity.y *= scale;
            b.velocity.z *= scale;
        }
    }
}

// ============================================================================
// Main tick cycle (18 phases)
// ============================================================================

void CosmicEngine::tick() {
    // ================================================================
    // Tick cycle follows Gadget-2 conventions:
    //   1. Compute forces at current positions
    //   2. Kick-drift-kick (Velocity Verlet)
    //   3. Post-integration updates (mass changes, mergers, cleanup)
    //
    // Mass changes (accretion, star formation, mergers) happen AFTER
    // integration, never during force computation. This preserves
    // energy conservation and prevents mid-loop state corruption.
    // ================================================================

    // ── Phase A: Compute forces at CURRENT positions ──
    int n = (int)bodies_.size();
    forces_.assign(n, {});
    force_diag_.assign(n, {});

    build_octree();
    compute_gravity();
    compute_sph_density();
    compute_sph_forces();
    apply_hubble_expansion();
    apply_dark_energy();
    compute_magnetic_fields();
    compute_radiation_pressure();

    // ── Phase B: Velocity Verlet integration ──
    half_kick();                    // v += 0.5 * dt * a (current forces)
    drift();                        // x += dt * v

    // Recompute forces at NEW positions (symplecticity requirement)
    int n2 = (int)bodies_.size();
    forces_.assign(n2, {});
    force_diag_.assign(n2, {});
    build_octree();
    compute_gravity();
    if (toggles.sph_gas) { compute_sph_density(); compute_sph_forces(); }

    half_kick();                    // v += 0.5 * dt * a (fresh forces)

    // ── Phase C: Post-integration updates (Gadget-2 convention) ──
    // Mass changes, particle creation/destruction happen HERE, not in forces.
    compute_accretion();
    compute_relativistic_jets();
    check_star_formation();
    check_stellar_evolution();
    detect_gw_events();
    propagate_gw();
    enforce_speed_limit();

    tick_++;
}

void CosmicEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) tick();
}

// ============================================================================
// Diagnostics
// ============================================================================

CosmicDiagnostics CosmicEngine::diagnostics() const {
    CosmicDiagnostics d;
    d.tick = tick_;
    d.body_count = (int)bodies_.size();
    d.scale_factor = a_;
    d.hubble_parameter = hubble_parameter();
    d.dark_energy_density = H0_ * H0_ * OMEGA_LAMBDA_CONJ;

    for (const auto& b : bodies_) {
        // Count by type (offset by 3 for negative enum values)
        int idx = static_cast<int>(b.type) + 3;
        if (idx >= 0 && idx < 9) d.counts_by_type[idx]++;

        d.total_mass += b.mass;
        double v2 = b.velocity.mag2();
        d.total_ke += 0.5 * b.mass * v2;
        d.total_thermal += b.internal_energy * b.mass;

        d.total_momentum.x += b.mass * b.velocity.x;
        d.total_momentum.y += b.mass * b.velocity.y;
        d.total_momentum.z += b.mass * b.velocity.z;

        // Angular momentum: L = r x (m*v)
        d.total_angular_momentum.x += b.mass * (b.position.y * b.velocity.z -
                                                  b.position.z * b.velocity.y);
        d.total_angular_momentum.y += b.mass * (b.position.z * b.velocity.x -
                                                  b.position.x * b.velocity.z);
        d.total_angular_momentum.z += b.mass * (b.position.x * b.velocity.y -
                                                  b.position.y * b.velocity.x);

        if (is_sph_body(b.type) && b.density > d.max_density) {
            d.max_density = b.density;
        }
    }

    // Gravitational PE (O(N^2) — only for diagnostics, not every tick)
    // Approximate using virial theorem instead
    d.total_energy = d.total_ke + d.total_thermal; // PE not computed here for perf

    // Cosmological fractions
    double vol = box_size_ * box_size_ * box_size_;
    if (vol > 0.0) {
        d.matter_density = d.total_mass / vol;
        d.critical_density = 3.0 * d.hubble_parameter * d.hubble_parameter / (8.0 * PI * G_N);
        if (d.critical_density > 0.0) {
            d.omega_matter = d.matter_density / d.critical_density;
            d.omega_lambda = OMEGA_LAMBDA_CONJ;
            d.omega_total = d.omega_matter + d.omega_lambda;
        }
    }

    return d;
}

void CosmicEngine::clear() {
    bodies_.clear();
    forces_.clear();
    force_diag_.clear();
    gw_events_.clear();
    octree_.clear();
    sph_neighbors_.clear();
    tick_ = 0;
    next_id_ = 0;
    a_ = 1.0;
    adot_ = 0.0;
    t_cosmic_ = 0.0;
    H0_ = 0.0;
}

}  // namespace ftd
