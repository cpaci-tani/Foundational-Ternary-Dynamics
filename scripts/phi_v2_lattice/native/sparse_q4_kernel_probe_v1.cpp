#include "sparse_q4_kernel_v1.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace Q=ftd::q4native_v1;
namespace {
std::uint64_t read_le(unsigned n) {
    std::uint64_t out=0;
    for(unsigned i=0;i<n;++i) {
        const int c=std::cin.get();
        if(c==std::char_traits<char>::eof()) throw std::runtime_error("truncated stream integer");
        out|=std::uint64_t(static_cast<unsigned char>(c))<<(8*i);
    }
    return out;
}
void write_le(std::uint64_t x,unsigned n) {
    for(unsigned i=0;i<n;++i) {std::cout.put(static_cast<char>(x&255));x>>=8;}
}
void bytes(const std::vector<std::uint8_t>&x) {
    write_le(x.size(),8);
    std::cout.write(reinterpret_cast<const char*>(x.data()),static_cast<std::streamsize>(x.size()));
}
void rejected(const char*message) {
    std::cout.put(1);const std::string text(message);
    write_le(text.size(),8);std::cout.write(text.data(),static_cast<std::streamsize>(text.size()));
}
}
int main(int argc,char**argv) {
#ifdef _WIN32
    if(_setmode(_fileno(stdin),_O_BINARY)==-1 || _setmode(_fileno(stdout),_O_BINARY)==-1) {
        std::cerr<<"binary standard stream setup failed\n";return 2;
    }
#endif
    try {
        if(argc==2 && std::string(argv[1])=="--identity") {
            std::cout<<"{\"schema\":\"strict-sparse-q4-native-identity-1\",\"law_id\":\""<<Q::LAW_ID
                     <<"\",\"rule_hash\":\""<<Q::RULE_HASH<<"\",\"frame_hash\":\""<<Q::FRAME_HASH
                     <<"\",\"dense_encoding\":\""<<Q::DENSE_ENCODING<<"\",\"backend_id\":\""<<Q::BACKEND_ID
                     <<"\",\"wire_id\":\""<<Q::WIRE_ID<<"\",\"supported_even_L\":[4,64],"
                     <<"\"ordinal\":\"owned canonical hexadecimal; exact increment and modulo19\","
                     <<"\"interface_version\":1,\"payload_size\":"<<sizeof(Q::Payload)
                     <<",\"events_size\":"<<sizeof(Q::Events)<<",\"canonical_adoption\":false}\n";
            return std::cout?0:2;
        }
        if(argc!=2 || std::string(argv[1])!="--step-stream") throw std::runtime_error("expected --identity or --step-stream");
        const auto count=read_le(4);
        if(count>4096) throw std::runtime_error("request count exceeds4096");
        for(std::uint64_t i=0;i<count;++i) {
            const auto length=read_le(8);
            if(length>(1U<<20)) throw std::runtime_error("request exceeds1MiB transport envelope");
            std::vector<std::uint8_t> data(static_cast<std::size_t>(length));
            if(length) std::cin.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(length));
            if(!std::cin) throw std::runtime_error("truncated request frame");
            try {
                const auto input=Q::decode_state(data.data(),data.size());
                if(Q::encode_state(input)!=data) throw std::runtime_error("noncanonical decoded state");
                const auto before=Q::encode_state(input);
                const auto output=Q::step(input);
                if(Q::encode_state(input)!=before) throw std::runtime_error("input mutated");
                const auto state_wire=Q::encode_state(output.state),event_wire=Q::encode_events(output.events);
                const auto state_again=Q::decode_state(state_wire.data(),state_wire.size());
                const auto events_again=Q::decode_events(event_wire.data(),event_wire.size());
                if(Q::encode_state(state_again)!=state_wire || Q::encode_events(events_again)!=event_wire)
                    throw std::runtime_error("successor/event codec roundtrip");
                // Complete both encoded results before emitting success.
                std::cout.put(0);bytes(state_wire);bytes(event_wire);
            } catch(const Q::Error&e) {rejected(e.what());}
              catch(const std::bad_alloc&) {rejected("ALLOCATION_FAILURE: probe buffer");}
            std::cout.flush();
            if(!std::cout) throw std::runtime_error("output stream failure");
        }
        if(std::cin.peek()!=std::char_traits<char>::eof()) throw std::runtime_error("trailing stream bytes");
        return 0;
    } catch(const std::exception&e) {
        std::cerr<<"FAILED_STREAM: "<<e.what()<<'\n';return 2;
    }
}
