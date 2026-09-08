// Registered observations of accepted CUDA evolution. No kernel/law changes.
#include "staged_cuda.h"
#include "frozen_tables.h"
#include <openssl/sha.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace {
using namespace ftd::strict;
std::string digest(const std::vector<std::uint8_t>& bytes) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(bytes.data(),bytes.size(),hash);
    std::ostringstream out;out<<std::hex<<std::setfill('0');
    for(auto byte:hash)out<<std::setw(2)<<unsigned(byte);
    return out.str();
}
void observe(std::ostream& out,const State& st) {
    out<<"{\"microtick\":\""<<st.microtick<<"\",\"sha256\":\""<<digest(encode(st))
       <<"\",\"L\":"<<st.L<<",\"work\":"<<work_units(st)<<",\"field\":[";
    bool first=true;
    for(std::size_t x=0;x<st.s.size();++x)for(unsigned c=0;c<384;++c)if(st.bank[x*384+c]) {
        if(!first)out<<',';first=false;
        out<<'['<<x<<','<<c<<','<<unsigned(st.ell[x])<<','<<int(st.s[x])<<']';
    }
    out<<"],\"relation\":[";first=true;
    for(unsigned kind=0;kind<2;++kind) {
        const auto& records=kind?st.fcc:st.sc;
        const auto& gates=kind?st.gate_fcc:st.gate_sc;
        unsigned orientations=kind?6:3;
        for(std::size_t x=0;x<st.s.size();++x)for(unsigned ori=0;ori<orientations;++ori)
            for(unsigned slot=0;slot<2;++slot) {
                auto token=records[(x*orientations+ori)*2+slot];
                if(token==frozen::BLANK)continue;
                if(!first)out<<',';first=false;
                out<<"[\""<<(kind?"fcc":"sc")<<"\","<<x<<','<<ori<<','<<slot<<','<<unsigned(token)
                   <<','<<unsigned(gates[x*orientations+ori])<<','<<(kind?0:unsigned(st.admitted_sc[x*3+ori]))<<']';
            }
    }
    out<<"]}";
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc!=4)throw std::runtime_error("usage: carrier_campaign MANIFEST_TSV TRACE_JSONL TICKS");
        std::string count=argv[3];
        if(count.empty()||count.find_first_not_of("0123456789")!=std::string::npos)
            throw std::runtime_error("invalid tick count");
        auto ticks=std::stoull(count);
        if(ticks!=64)throw std::runtime_error("registered carrier horizon is exactly64 physical ticks");
        std::ifstream manifest(argv[1]);if(!manifest)throw std::runtime_error("cannot open manifest");
        std::filesystem::path target(argv[2]),partial=target.string()+".part";
        if(std::filesystem::exists(target))throw std::runtime_error("refuse to overwrite completed campaign trace");
        std::ofstream output(partial);if(!output)throw std::runtime_error("cannot open trace");
        std::cerr<<gpu::device_json()<<'\n';
        unsigned cases=0;std::string line;
        while(std::getline(manifest,line)) {
            if(!line.empty()&&line.back()=='\r')line.pop_back();
            auto separator=line.find('\t');
            if(separator==std::string::npos)throw std::runtime_error("manifest must contain caseID<TAB>binary path");
            const auto id=line.substr(0,separator),path=line.substr(separator+1);
            if(id.empty()||id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_")!=std::string::npos)
                throw std::runtime_error("invalid registered caseID");
            std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("cannot read preparation");
            std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
            auto state=decode(bytes);
            if(state.L!=9||state.microtick!=0)throw std::runtime_error("unregistered domain/preparation tick");
            output<<"{\"case_id\":\""<<id<<"\",\"states\":[";observe(output,state);
            std::vector<Events> events;
            for(std::uint64_t tick=0;tick<ticks;++tick) {
                auto rows=gpu::advance(state,1);events.push_back(std::move(rows.front()));
                output<<',';observe(output,state);
            }
            output<<"],\"events\":"<<events_json(events)<<"}\n";
            if(!output)throw std::runtime_error("trace write failed");
            auto final_path=std::filesystem::path(path);final_path.replace_extension(".final.bin");
            std::ofstream final(final_path,std::ios::binary);auto final_bytes=encode(state);
            final.write(reinterpret_cast<const char*>(final_bytes.data()),std::streamsize(final_bytes.size()));
            if(!final)throw std::runtime_error("endpoint snapshot write failed");
            if(++cases%50==0)std::cerr<<"completed_cases="<<cases<<'\n';
        }
        if(cases!=600)throw std::runtime_error("registered matrix must contain exactly600 cases");
        output.close();if(!output)throw std::runtime_error("trace close failed");
        std::filesystem::rename(partial,target);
        std::cerr<<"completed_cases="<<cases<<" microticks="<<cases*ticks<<'\n';
        return 0;
    } catch(const std::exception& e) {std::cerr<<"carrier campaign failed: "<<e.what()<<'\n';return 1;}
}
