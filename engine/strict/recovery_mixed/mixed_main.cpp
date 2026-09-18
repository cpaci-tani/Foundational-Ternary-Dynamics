// Finite intervention instrument. Never substitutes a CPU result for GPU state.
#include "campaign_scaffolding.h"
#include "staged_cuda.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace {
using namespace ftd::strict;
void snapshot(std::ostream& out, const State& state, const std::filesystem::path& dir,
              const std::string& id, bool initial) {
    const auto bytes = encode(state);
    std::ostringstream name; name << id << ".t" << std::setw(2) << std::setfill('0')
                                 << state.microtick << ".bin";
    const auto path = dir / name.str();
    if (!initial) {
        if (std::filesystem::exists(path)) throw std::runtime_error("refuse snapshot overwrite");
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
        if (!file) throw std::runtime_error("snapshot write failed");
    }
    out << "{\"microtick\":" << state.microtick << ",\"file\":\"" << name.str()
        << "\",\"sha256\":\"" << campaign::digest(bytes) << "\",\"work\":" << work_units(state) << '}';
}
}

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("usage: mixed_campaign MANIFEST_TSV TRACE_JSONL");
        const std::filesystem::path target(argv[2]), dir = target.parent_path();
        campaign::refuse_existing_trace(target, "refuse trace overwrite");
        std::ifstream manifest(argv[1]);
        if (!manifest) throw std::runtime_error("manifest unavailable");
        auto output = campaign::open_partial_trace(target, "trace unavailable");
        std::cerr << gpu::device_json() << '\n';
        unsigned cases = 0; std::string line;
        while (std::getline(manifest, line)) {
            campaign::strip_carriage_return(line);
            const auto [id, path] = campaign::split_manifest_row(line, "invalid manifest row");
            std::ostringstream expected; expected << "mixed_" << std::setw(2)
                                                 << std::setfill('0') << cases;
            if (id != expected.str() || cases >= 10) throw std::runtime_error("case order/count invalid");
            std::ifstream file(path, std::ios::binary);
            if (!file) throw std::runtime_error("preparation unavailable");
            const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)), {});
            auto state = decode(bytes);
            if (state.L != 17 || state.microtick != 0) throw std::runtime_error("wrong initial domain/clock");
            auto cpu = state;
            output << "{\"case_id\":\"" << id << "\",\"states\":[";
            snapshot(output, state, dir, id, true);
            std::vector<Events> events;
            for (unsigned tick = 0; tick < 16; ++tick) {
                auto gpu_events = gpu::advance(state, 1);
                const auto cpu_events = advance(cpu, 1);
                if (encode(cpu) != encode(state) || events_json(cpu_events) != events_json(gpu_events))
                    throw std::runtime_error("complete GPU/native state or event parity failed");
                events.push_back(std::move(gpu_events.front()));
                output << ','; snapshot(output, state, dir, id, false);
            }
            output << "],\"events\":" << events_json(events)
                   << ",\"native_complete_parity\":true}\n";
            if (!output) throw std::runtime_error("trace write failed");
            ++cases; std::cerr << "completed_cases=" << cases << '\n';
        }
        if (cases != 10) throw std::runtime_error("ten preparations required");
        campaign::publish_trace(output, target);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "mixed campaign failed: " << error.what() << '\n'; return 1;
    }
}
