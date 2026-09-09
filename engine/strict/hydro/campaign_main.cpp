// H2-prime registered response campaign instrument for phi-hydro-staged-candidate-1.
// Mirrors engine/strict/recovery_hydro/hydro_main.cpp (Phase 1) for the successor law:
// stroboscopic observation of the accepted CUDA evolution. No kernel or law changes.
#include "hydro_cuda.h"
#include "hydro_tables.h"
#include <openssl/sha.h>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace ftd::hydro;
constexpr unsigned MOMENTS = 4;  // mass, px, py, pz

std::string digest(const std::vector<std::uint8_t>& bytes) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(bytes.data(), bytes.size(), hash);
    std::ostringstream out; out << std::hex << std::setfill('0');
    for (auto byte : hash) out << std::setw(2) << unsigned(byte);
    return out.str();
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open input for hashing: " + path);
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)), {});
}

struct Case { std::string id, path; unsigned L; long kx, ky, kz; unsigned pol; };

// case_id path L kx ky kz pol (7 tab-separated fields; case_id chars mirror hydro_main.cpp's
// registered set, widened with '-' since campaign.py's case IDs use it as a separator).
Case parse(const std::string& line) {
    std::istringstream fields(line);
    Case c; std::string L, kx, ky, kz, pol;
    if (!std::getline(fields, c.id, '\t') || !std::getline(fields, c.path, '\t') || !std::getline(fields, L, '\t')
        || !std::getline(fields, kx, '\t') || !std::getline(fields, ky, '\t') || !std::getline(fields, kz, '\t')
        || !std::getline(fields, pol, '\t'))
        throw std::runtime_error("manifest line needs 7 tab-separated fields");
    if (c.id.empty() || c.id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_-") != std::string::npos)
        throw std::runtime_error("invalid registered case_id");
    c.L = unsigned(std::stoul(L)); c.kx = std::stol(kx); c.ky = std::stol(ky); c.kz = std::stol(kz);
    c.pol = unsigned(std::stoul(pol));
    if (c.pol != 0 && c.pol != 1) throw std::runtime_error("polarity must be 0 or 1");
    return c;
}

// Fields only; the caller frames the JSON object. `moments` aggregates the case's
// registered polarity's occupancy over the four passive phases into 24 velocity slots per
// site (at most one phase is ever set per (site, pol, velocity): staged.py's exclusion
// invariant, enforced by validate()), Fourier-projects that onto the registered
// wavevector; `total_momentum`/`mass` are exact integer sums over BOTH polarities and all
// four phases (the entire 192-channel bank) -- an arm-independent conservation receipt,
// distinct from the arm-specific `moments` observable.
void observe(std::ostream& out, const State& st, const Case& c) {
    const auto L = st.L; const std::size_t n = st.s.size();
    double re[MOMENTS] = {0, 0, 0, 0}, im[MOMENTS] = {0, 0, 0, 0};
    long long mass = 0, momentum[3] = {0, 0, 0};
    const double two_pi = 2.0 * std::acos(-1.0);
    const double factor = two_pi / double(L);
    for (std::size_t i = 0; i < n; ++i) {
        const std::uint8_t* bank = st.bank.data() + i * tables::N_CHANNELS;
        long site[MOMENTS] = {0, 0, 0, 0};
        for (unsigned v = 0; v < tables::N_VEL; ++v) {
            bool occupied_here = false;
            for (unsigned k = 0; k < tables::N_PHASE; ++k)
                if (bank[c.pol * 96 + k * tables::N_VEL + v]) occupied_here = true;
            if (occupied_here) {
                site[0] += 1;
                for (unsigned a = 0; a < 3; ++a) site[a + 1] += tables::VELOCITY[v][a];
            }
            for (unsigned pol2 = 0; pol2 < 2; ++pol2)
                for (unsigned k = 0; k < tables::N_PHASE; ++k)
                    if (bank[pol2 * 96 + k * tables::N_VEL + v]) {
                        mass += 1;
                        for (unsigned a = 0; a < 3; ++a) momentum[a] += tables::VELOCITY[v][a];
                    }
        }
        const long x = long(i / (std::size_t(L) * L)), y = long((i / L) % L), z = long(i % L);
        const double phase = factor * double(c.kx * x + c.ky * y + c.kz * z);
        const double cs = std::cos(phase), sn = std::sin(phase);  // e^{-ik.x} = cos(phase) - i*sin(phase)
        for (unsigned a = 0; a < MOMENTS; ++a) { re[a] += double(site[a]) * cs; im[a] += -double(site[a]) * sn; }
    }
    out << std::setprecision(17);
    out << "\"moments\":[";
    for (unsigned a = 0; a < MOMENTS; ++a) out << (a ? "," : "") << '[' << re[a] << ',' << im[a] << ']';
    out << "],\"total_momentum\":[" << momentum[0] << ',' << momentum[1] << ',' << momentum[2]
        << "],\"mass\":" << mass;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 5) throw std::runtime_error("usage: ftd_hydro_campaign TABLE MANIFEST_TSV TRACE_JSONL STAGES");
        const std::string count = argv[4];
        if (count.empty() || count.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("invalid stage count");
        const auto stages = std::stoull(count);
        ftd::hydro::load_table(argv[1]);
        const auto table_sha256 = digest(read_file(argv[1]));
        const auto manifest_bytes = read_file(argv[2]);
        const auto manifest_sha256 = digest(manifest_bytes);
        std::istringstream manifest(std::string(manifest_bytes.begin(), manifest_bytes.end()));
        std::filesystem::path target(argv[3]), partial = target.string() + ".part";
        if (std::filesystem::exists(target)) throw std::runtime_error("refuse to overwrite completed campaign trace");
        std::ofstream output(partial); if (!output) throw std::runtime_error("cannot open trace");
        std::cerr << ftd::hydro::gpu::device_json() << '\n';
        output << "{\"header\":true,\"table_path\":\"" << argv[1] << "\",\"table_sha256\":\"" << table_sha256
               << "\",\"manifest_path\":\"" << argv[2] << "\",\"manifest_sha256\":\"" << manifest_sha256
               << "\",\"stages\":" << stages << "}\n";
        if (!output) throw std::runtime_error("trace header write failed");
        unsigned completed = 0; std::string line;
        while (std::getline(manifest, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            const Case c = parse(line);
            std::ifstream input(c.path, std::ios::binary);
            if (!input) throw std::runtime_error("cannot read preparation: " + c.path);
            std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
            auto state = ftd::hydro::decode(bytes);
            if (state.L != c.L || state.microtick != 0)
                throw std::runtime_error("unregistered domain or preparation tick: " + c.id);
            const auto work = ftd::hydro::work_units(state);
            for (std::uint64_t s = 0; s <= stages; ++s) {
                if (s) {
                    ftd::hydro::gpu::advance(state, 4);
                    if (ftd::hydro::work_units(state) != work)
                        throw std::runtime_error("work unit drift across a stage: " + c.id);
                }
                output << "{\"case_id\":\"" << c.id << "\",\"stage\":" << s << ',';
                observe(output, state, c);
                output << "}\n";
                if (!output) throw std::runtime_error("trace write failed");
            }
            std::cerr << "completed_cases=" << ++completed << '\n';
        }
        if (!completed) throw std::runtime_error("empty manifest");
        output.close(); if (!output) throw std::runtime_error("trace close failed");
        std::filesystem::rename(partial, target);
        std::cerr << "completed_cases=" << completed << " stages=" << stages << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "hydro campaign failed: " << e.what() << '\n'; return 1; }
}
