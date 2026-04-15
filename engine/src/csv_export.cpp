#include "ftd/csv_export.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace ftd {
namespace csv {

bool export_flux_field(const RenderBridge& bridge, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;
    out << "x,y,z,Jx,Jy,Jz,density,state,latency,tau\n";

    const auto& lat = bridge.lattice();
    const auto& voxels = bridge.voxels();
    int N = lat.size();

    for (int idx = 0; idx < lat.total_sites(); ++idx) {
        auto c = lat.coord(idx);
        const auto& v = voxels[idx];
        out << c.x << "," << c.y << "," << c.z << ","
            << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
            << v.density() << ","
            << static_cast<int>(v.state) << ","
            << v.latency << ","
            << v.tau << "\n";
    }

    out.close();
    return true;
}

bool export_density_slice(const RenderBridge& bridge, const std::string& filename,
                          char axis, int index) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;

    const auto& lat = bridge.lattice();
    const auto& voxels = bridge.voxels();
    int N = lat.size();

    // Validate index
    if (index < 0 || index >= N) {
        std::cerr << "CSV export error: slice index " << index
                  << " out of range [0, " << N << ")\n";
        return false;
    }

    // Write header with axis labels
    switch (axis) {
        case 'x': case 'X':
            out << "y,z,density,Jx,Jy,Jz,state\n";
            for (int y = 0; y < N; ++y) {
                for (int z = 0; z < N; ++z) {
                    int idx = lat.index(index, y, z);
                    const auto& v = voxels[idx];
                    out << y << "," << z << ","
                        << v.density() << ","
                        << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
                        << static_cast<int>(v.state) << "\n";
                }
            }
            break;
        case 'y': case 'Y':
            out << "x,z,density,Jx,Jy,Jz,state\n";
            for (int x = 0; x < N; ++x) {
                for (int z = 0; z < N; ++z) {
                    int idx = lat.index(x, index, z);
                    const auto& v = voxels[idx];
                    out << x << "," << z << ","
                        << v.density() << ","
                        << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
                        << static_cast<int>(v.state) << "\n";
                }
            }
            break;
        case 'z': case 'Z':
            out << "x,y,density,Jx,Jy,Jz,state\n";
            for (int x = 0; x < N; ++x) {
                for (int y = 0; y < N; ++y) {
                    int idx = lat.index(x, y, index);
                    const auto& v = voxels[idx];
                    out << x << "," << y << ","
                        << v.density() << ","
                        << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
                        << static_cast<int>(v.state) << "\n";
                }
            }
            break;
        default:
            std::cerr << "CSV export error: invalid axis '" << axis
                      << "' (use 'x', 'y', or 'z')\n";
            return false;
    }

    out.close();
    return true;
}

bool export_diagnostics_row(const RenderBridge& bridge, const std::string& filename) {
    // Check if file exists and has content
    bool needs_header = false;
    {
        std::ifstream test(filename);
        if (!test.is_open() || test.peek() == std::ifstream::traits_type::eof()) {
            needs_header = true;
        }
    }

    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;

    if (needs_header) {
        out << "tick,manifested,positive,negative,total_flux,"
               "total_energy,avg_drag,max_bandwidth,total_entropy,"
               "field_energy,wave_energy,particle_ke,gauss_violation,charge_total\n";
    }

    auto d = bridge.diagnostics();
    auto ea = bridge.energy_audit();
    out << d.tick << ","
        << d.manifested_count << ","
        << d.positive_count << ","
        << d.negative_count << ","
        << d.total_flux << ","
        << d.total_energy << ","
        << d.avg_drag << ","
        << d.max_bandwidth << ","
        << d.total_entropy << ","
        << ea.field_energy << ","
        << ea.wave_energy << ","
        << ea.particle_ke << ","
        << ea.gauss_violation << ","
        << ea.charge_total << "\n";

    out.close();
    return true;
}

