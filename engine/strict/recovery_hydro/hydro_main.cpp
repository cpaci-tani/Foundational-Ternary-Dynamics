// Registered stroboscopic observation of the accepted CUDA evolution. No kernel or law changes.
#include "staged_cuda.h"
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
using namespace ftd::strict;
constexpr unsigned MODES = 7, CHANNELS = 192, PERIOD = 48;

std::string digest(const std::vector<std::uint8_t>& bytes) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(bytes.data(), bytes.size(), hash);
    std::ostringstream out; out << std::hex << std::setfill('0');
    for (auto byte : hash) out << std::setw(2) << unsigned(byte);
    return out.str();
}

struct Case { std::string id, path; unsigned L; long mx, my, mz; unsigned offset; };

std::vector<std::vector<long>> read_weights(const std::string& path) {
    std::ifstream in(path); if (!in) throw std::runtime_error("cannot open weights");
    std::vector<std::vector<long>> weights; std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::vector<long> row; std::istringstream fields(line); long v;
        while (fields >> v) row.push_back(v);
        if (row.size() != CHANNELS) throw std::runtime_error("weight row must have 192 integers");
        weights.push_back(row);
    }
    if (weights.size() != MODES) throw std::runtime_error("exactly 7 weight rows required");
    return weights;
}

// Prints the observation FIELDS only (no surrounding braces); the caller frames the object.
void observe(std::ostream& out, const State& st, const Case& c, const std::vector<std::vector<long>>& w) {
    const auto L = st.L; const std::size_t n = st.s.size();
    double re[MODES] = {0}, im[MODES] = {0}; std::uint64_t population = 0;
    const double two_pi_over_L = 2.0 * M_PI / double(L);
    for (std::size_t i = 0; i < n; ++i) {
        long site[MODES] = {0}; bool any = false;
        const std::uint8_t* bank = st.bank.data() + i * 384 + c.offset;
        for (unsigned ch = 0; ch < CHANNELS; ++ch) if (bank[ch]) {
            any = true; ++population;
            for (unsigned a = 0; a < MODES; ++a) site[a] += w[a][ch];
        }
        if (!any) continue;
        const long x = long(i / (std::size_t(L) * L)), y = long((i / L) % L), z = long(i % L);
        const double theta = -two_pi_over_L * double(c.mx * x + c.my * y + c.mz * z);
        const double cs = std::cos(theta), sn = std::sin(theta);
        for (unsigned a = 0; a < MODES; ++a) { re[a] += site[a] * cs; im[a] += site[a] * sn; }
    }
    out << "\"microtick\":\"" << st.microtick << "\",\"sha256\":\"" << digest(encode(st))
        << "\",\"population\":" << population << ",\"moments\":[";
    out << std::setprecision(17);
    for (unsigned a = 0; a < MODES; ++a) out << (a ? "," : "") << '[' << re[a] << ',' << im[a] << ']';
    out << "]";
}

Case parse(const std::string& line) {
    std::istringstream fields(line); Case c; std::string L, mx, my, mz, off;
    if (!std::getline(fields, c.id, '\t') || !std::getline(fields, c.path, '\t') || !std::getline(fields, L, '\t')
        || !std::getline(fields, mx, '\t') || !std::getline(fields, my, '\t') || !std::getline(fields, mz, '\t')
        || !std::getline(fields, off, '\t')) throw std::runtime_error("manifest line needs 7 tab-separated fields");
    if (c.id.empty() || c.id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos)
        throw std::runtime_error("invalid registered caseID");
    c.L = unsigned(std::stoul(L)); c.mx = std::stol(mx); c.my = std::stol(my); c.mz = std::stol(mz);
    c.offset = unsigned(std::stoul(off));
    if (c.offset != 0 && c.offset != 192) throw std::runtime_error("polarity offset must be 0 or 192");
    return c;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 5) throw std::runtime_error("usage: hydro_campaign MANIFEST_TSV TRACE_JSONL WEIGHTS_TSV STROBOSCOPES");
        std::string count = argv[4];
        if (count.empty() || count.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("invalid stroboscope count");
        const auto stroboscopes = std::stoull(count);
        const auto weights = read_weights(argv[3]);
        std::ifstream manifest(argv[1]); if (!manifest) throw std::runtime_error("cannot open manifest");
        std::filesystem::path target(argv[2]), partial = target.string() + ".part";
        if (std::filesystem::exists(target)) throw std::runtime_error("refuse to overwrite completed campaign trace");
        std::ofstream output(partial); if (!output) throw std::runtime_error("cannot open trace");
        std::cerr << gpu::device_json() << '\n';
        unsigned cases = 0; std::string line;
        while (std::getline(manifest, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            const Case c = parse(line);
            std::ifstream input(c.path, std::ios::binary); if (!input) throw std::runtime_error("cannot read preparation");
            std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
            auto state = decode(bytes);
            if (state.L != c.L || state.microtick != 0) throw std::runtime_error("unregistered domain or preparation tick");
            const auto work = work_units(state);
            output << "{\"case_id\":\"" << c.id << "\",\"L\":" << c.L << ",\"k\":[" << c.mx << ',' << c.my << ',' << c.mz
                   << "],\"polarity_offset\":" << c.offset << ",\"stroboscopes\":[{\"n\":0,";
            observe(output, state, c, weights);
            output << '}';
            for (std::uint64_t s = 1; s <= stroboscopes; ++s) {
                gpu::advance(state, PERIOD);   // events are computed by the accepted backend and discarded here
                if (work_units(state) != work) throw std::runtime_error("work unit drift across a period");
                output << ",{\"n\":" << s << ',';
                observe(output, state, c, weights);
                output << '}';
            }
            output << "]}\n";
            if (!output) throw std::runtime_error("trace write failed");
            auto final_path = std::filesystem::path(c.path); final_path.replace_extension(".final.bin");
            std::ofstream final(final_path, std::ios::binary); auto final_bytes = encode(state);
            final.write(reinterpret_cast<const char*>(final_bytes.data()), std::streamsize(final_bytes.size()));
            if (!final) throw std::runtime_error("endpoint snapshot write failed");
            std::cerr << "completed_cases=" << ++cases << '\n';
        }
        if (!cases) throw std::runtime_error("empty manifest");
        output.close(); if (!output) throw std::runtime_error("trace close failed");
        std::filesystem::rename(partial, target);
        std::cerr << "completed_cases=" << cases << " microticks=" << cases * stroboscopes * PERIOD << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "hydro campaign failed: " << e.what() << '\n'; return 1; }
}
