#include "hydro_runtime.h"
#include "hydro_sha256.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
int selftest() {
    const unsigned char abc_bytes[3] = {'a', 'b', 'c'};
    const auto abc = ftd::hydro::sha256(abc_bytes, 3);
    const auto empty = ftd::hydro::sha256(nullptr, 0);
    const bool abc_ok = abc[0] == 0xba && abc[1] == 0x78 && abc[2] == 0x16 && abc[3] == 0xbf;
    const bool empty_ok = empty[0] == 0xe3 && empty[1] == 0xb0 && empty[2] == 0xc4 && empty[3] == 0x42;
    if (!abc_ok || !empty_ok) {
        std::cerr << "sha256 selftest FAILED\n";
        return 1;
    }
    std::cout << "ftd_hydro_cli sha256 selftest PASS\n";
    return 0;
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--selftest") return selftest();
        if (argc != 6) throw std::invalid_argument("usage: ftd_hydro_cli TABLE INPUT OUTPUT MICROTICKS EVENTS_JSON");
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
        auto events = ftd::hydro::advance(state, microticks);
        const auto encoded = ftd::hydro::encode(state);
        const auto json = ftd::hydro::events_json(events);
        // Input validation and the entire batch complete before either output is opened.
        std::ofstream output(argv[3], std::ios::binary), event_output(argv[5], std::ios::binary);
        if (!output || !event_output) throw std::runtime_error("cannot open outputs");
        output.write(reinterpret_cast<const char*>(encoded.data()), std::streamsize(encoded.size()));
        event_output << json;
        if (!output || !event_output) throw std::runtime_error("failed writing outputs");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
