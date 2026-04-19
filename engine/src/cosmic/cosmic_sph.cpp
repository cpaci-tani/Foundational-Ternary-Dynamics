/**
 * CosmicEngine SPH hydrodynamics.
 *
 * Extracted from cosmic_engine.cpp (CE3). Owns the Monaghan 1992 cubic spline
 * kernel math, the O(N^2) neighbor search, density computation, and the
 * pressure + artificial viscosity force evaluation.
 */

#include "ftd/cosmic_engine.h"
#include <algorithm>
#include <cmath>

namespace ftd {

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

}  // namespace ftd
