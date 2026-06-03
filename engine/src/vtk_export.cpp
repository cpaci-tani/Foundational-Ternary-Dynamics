#include "ftd/vtk_export.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

#include "ftd/field_operators.h"

namespace ftd {
namespace sciviz {
namespace {

namespace fs = std::filesystem;

std::string frame_name(int frame, const char* ext) {
    std::ostringstream os;
    os << "frame_" << std::setw(6) << std::setfill('0') << frame << "." << ext;
    return os.str();
}

std::string path_join(const std::string& a, const std::string& b) {
    return (fs::path(a) / fs::path(b)).generic_string();
}

std::string xml_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

bool create_dir(const fs::path& p) {
    std::error_code ec;
    fs::create_directories(p, ec);
    if (ec) {
        std::cerr << "VTK export error: cannot create " << p.string()
                  << ": " << ec.message() << "\n";
        return false;
    }
    return true;
}

void open_data_array(std::ostream& out,
                     const char* type,
                     const char* name,
                     int components = 1) {
    out << "        <DataArray type=\"" << type << "\" Name=\"" << name << "\"";
    if (components > 1) out << " NumberOfComponents=\"" << components << "\"";
    out << " format=\"ascii\">\n          ";
}

void close_data_array(std::ostream& out) {
    out << "\n        </DataArray>\n";
}

template <typename Fn>
void write_sampled_scalar(std::ostream& out, const Lattice& lat, int stride, Fn fn) {
    const int N = lat.size();
    bool first = true;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                if (!first) out << ' ';
                first = false;
                out << fn(lat.index(x, y, z));
            }
        }
    }
}

template <typename Fn>
void write_sampled_vec3(std::ostream& out, const Lattice& lat, int stride, Fn fn) {
    const int N = lat.size();
    bool first = true;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                Vec3 v = fn(lat.index(x, y, z));
                if (!first) out << ' ';
                first = false;
                out << v.x << ' ' << v.y << ' ' << v.z;
            }
        }
    }
}

template <typename Fn>
void write_scalar_array(std::ostream& out,
                        const char* type,
                        const char* name,
                        const Lattice& lat,
                        int stride,
                        Fn fn) {
    open_data_array(out, type, name);
    write_sampled_scalar(out, lat, stride, fn);
    close_data_array(out);
}

template <typename Fn>
void write_vec3_array(std::ostream& out,
                      const char* name,
                      const Lattice& lat,
                      int stride,
                      Fn fn) {
    open_data_array(out, "Float64", name, 3);
    write_sampled_vec3(out, lat, stride, fn);
    close_data_array(out);
}

std::vector<const ClusterHistory*> current_histories(const ClusterTracker& tracker, int tick) {
    std::vector<const ClusterHistory*> out;
    for (const auto& kv : tracker.histories()) {
        const ClusterHistory& h = kv.second;
        if (h.snapshots.empty()) continue;
        if (h.snapshots.back().tick == tick && h.alive()) out.push_back(&h);
    }
    std::sort(out.begin(), out.end(), [](const ClusterHistory* a, const ClusterHistory* b) {
        return a->cluster_id < b->cluster_id;
    });
    return out;
}

void write_vtp_vertices(std::ostream& out, int n) {
    out << "      <Verts>\n";
    open_data_array(out, "Int32", "connectivity");
    for (int i = 0; i < n; ++i) {
        if (i) out << ' ';
        out << i;
    }
    close_data_array(out);
    open_data_array(out, "Int32", "offsets");
    for (int i = 0; i < n; ++i) {
        if (i) out << ' ';
        out << (i + 1);
    }
    close_data_array(out);
    out << "      </Verts>\n";
}

}  // namespace

ResearchExportSession::ResearchExportSession(ExportOptions options)
    : options_(std::move(options)) {
    if (options_.frame_interval < 1) options_.frame_interval = 1;
    if (options_.spatial_stride < 1) options_.spatial_stride = 1;
}

