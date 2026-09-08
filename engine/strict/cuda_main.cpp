#include "staged_cuda.h"
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
        std::ifstream input(argv[1],std::ios::binary);
        if(!input)throw std::runtime_error("cannot open input");
        std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
        auto state=ftd::strict::decode(bytes);
        auto events=ftd::strict::gpu::advance(state,ticks);
        auto result=ftd::strict::encode(state);
        auto journal=ftd::strict::events_json(events);
        // Output files are opened only after the complete computation succeeds.
        std::ofstream output(argv[2],std::ios::binary);
        if(!output)throw std::runtime_error("cannot open output");
        output.write(reinterpret_cast<const char*>(result.data()),std::streamsize(result.size()));
        if(!output)throw std::runtime_error("cannot write output");
        std::ofstream log(argv[4]);
        if(!log)throw std::runtime_error("cannot open events output");
        log<<journal<<'\n';
        if(!log)throw std::runtime_error("cannot write events output");
        std::cerr<<ftd::strict::gpu::device_json()<<'\n';
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"strict CUDA error: "<<error.what()<<'\n';return 1;
    }
}
