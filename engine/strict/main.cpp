#include "staged_runtime.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

int main(int argc,char** argv) {
    try {
        if(argc!=5) throw std::invalid_argument("usage: ftd_strict_cli INPUT OUTPUT TICKS EVENTS_JSON");
        std::uint64_t ticks=0;
        const std::string text=argv[3];
        if(text.empty()) throw std::invalid_argument("empty tick count");
        for(char c:text) {
            if(c<'0'||c>'9'||ticks>(std::numeric_limits<std::uint64_t>::max()-unsigned(c-'0'))/10)
                throw std::invalid_argument("ticks must be unsigned decimal uint64");
            ticks=ticks*10+unsigned(c-'0');
        }
        std::ifstream input(argv[1],std::ios::binary);
        if(!input) throw std::runtime_error("cannot open input");
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
        auto state=ftd::strict::decode(bytes);
        auto events=ftd::strict::advance(state,ticks);
        const auto encoded=ftd::strict::encode(state);
        const auto json=ftd::strict::events_json(events);
        // Input validation and the entire batch complete before either output is opened.
        std::ofstream output(argv[2],std::ios::binary), event_output(argv[4],std::ios::binary);
        if(!output||!event_output) throw std::runtime_error("cannot open outputs");
        output.write(reinterpret_cast<const char*>(encoded.data()),std::streamsize(encoded.size()));
        event_output<<json;
        if(!output||!event_output) throw std::runtime_error("failed writing outputs");
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';
        return 2;
    }
}