bool ResearchExportSession::ensure_directories() {
    if (!create_dir(options_.output_dir)) return false;
    if (options_.export_fields && !create_dir(fs::path(options_.output_dir) / "fields")) return false;
    if (options_.export_particles && !create_dir(fs::path(options_.output_dir) / "particles")) return false;
    if (options_.export_clusters && !create_dir(fs::path(options_.output_dir) / "clusters")) return false;
    return true;
}

bool ResearchExportSession::record(const RenderBridge& bridge) {
    if (!ensure_directories()) return false;

    const int frame_index = static_cast<int>(frames_.size());
    FrameRecord rec;
    rec.frame = frame_index;
    rec.tick = bridge.current_tick();
    rec.time = bridge.physical_time();

    if (options_.export_fields &&
        !write_field_frame(bridge, frame_index, rec.fields_file)) return false;
    if (options_.export_particles &&
        !write_particle_frame(bridge, frame_index, rec.particles_file)) return false;

    if (options_.export_clusters) {
        cluster_tracker_.record(bridge);
        if (!write_cluster_frame(bridge, frame_index, rec.clusters_file)) return false;
    }

    if (!append_diagnostics(bridge, frame_index)) return false;
    frames_.push_back(std::move(rec));
    return true;
}

bool ResearchExportSession::write_field_frame(const RenderBridge& bridge,
                                              int frame_index,
                                              std::string& relative_file) const {
    relative_file = path_join("fields", frame_name(frame_index, "vti"));
    const fs::path full_path = fs::path(options_.output_dir) / fs::path(relative_file);
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    const auto& lat = bridge.lattice();
    const auto& vox = bridge.voxels();
    const auto& force = bridge.force_diag();
    const int stride = options_.spatial_stride;
    const int N = lat.size();
    const int sx = (N + stride - 1) / stride;
    const int sy = sx;
    const int sz = sx;

    out << std::setprecision(17) << std::scientific;
    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <ImageData WholeExtent=\"0 " << (sx - 1)
        << " 0 " << (sy - 1)
        << " 0 " << (sz - 1)
        << "\" Origin=\"0 0 0\" Spacing=\"" << stride << ' ' << stride << ' ' << stride << "\">\n";
    out << "    <Piece Extent=\"0 " << (sx - 1)
        << " 0 " << (sy - 1)
        << " 0 " << (sz - 1) << "\">\n";
    out << "      <PointData Scalars=\"density\" Vectors=\"flux\">\n";

    write_scalar_array(out, "Int32", "state", lat, stride,
                       [&](int i) { return static_cast<int>(vox[i].state); });
    write_scalar_array(out, "Float64", "density", lat, stride,
                       [&](int i) { return vox[i].density(); });
    write_vec3_array(out, "flux", lat, stride,
                     [&](int i) { return vox[i].flux; });
    write_vec3_array(out, "wave_vel", lat, stride,
                     [&](int i) { return vox[i].wave_vel; });
    write_vec3_array(out, "velocity", lat, stride,
                     [&](int i) { return vox[i].velocity; });
    write_scalar_array(out, "Float64", "latency", lat, stride,
                       [&](int i) { return vox[i].latency; });
    write_scalar_array(out, "Float64", "tau", lat, stride,
                       [&](int i) { return vox[i].tau; });
    write_scalar_array(out, "Int32", "spin", lat, stride,
                       [&](int i) { return static_cast<int>(vox[i].spin); });
    write_scalar_array(out, "Int32", "color", lat, stride,
                       [&](int i) { return static_cast<int>(vox[i].color); });
    write_scalar_array(out, "Int32", "flavor", lat, stride,
                       [&](int i) { return static_cast<int>(vox[i].flavor); });
    write_scalar_array(out, "Int32", "particle_id", lat, stride,
                       [&](int i) { return vox[i].particle_id; });
    write_scalar_array(out, "Int32", "pair_id", lat, stride,
                       [&](int i) { return vox[i].pair_id; });
    write_scalar_array(out, "Int32", "locked", lat, stride,
                       [&](int i) { return vox[i].locked ? 1 : 0; });

    if (options_.export_operators) {
        write_scalar_array(out, "Float64", "div_J", lat, stride,
                           [&](int i) { return divergence_flux_op(vox, lat, i); });
        write_scalar_array(out, "Float64", "gauss_error", lat, stride,
                           [&](int i) { return divergence_flux_op(vox, lat, i) - static_cast<double>(vox[i].state); });
        write_vec3_array(out, "curl_J", lat, stride,
                         [&](int i) { return curl_flux_op(vox, lat, i); });
        write_vec3_array(out, "E", lat, stride,
                         [&](int i) { return vox[i].wave_vel * -1.0; });
        write_vec3_array(out, "B", lat, stride,
                         [&](int i) { return curl_flux_op(vox, lat, i); });
        write_vec3_array(out, "poynting", lat, stride,
                         [&](int i) {
                             Vec3 e = vox[i].wave_vel * -1.0;
                             Vec3 b = curl_flux_op(vox, lat, i);
                             return Vec3::cross(e, b);
                         });
    }

    if (options_.export_forces) {
        write_vec3_array(out, "force_coulomb", lat, stride,
                         [&](int i) { return force[i].f_coulomb; });
        write_vec3_array(out, "force_strong", lat, stride,
                         [&](int i) { return force[i].f_strong; });
        write_vec3_array(out, "force_magnetic", lat, stride,
                         [&](int i) { return force[i].f_magnetic; });
        write_vec3_array(out, "force_gravity", lat, stride,
                         [&](int i) { return force[i].f_gravity; });
        write_vec3_array(out, "force_exchange", lat, stride,
                         [&](int i) { return force[i].f_exchange; });
    }

    if (options_.export_dual_fields) {
        write_vec3_array(out, "flux_L", lat, stride,
                         [&](int i) { return vox[i].flux_L; });
        write_vec3_array(out, "flux_R", lat, stride,
                         [&](int i) { return vox[i].flux_R; });
    }

    if (options_.export_interaction_fields) {
        write_vec3_array(out, "flux_strong", lat, stride,
                         [&](int i) { return vox[i].flux_strong; });
        write_vec3_array(out, "flux_weak", lat, stride,
                         [&](int i) { return vox[i].flux_weak; });
    }

    out << "      </PointData>\n";
    out << "      <CellData/>\n";
    out << "    </Piece>\n";
    out << "  </ImageData>\n";
    out << "</VTKFile>\n";
    return true;
}

