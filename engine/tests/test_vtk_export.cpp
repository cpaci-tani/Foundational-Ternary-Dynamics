/**
 * Test: Native ParaView/VTK XML export.
 *
 * Validates the dependency-free research export surface without invoking
 * ParaView or VTK libraries.
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/vtk_export.h"

namespace fs = std::filesystem;

int g_failures = 0;
int g_passes = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
        ++g_passes;
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

std::string read_text(const fs::path& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int count_lines(const fs::path& path) {
    std::ifstream in(path);
    int n = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) ++n;
    }
    return n;
}

int count_occurrences(const std::string& haystack, const std::string& needle) {
    int count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

std::string data_array_payload(const std::string& xml, const std::string& name) {
    const std::string marker = "Name=\"" + name + "\"";
    size_t name_pos = xml.find(marker);
    if (name_pos == std::string::npos) return {};
    size_t start = xml.find('>', name_pos);
    if (start == std::string::npos) return {};
    size_t end = xml.find("</DataArray>", start);
    if (end == std::string::npos) return {};
    return xml.substr(start + 1, end - start - 1);
}

int count_numbers(const std::string& text) {
    std::istringstream in(text);
    std::string token;
    int n = 0;
    while (in >> token) ++n;
    return n;
}

bool payload_contains_int(const std::string& text, int target) {
    std::istringstream in(text);
    int value = 0;
    while (in >> value) {
        if (value == target) return true;
    }
    return false;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: ParaView/VTK Research Export\n";
    std::cout << "================================================================\n\n";

    const fs::path outdir = "test_vtk_export_out";
    fs::remove_all(outdir);

    ftd::RenderBridge rb(6);
    rb.force_cpu();
    rb.toggles.disable_all();

    rb.inject_particle(1, 1, 1, +1, {1.0, 0.0, 0.0}, +1, 1);
    rb.inject_particle(2, 1, 1, +1, {0.8, 0.2, 0.0}, +1, 1);
    rb.inject_particle(3, 1, 1, +1, {0.6, 0.3, 0.1}, +1, 1);
    rb.inject_particle(4, 1, 1, +1, {0.4, 0.4, 0.2}, +1, 1);

    ftd::sciviz::ExportOptions options;
    options.output_dir = outdir.string();
    options.run_name = "vtk_export_test";
    options.frame_interval = 1;
    options.spatial_stride = 1;

    ftd::sciviz::ResearchExportSession session(options);
    check("record frame 0 succeeds", session.record(rb));
    rb.tick();
    check("record frame 1 succeeds", session.record(rb));
    check("finalize succeeds", session.finalize());
    check("frame_count is 2", session.frame_count() == 2);

    const fs::path vti = outdir / "fields" / "frame_000000.vti";
    const fs::path particles = outdir / "particles" / "frame_000000.vtp";
    const fs::path clusters = outdir / "clusters" / "frame_000000.vtp";
    const fs::path fields_pvd = outdir / "fields.pvd";
    const fs::path particles_pvd = outdir / "particles.pvd";
    const fs::path clusters_pvd = outdir / "clusters.pvd";
    const fs::path manifest = outdir / "manifest.json";
    const fs::path diagnostics = outdir / "diagnostics.csv";
    const fs::path cluster_tracks = outdir / "cluster_tracks.csv";

    check("field VTI exists", fs::exists(vti));
    check("particle VTP exists", fs::exists(particles));
    check("cluster VTP exists", fs::exists(clusters));
    check("field PVD exists", fs::exists(fields_pvd));
    check("particle PVD exists", fs::exists(particles_pvd));
    check("cluster PVD exists", fs::exists(clusters_pvd));
    check("manifest exists", fs::exists(manifest));
    check("diagnostics CSV exists", fs::exists(diagnostics));
    check("cluster tracks CSV exists", fs::exists(cluster_tracks));

    const std::string vti_xml = read_text(vti);
    check("VTI is ImageData", vti_xml.find("<VTKFile type=\"ImageData\"") != std::string::npos);
    check("VTI extent matches 6^3", vti_xml.find("WholeExtent=\"0 5 0 5 0 5\"") != std::string::npos);

    const char* required_arrays[] = {
        "state", "density", "flux", "wave_vel", "velocity", "latency",
        "tau", "spin", "color", "flavor", "particle_id", "pair_id",
        "locked", "div_J", "gauss_error", "curl_J", "E", "B",
        "poynting", "force_coulomb", "flux_L", "flux_R",
        "flux_strong", "flux_weak"
    };
    bool all_arrays_present = true;
    for (const char* name : required_arrays) {
        const std::string marker = std::string("Name=\"") + name + "\"";
        all_arrays_present = all_arrays_present && (vti_xml.find(marker) != std::string::npos);
    }
    check("VTI contains required scalar/vector arrays", all_arrays_present);
    check("VTI state array has L^3 point values",
          count_numbers(data_array_payload(vti_xml, "state")) == 216);

    const std::string particle_xml = read_text(particles);
    check("particle VTP is PolyData", particle_xml.find("<VTKFile type=\"PolyData\"") != std::string::npos);
    check("particle VTP has four points", particle_xml.find("NumberOfPoints=\"4\"") != std::string::npos);
    check("particle VTP contains particle metadata",
          particle_xml.find("Name=\"particle_id\"") != std::string::npos &&
          particle_xml.find("Name=\"color\"") != std::string::npos &&
          particle_xml.find("Name=\"force_coulomb\"") != std::string::npos);

    const std::string cluster_xml = read_text(clusters);
    check("cluster VTP has one cluster point", cluster_xml.find("NumberOfPoints=\"1\"") != std::string::npos);
    check("cluster VTP contains size-4 component",
          payload_contains_int(data_array_payload(cluster_xml, "cluster_size"), 4));

    const std::string fields_pvd_xml = read_text(fields_pvd);
    const std::string particles_pvd_xml = read_text(particles_pvd);
    const std::string clusters_pvd_xml = read_text(clusters_pvd);
    check("fields PVD has two datasets", count_occurrences(fields_pvd_xml, "<DataSet ") == 2);
    check("particles PVD has two datasets", count_occurrences(particles_pvd_xml, "<DataSet ") == 2);
    check("clusters PVD has two datasets", count_occurrences(clusters_pvd_xml, "<DataSet ") == 2);

    const std::string manifest_json = read_text(manifest);
    check("manifest records VTK XML ASCII",
          manifest_json.find("\"format\": \"VTK XML\"") != std::string::npos &&
          manifest_json.find("\"encoding\": \"ascii\"") != std::string::npos);
    check("manifest records two frames", manifest_json.find("\"frame_count\": 2") != std::string::npos);

    check("diagnostics has one header plus two rows", count_lines(diagnostics) == 3);
    check("diagnostics has one header only",
          count_occurrences(read_text(diagnostics), "frame,tick,physical_time") == 1);

    const std::string tracks = read_text(cluster_tracks);
    check("cluster_tracks contains size column", tracks.find("cluster_id,birth_tick") == 0);
    check("cluster_tracks records size-4 cluster", tracks.find(",4,1,") != std::string::npos);

    fs::remove_all(outdir);

    std::cout << "\n================================================================\n";
    std::cout << "  VTK Export Tests: " << g_passes << " passed, "
              << g_failures << " failed\n";
    std::cout << "================================================================\n";

    return g_failures;
}
