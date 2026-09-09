#include "staged_cuda.h"
#include "cli_publication.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int argc,char** argv) {
    try {
        if(argc==2 && std::string(argv[1])=="--device") {
            std::cout<<ftd::strict::gpu::device_json()<<'\n';return 0;
        }
        if(argc!=5)throw std::runtime_error("usage: ftd_strict_cuda_cli INPUT OUTPUT TICKS EVENTS_JSON (or --device)");
        std::string count=argv[3];
        if(count.empty()||count.find_first_not_of("0123456789")!=std::string::npos)
            throw std::runtime_error("TICKS must be an unsigned integer");
        auto ticks=std::stoull(count);
        ftd::strict::cli::validate_paths(argv[1],argv[2],argv[4]);
        std::ifstream input(argv[1],std::ios::binary);
        if(!input)throw std::runtime_error("cannot open input");
        std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
        auto state=ftd::strict::decode(bytes);
        auto events=ftd::strict::gpu::advance(state,ticks);
        auto result=ftd::strict::encode(state);
        auto journal=ftd::strict::events_json(events);
        const auto device = ftd::strict::gpu::device_json();
        ftd::strict::cli::publish_pair(argv[1],argv[2],argv[4],result,journal+'\n');
        std::cerr<<device<<'\n';
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"strict CUDA error: "<<error.what()<<'\n';return 1;
    }
}