bool ResearchExportSession::write_particle_frame(const RenderBridge& bridge,
                                                 int frame_index,
                                                 std::string& relative_file) const {
    relative_file = path_join("particles", frame_name(frame_index, "vtp"));
    const fs::path full_path = fs::path(options_.output_dir) / fs::path(relative_file);
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    const auto& lat = bridge.lattice();
    const auto& vox = bridge.voxels();
    const auto& force = bridge.force_diag();
    std::vector<int> ids;
    ids.reserve(bridge.diagnostics().manifested_count);
    for (int64_t i = 0; i < lat.total_sites(); ++i) {
        if (vox[static_cast<size_t>(i)].state != 0) ids.push_back(static_cast<int>(i));
    }

    out << std::setprecision(17) << std::scientific;
    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <PolyData>\n";
    out << "    <Piece NumberOfPoints=\"" << ids.size()
        << "\" NumberOfVerts=\"" << ids.size()
        << "\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
    out << "      <PointData Scalars=\"state\" Vectors=\"flux\">\n";

    auto write_particle_scalar = [&](const char* type, const char* name, auto fn) {
        open_data_array(out, type, name);
        for (size_t p = 0; p < ids.size(); ++p) {
            if (p) out << ' ';
            out << fn(ids[p]);
        }
        close_data_array(out);
    };
    auto write_particle_vec = [&](const char* name, auto fn) {
        open_data_array(out, "Float64", name, 3);
        for (size_t p = 0; p < ids.size(); ++p) {
            Vec3 v = fn(ids[p]);
            if (p) out << ' ';
            out << v.x << ' ' << v.y << ' ' << v.z;
        }
        close_data_array(out);
    };

    write_particle_scalar("Int32", "lattice_index", [](int i) { return i; });
    write_particle_scalar("Int32", "particle_id", [&](int i) { return vox[i].particle_id; });
    write_particle_scalar("Int32", "pair_id", [&](int i) { return vox[i].pair_id; });
    write_particle_scalar("Int32", "state", [&](int i) { return static_cast<int>(vox[i].state); });
    write_particle_scalar("Int32", "spin", [&](int i) { return static_cast<int>(vox[i].spin); });
    write_particle_scalar("Int32", "color", [&](int i) { return static_cast<int>(vox[i].color); });
    write_particle_scalar("Int32", "flavor", [&](int i) { return static_cast<int>(vox[i].flavor); });
    write_particle_scalar("Float64", "density", [&](int i) { return vox[i].density(); });
    write_particle_scalar("Float64", "latency", [&](int i) { return vox[i].latency; });
    write_particle_scalar("Float64", "tau", [&](int i) { return vox[i].tau; });
    write_particle_scalar("Int32", "locked", [&](int i) { return vox[i].locked ? 1 : 0; });
    write_particle_vec("flux", [&](int i) { return vox[i].flux; });
    write_particle_vec("velocity", [&](int i) { return vox[i].velocity; });
    if (options_.export_forces) {
        write_particle_vec("force_coulomb", [&](int i) { return force[i].f_coulomb; });
        write_particle_vec("force_strong", [&](int i) { return force[i].f_strong; });
        write_particle_vec("force_magnetic", [&](int i) { return force[i].f_magnetic; });
        write_particle_vec("force_gravity", [&](int i) { return force[i].f_gravity; });
        write_particle_vec("force_exchange", [&](int i) { return force[i].f_exchange; });
    }

    out << "      </PointData>\n";
    out << "      <Points>\n";
    open_data_array(out, "Float64", "Points", 3);
    for (size_t p = 0; p < ids.size(); ++p) {
        Coord c = lat.coord(ids[p]);
        if (p) out << ' ';
        out << c.x << ' ' << c.y << ' ' << c.z;
    }
    close_data_array(out);
    out << "      </Points>\n";
    write_vtp_vertices(out, static_cast<int>(ids.size()));
    out << "    </Piece>\n";
    out << "  </PolyData>\n";
    out << "</VTKFile>\n";
    return true;
}

