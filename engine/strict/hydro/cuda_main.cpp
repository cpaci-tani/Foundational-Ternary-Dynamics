#include "hydro_cuda.h"
#include "hydro_runtime.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--device") {
            std::cout << ftd::hydro::gpu::device_json() << '\n';
            return 0;
        }
        if (argc != 6)
            throw std::invalid_argument("usage: ftd_hydro_cuda_cli TABLE INPUT OUTPUT MICROTICKS EVENTS_JSON (or --device)");
        std::uint64_t microticks = 0;
        const std::string text = argv[4];
        if (text.empty()) throw std::invalid_argument("empty microtick count");
        for (char c : text) {
            if (c < '0' || c > '9' || microticks > (std::numeric_limits<std::uint64_t>::max() - unsigned(c - '0')) / 10)
                throw std::invalid_argument("microticks must be unsigned decimal uint64");
            microticks = microticks * 10 + unsigned(c - '0');
        }
        ftd::hydro::load_table(argv[1]);
        std::ifstream input(argv[2], std::ios::binary);
        if (!input) throw std::runtime_error("cannot open input");
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
        auto state = ftd::hydro::decode(bytes);
        auto events = ftd::hydro::gpu::advance(state, microticks);
        const auto encoded = ftd::hydro::encode(state);
        const auto json = ftd::hydro::events_json(events);
        // Input validation and the entire batch complete before either output is opened.
        std::ofstream output(argv[3], std::ios::binary), event_output(argv[5], std::ios::binary);
        if (!output || !event_output) throw std::runtime_error("cannot open outputs");
        output.write(reinterpret_cast<const char*>(encoded.data()), std::streamsize(encoded.size()));
        event_output << json;
        if (!output || !event_output) throw std::runtime_error("failed writing outputs");
        std::cerr << ftd::hydro::gpu::device_json() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
