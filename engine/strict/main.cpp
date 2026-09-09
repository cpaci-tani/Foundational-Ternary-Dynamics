#include "staged_runtime.h"
#include "cli_publication.h"
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
        ftd::strict::cli::validate_paths(argv[1],argv[2],argv[4]);
        std::ifstream input(argv[1],std::ios::binary);
        if(!input) throw std::runtime_error("cannot open input");
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
        auto state=ftd::strict::decode(bytes);
        auto events=ftd::strict::advance(state,ticks);
        const auto encoded=ftd::strict::encode(state);
        const auto json=ftd::strict::events_json(events);
        ftd::strict::cli::publish_pair(argv[1],argv[2],argv[4],encoded,json);
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';
        return 2;
    }
}