bool ResearchExportSession::write_cluster_frame(const RenderBridge& bridge,
                                                int frame_index,
                                                std::string& relative_file) const {
    relative_file = path_join("clusters", frame_name(frame_index, "vtp"));
    const fs::path full_path = fs::path(options_.output_dir) / fs::path(relative_file);
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    const auto histories = current_histories(cluster_tracker_, bridge.current_tick());
    const int n = static_cast<int>(histories.size());

    out << std::setprecision(17) << std::scientific;
    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <PolyData>\n";
    out << "    <Piece NumberOfPoints=\"" << n
        << "\" NumberOfVerts=\"" << n
        << "\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
    out << "      <PointData Scalars=\"cluster_size\">\n";

    auto write_cluster_scalar = [&](const char* type, const char* name, auto fn) {
        open_data_array(out, type, name);
        for (int i = 0; i < n; ++i) {
            if (i) out << ' ';
            out << fn(*histories[static_cast<size_t>(i)]);
        }
        close_data_array(out);
    };

    write_cluster_scalar("Int32", "cluster_id", [](const ClusterHistory& h) { return h.cluster_id; });
    write_cluster_scalar("Int32", "cluster_size", [](const ClusterHistory& h) { return h.snapshots.back().size; });
    write_cluster_scalar("Int32", "state_sign", [](const ClusterHistory& h) { return static_cast<int>(h.state_sign); });
    write_cluster_scalar("Int32", "birth_tick", [](const ClusterHistory& h) { return h.birth_tick; });
    write_cluster_scalar("Int32", "max_size", [](const ClusterHistory& h) { return h.max_size; });
    write_cluster_scalar("Int32", "lifetime", [](const ClusterHistory& h) { return h.lifetime(); });

    out << "      </PointData>\n";
    out << "      <Points>\n";
    open_data_array(out, "Float64", "Points", 3);
    for (int i = 0; i < n; ++i) {
        const ClusterSnapshot& s = histories[static_cast<size_t>(i)]->snapshots.back();
        if (i) out << ' ';
        out << s.centroid_x << ' ' << s.centroid_y << ' ' << s.centroid_z;
    }
    close_data_array(out);
    out << "      </Points>\n";
    write_vtp_vertices(out, n);
    out << "    </Piece>\n";
    out << "  </PolyData>\n";
    out << "</VTKFile>\n";
    return true;
}