bool export_line_profile(const RenderBridge& bridge, const std::string& filename,
                         char axis, int fixed1, int fixed2) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;
    out << "position,density,Jx,Jy,Jz,state\n";

    const auto& lat = bridge.lattice();
    const auto& voxels = bridge.voxels();
    int N = lat.size();

    for (int i = 0; i < N; ++i) {
        int idx = 0;
        switch (axis) {
            case 'x': case 'X': idx = lat.index(i, fixed1, fixed2); break;
            case 'y': case 'Y': idx = lat.index(fixed1, i, fixed2); break;
            case 'z': case 'Z': idx = lat.index(fixed1, fixed2, i); break;
            default:
                std::cerr << "CSV export error: invalid axis '" << axis << "'\n";
                return false;
        }
        const auto& v = voxels[idx];
        out << i << ","
            << v.density() << ","
            << v.flux.x << "," << v.flux.y << "," << v.flux.z << ","
            << static_cast<int>(v.state) << "\n";
    }

    out.close();
    return true;
}

bool export_particle_snapshot(const RenderBridge& bridge, const std::string& filename) {
    bool needs_header = false;
    {
        std::ifstream test(filename);
        if (!test.is_open() || test.peek() == std::ifstream::traits_type::eof()) {
            needs_header = true;
        }
    }

    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;

    if (needs_header) {
        out << "tick,particle_id,x,y,z,state,vx,vy,vz,"
               "density,spin,color,pair_id,f_em_mag,f_grav_mag\n";
    }

    const auto& lat = bridge.lattice();
    const auto& voxels = bridge.voxels();
    int tick = bridge.current_tick();

    for (int idx = 0; idx < lat.total_sites(); ++idx) {
        const auto& v = voxels[idx];
        if (v.state == 0) continue;

        auto c = lat.coord(idx);
        const auto& fd = bridge.force_diag_at(idx);
        out << tick << ","
            << v.particle_id << ","
            << c.x << "," << c.y << "," << c.z << ","
            << static_cast<int>(v.state) << ","
            << v.velocity.x << "," << v.velocity.y << "," << v.velocity.z << ","
            << v.density() << ","
            << static_cast<int>(v.spin) << ","
            << static_cast<int>(v.color) << ","
            << v.pair_id << ","
            << fd.f_coulomb.mag() << ","
            << fd.f_gravity.mag() << "\n";
    }

    out.close();
    return true;
}

bool export_radial_profile(const RenderBridge& bridge, const std::string& filename,
                           int cx, int cy, int cz) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "CSV export error: cannot open " << filename << "\n";
        return false;
    }

    out << std::setprecision(8) << std::scientific;
    out << "r,axis,grad_divJ_mag,density,div_J,state\n";

    const auto& lat = bridge.lattice();
    const auto& voxels = bridge.voxels();
    int N = lat.size();
    int max_r = N / 2 - 1;

    // Axes: +x, +y, +z, +xyz diagonal
    const char* axis_names[] = {"+x", "+y", "+z", "+xyz"};
    int dx[] = {1, 0, 0, 1};
    int dy[] = {0, 1, 0, 1};
    int dz[] = {0, 0, 1, 1};

    for (int a = 0; a < 4; ++a) {
        for (int r = 1; r <= max_r; ++r) {
            int px = cx + dx[a] * r;
            int py = cy + dy[a] * r;
            int pz = cz + dz[a] * r;

            // Wrap periodically
            px = ((px % N) + N) % N;
            py = ((py % N) + N) % N;
            pz = ((pz % N) + N) % N;

            int idx = lat.index(px, py, pz);
            const auto& v = voxels[idx];
            Vec3 gdj = bridge.gradient_divergence(idx);
            double div = bridge.divergence_flux(idx);

            double actual_r = (a == 3) ? r * std::sqrt(3.0) : static_cast<double>(r);

            out << actual_r << ","
                << axis_names[a] << ","
                << gdj.mag() << ","
                << v.density() << ","
                << div << ","
                << static_cast<int>(v.state) << "\n";
        }
    }

    out.close();
    return true;
}

}  // namespace csv
}  // namespace ftd