bool ResearchExportSession::append_diagnostics(const RenderBridge& bridge, int frame_index) {
    const fs::path full_path = fs::path(options_.output_dir) / "diagnostics.csv";
    std::ofstream out(full_path, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    if (!diagnostics_header_written_) {
        out << "frame,tick,physical_time,total_flux,total_energy,manifested,positive,negative,"
               "entropy,field_energy,wave_energy,particle_ke,coulomb_pe,gauss_violation,"
               "max_gauss_error,charge_total,drift_frac,energy_residual\n";
        diagnostics_header_written_ = true;
    }

    const auto d = bridge.diagnostics();
    const auto ea = bridge.energy_audit();
    const auto& ledger = bridge.energy_ledger();
    out << std::setprecision(17) << std::scientific
        << frame_index << ','
        << d.tick << ','
        << bridge.physical_time() << ','
        << d.total_flux << ','
        << d.total_energy << ','
        << d.manifested_count << ','
        << d.positive_count << ','
        << d.negative_count << ','
        << d.total_entropy << ','
        << ea.field_energy << ','
        << ea.wave_energy << ','
        << ea.particle_ke << ','
        << ea.coulomb_pe << ','
        << ea.gauss_violation << ','
        << ea.max_gauss_error << ','
        << ea.charge_total << ','
        << ledger.drift_frac << ','
        << ledger.residual << '\n';
    return true;
}

bool ResearchExportSession::write_pvd(const std::string& relative_path,
                                       const std::string& vtk_type,
                                       const std::vector<std::string>& files) const {
    const fs::path full_path = fs::path(options_.output_dir) / fs::path(relative_path);
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <Collection>\n";
    for (size_t i = 0; i < files.size(); ++i) {
        const FrameRecord& f = frames_[i];
        out << "    <DataSet timestep=\"" << f.tick
            << "\" group=\"\" part=\"0\" file=\""
            << xml_escape(files[i]) << "\"/>\n";
    }
    out << "  </Collection>\n";
    out << "</VTKFile>\n";
    (void)vtk_type;
    return true;
}

bool ResearchExportSession::write_manifest() const {
    const fs::path full_path = fs::path(options_.output_dir) / "manifest.json";
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    out << "{\n";
    out << "  \"schema_version\": 1,\n";
    out << "  \"format\": \"VTK XML\",\n";
    out << "  \"encoding\": \"ascii\",\n";
    out << "  \"run_name\": \"" << json_escape(options_.run_name) << "\",\n";
    out << "  \"frame_count\": " << frames_.size() << ",\n";
    out << "  \"frame_interval\": " << options_.frame_interval << ",\n";
    out << "  \"spatial_stride\": " << options_.spatial_stride << ",\n";
    out << "  \"outputs\": {\n";
    out << "    \"fields\": \"fields.pvd\",\n";
    out << "    \"particles\": \"particles.pvd\",\n";
    out << "    \"clusters\": \"clusters.pvd\",\n";
    out << "    \"diagnostics\": \"diagnostics.csv\",\n";
    out << "    \"cluster_tracks\": \"cluster_tracks.csv\"\n";
    out << "  },\n";
    out << "  \"frames\": [\n";
    for (size_t i = 0; i < frames_.size(); ++i) {
        const auto& f = frames_[i];
        out << "    {\"frame\": " << f.frame
            << ", \"tick\": " << f.tick
            << ", \"physical_time\": " << std::setprecision(17) << f.time
            << ", \"fields\": \"" << json_escape(f.fields_file)
            << "\", \"particles\": \"" << json_escape(f.particles_file)
            << "\", \"clusters\": \"" << json_escape(f.clusters_file) << "\"}";
        if (i + 1 < frames_.size()) out << ',';
        out << '\n';
    }
    out << "  ]\n";
    out << "}\n";
    return true;
}

bool ResearchExportSession::write_cluster_tracks() const {
    if (!options_.export_clusters) return true;
    const fs::path full_path = fs::path(options_.output_dir) / "cluster_tracks.csv";
    std::ofstream out(full_path);
    if (!out.is_open()) {
        std::cerr << "VTK export error: cannot open " << full_path.string() << "\n";
        return false;
    }

    out << "cluster_id,birth_tick,death_tick,alive,tick,size,state_sign,"
           "centroid_x,centroid_y,centroid_z,max_size,lifetime\n";

    std::vector<const ClusterHistory*> histories;
    histories.reserve(cluster_tracker_.histories().size());
    for (const auto& kv : cluster_tracker_.histories()) histories.push_back(&kv.second);
    std::sort(histories.begin(), histories.end(), [](const ClusterHistory* a, const ClusterHistory* b) {
        return a->cluster_id < b->cluster_id;
    });

    for (const ClusterHistory* h : histories) {
        for (const ClusterSnapshot& s : h->snapshots) {
            out << h->cluster_id << ','
                << h->birth_tick << ','
                << h->death_tick << ','
                << (h->alive() ? 1 : 0) << ','
                << s.tick << ','
                << s.size << ','
                << static_cast<int>(s.state_sign) << ','
                << std::setprecision(17) << s.centroid_x << ','
                << s.centroid_y << ','
                << s.centroid_z << ','
                << h->max_size << ','
                << h->lifetime() << '\n';
        }
    }
    return true;
}

bool ResearchExportSession::finalize() {
    if (!ensure_directories()) return false;

    std::vector<std::string> field_files;
    std::vector<std::string> particle_files;
    std::vector<std::string> cluster_files;
    field_files.reserve(frames_.size());
    particle_files.reserve(frames_.size());
    cluster_files.reserve(frames_.size());
    for (const auto& f : frames_) {
        if (!f.fields_file.empty()) field_files.push_back(f.fields_file);
        if (!f.particles_file.empty()) particle_files.push_back(f.particles_file);
        if (!f.clusters_file.empty()) cluster_files.push_back(f.clusters_file);
    }

    if (options_.export_fields && !write_pvd("fields.pvd", "ImageData", field_files)) return false;
    if (options_.export_particles && !write_pvd("particles.pvd", "PolyData", particle_files)) return false;
    if (options_.export_clusters && !write_pvd("clusters.pvd", "PolyData", cluster_files)) return false;
    if (!write_cluster_tracks()) return false;
    return write_manifest();
}

bool export_research_snapshot(const RenderBridge& bridge,
                              const ExportOptions& options) {
    ResearchExportSession session(options);
    return session.record(bridge) && session.finalize();
}

}  // namespace sciviz
}  // namespace ftd
