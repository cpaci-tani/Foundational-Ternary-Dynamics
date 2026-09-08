#include "ftd/ws_command_validation.h"
#include "../src/ws_server_internal.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int failures=0, checks=0;
void check(bool ok,const std::string& label) { ++checks; if (!ok) { ++failures; std::cerr<<"FAIL "<<label<<'\n'; } }
struct SocketPair {
    SOCKET server=INVALID_SOCKET, client=INVALID_SOCKET;
    SocketPair() {
        SOCKET listener=::socket(AF_INET,SOCK_STREAM,0);
        if (listener==INVALID_SOCKET) throw std::runtime_error("socket");
        sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        if (::bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) || ::listen(listener,1)) throw std::runtime_error("bind/listen");
#ifdef _WIN32
        int length=sizeof(address);
#else
        socklen_t length=sizeof(address);
#endif
        if (::getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length)) throw std::runtime_error("getsockname");
        client=::socket(AF_INET,SOCK_STREAM,0);
        if (::connect(client,reinterpret_cast<sockaddr*>(&address),sizeof(address))) throw std::runtime_error("connect");
        server=::accept(listener,nullptr,nullptr); closesocket(listener);
        if (server==INVALID_SOCKET) throw std::runtime_error("accept");
#ifdef _WIN32
        DWORD timeout=2000; ::setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
#else
        timeval timeout{2,0}; ::setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
#endif
    }
    ~SocketPair() { closesocket(server); closesocket(client); }
    std::string response() {
        unsigned char h[2]; if (!ftd::recv_exact(client,h,2)) throw std::runtime_error("missing reply");
        if (h[0]!=0x81 || (h[1]&128)) throw std::runtime_error("invalid server text frame");
        std::size_t count=h[1]&127;
        if (count==127) throw std::runtime_error("unexpected 64-bit reply size");
        if (count==126) { unsigned char n[2]; if(!ftd::recv_exact(client,n,2)) throw std::runtime_error("short size"); count=(n[0]<<8)|n[1]; }
        if (count>=65536) throw std::runtime_error("unexpected reply size");
        std::string out(count,'\0'); if(!ftd::recv_exact(client,out.data(),count)) throw std::runtime_error("short reply"); return out;
    }
};
}
int main() {
    using namespace ftd;
#ifdef _WIN32
    _putenv_s("FTD_FORCE_CPU","1"); WSADATA wsa{}; if(WSAStartup(MAKEWORD(2,2),&wsa)) return 2;
#else
    setenv("FTD_FORCE_CPU","1",1);
#endif
    const std::vector<std::string> valid{
        R"({"cmd":"tick"})",R"({"cmd":"run"})",R"({"cmd":"run","n":2.0})",
        R"({"cmd":"set_toggle","name":"wave_propagation","value":false})",
        R"({"cmd":"set_flux_boundary","mode":1})",R"({"cmd":"set_flux_periodic_axis","axis":3})",
        R"({"cmd":"set_param","name":"langevin_seed","value":4294967295})",
        R"({"cmd":"apply_profile","name":"","applyProfile":true,"toggle_wave_propagation":true,"toggle_reflective_boundary":false,"fluxBoundaryMode":0,"fluxPeriodicAxis":3})",
        R"({"cmd":"setup_scenario","name":"empty"})",
        R"({"cmd":"resize_scenario","size":33,"name":"empty","applyProfile":true,"fluxBoundaryMode":0})",
        R"({"cmd":"get_field_sample","kind":"electric","stride":2147483647,"token":4294967295})",
        R"({"cmd":"get_field_slices","kind":"flux","stride":0,"mid":-1})",
        R"({"cmd":"get_flux_volume"})",R"({"cmd":"get_flux_slice","axis":2,"index":-1})",
        R"({"cmd":"inspect_voxel","x":2147483647,"y":-2147483648,"z":0})",
        R"({"cmd":"inject_particle","x":0,"y":1,"z":2,"state":-1,"fx":0,"fy":0,"fz":0})",
        R"({"cmd":"inject_wavepacket","x":0,"y":1,"z":2,"state":1})",
        R"({"cmd":"set_telemetry_demand","mask":3,"diagnostics":true,"audit":true,"gravity":false,"lagrangian":false,"everyTicks":{"audit":6,"lagrangian":8}})",
        R"({"cmd":"info","_requestId":9007199254740991})"
    };
    for (const auto& raw:valid) { try { validate_ws_command(parse_json_object(raw)); check(true,"valid caller schema"); } catch(const std::exception& e) { check(false,raw+e.what()); } }
    const std::vector<std::string> invalid{
        R"({"cmd":"set_toggle","name":"wave_propagation","value":0,"other":true})",
        R"({"cmd":"set_toggle","name":"wave_propagation","value":null})",
        R"({"cmd":"set_toggle","name":"wave_propagation"})",
        R"({"cmd":"tick","cmd":"reset"})",R"({"cmd":"tick","\u0063md":"reset"})",
        R"({"nested":{"cmd":"tick"}})",R"({"cmd":"tick"}x)",R"({"cmd":"tick","unused":true})",
        R"({"cmd":"set_flux_boundary","mode":1.5})",R"({"cmd":"set_flux_boundary","mode":1.00000000000000000001})",
        R"({"cmd":"set_flux_periodic_axis","axis":4})",
        R"({"cmd":"run","n":null})",R"({"cmd":"run","n":1e100})",
        R"({"cmd":"tick","_requestId":1e100})",R"({"cmd":"tick","_requestId":9007199254740990.9})",
        R"({"cmd":"tick","_requestId":9007199254740992})",R"({"cmd":"tick","_requestId":0})",
        R"({"cmd":"inspect_voxel","x":2147483648,"y":0,"z":0})",
        R"({"cmd":"get_force_at","x":0,"y":-2147483649,"z":0})",
        R"({"cmd":"get_flux_slice","axis":0.5,"index":0})",
        R"({"cmd":"get_flux_volume","axisSamples":1e100})",
        R"({"cmd":"get_field_sample","kind":"flux","token":4294967296})",
        R"({"cmd":"get_field_sample","kind":"flux","stride":0.5})",
        R"({"cmd":"get_field_slices","kind":"flux","mid":1e100})",
        R"({"cmd":"set_param","name":"langevin_seed","value":4294967296})",
        R"({"cmd":"set_param","name":"langevin_seed","value":1.5})",
        R"({"cmd":"set_param","name":"omega0","value":"2"})",
        R"({"cmd":"inject_particle","x":0,"y":0,"z":0,"state":256,"fx":0,"fy":0,"fz":0})",
        R"({"cmd":"inject_wavepacket","x":0,"y":0,"z":0,"state":0.5})",
        R"({"cmd":"inject_flux","x":0,"y":0,"z":0,"fx":1e999,"fy":0,"fz":0})",
        R"({"cmd":"inject_wave_vel_add","x":0,"y":0,"z":0,"wx":0,"wy":0,"wz":true})",
        R"({"cmd":"create_pair","x":0,"y":0,"z":0,"fx":0,"fy":0})",
        R"({"cmd":"resize","size":1e100})",R"({"cmd":"resize_scenario","size":9.5,"name":"empty"})",
        R"({"cmd":"preflight_resize","size":null})",
        R"({"cmd":"setup_scenario","name":"empty","applyProfile":false,"toggle_wave_propagation":true})",
        R"({"cmd":"apply_profile","toggle_wave_propagation":false,"toggle_damping":3})",
        R"({"cmd":"apply_profile","toggle_wave_propagation":false,"fluxBoundaryMode":1.5})",
        R"({"cmd":"apply_profile","toggle_reflective_boundary":true,"fluxBoundaryMode":0})",
        R"({"cmd":"apply_profile","toggle_not_a_term":true})",
        R"({"cmd":"set_telemetry_demand","mask":1,"diagnostics":false})",
        R"({"cmd":"set_telemetry_demand","audit":false,"everyTicks":{"audit":1.5}})",
        R"({"cmd":"set_telemetry_demand","audit":false,"everyTicks":null})",
        R"({"cmd":"set_telemetry_demand","audit":false,"everyTicks":{"audit":2,"audit":3}})",
        R"({"cmd":"get_telemetry","audit":1})",R"({"cmd":"invalid\u0000\ncommand"})"
    };
    int size=4; auto bridge=std::make_unique<RenderBridge>(size); bridge->force_cpu();
    NativeTelemetryScheduler telemetry;
    bridge->inject_flux(0,0,0,{0.25,0.5,0.75});
    DynamicalStateDigest initial{}; check(bridge->capture_dynamical_state_digest(initial),"initial CPU digest");
    const auto initial_toggles=bridge->toggles; const auto initial_seed=bridge->toggles.langevin_seed;
    const auto initial_epoch=telemetry.epoch(); const auto initial_source=telemetry.source_epoch();
    const auto initial_snapshot=telemetry.snapshot_version(); const auto initial_demand=telemetry.demand();
    auto* original=bridge.get(); SocketPair sockets;
    for(const auto& raw:invalid) {
        bool rejected=false; try {validate_ws_command(parse_json_object(raw));} catch(const std::invalid_argument&) {rejected=true;}
        check(rejected,"schema rejects "+raw);
        check(ws_server_detail::handle_command(raw,sockets.server,bridge,telemetry,size),"error reply succeeds");
        const auto reply=parse_json_object(sockets.response()); check(reply.has("error"),"actual dispatcher error is valid JSON");
        DynamicalStateDigest after{}; bridge->capture_dynamical_state_digest(after);
        check(bridge.get()==original && size==4 && bridge->current_tick()==0
              && initial.hash_lo==after.hash_lo && initial.hash_hi==after.hash_hi
              && initial.state_version==after.state_version,"rejection precedes engine mutation");
        bool same=true; for(const auto& spec:TOGGLE_SPECS) same &= (bridge->toggles.*spec.field)==(initial_toggles.*spec.field);
        check(same && bridge->toggles.langevin_seed==initial_seed,"rejection preserves profile and seed");
        check(telemetry.epoch()==initial_epoch && telemetry.source_epoch()==initial_source
              && telemetry.snapshot_version()==initial_snapshot
              && telemetry.demand().enabled_mask==initial_demand.enabled_mask
              && telemetry.demand().every_ticks==initial_demand.every_ticks,"rejection precedes scheduler mutation");
    }
    // Valid decoded labels must be escaped again when serialized in success
    // responses, not only in errors. This traverses the actual profile path.
    ws_server_detail::handle_command(R"({"cmd":"apply_profile","name":"x\"y\n\u0000z","_requestId":17})",sockets.server,bridge,telemetry,size);
    const auto profile_reply=parse_json_object(sockets.response());
    check(profile_reply.boolean("ok") && profile_reply.string("scenario")==std::string("x\"y\n\0z",6)
          && profile_reply.integer("_requestId",1,kJsonSafeInteger)==17,
          "profile success escapes quote, newline and NUL while preserving decoded label");
    // Valid updates still follow the public dispatcher and retain exact correlation.
    ws_server_detail::handle_command(R"({"cmd":"set_param","name":"langevin_seed","value":4294967295,"_requestId":9007199254740991})",sockets.server,bridge,telemetry,size);
    auto reply=parse_json_object(sockets.response());
    check(reply.integer("_requestId",1,kJsonSafeInteger)==kJsonSafeInteger && bridge->toggles.langevin_seed==4294967295u,"maximum supported id/seed accepted exactly");
    ws_server_detail::handle_command(R"({"cmd":"inject_flux","x":2147483647,"y":-2147483648,"z":0,"fx":0.125,"fy":0.25,"fz":0.5})",sockets.server,bridge,telemetry,size);
    const auto& voxels=static_cast<const RenderBridge&>(*bridge).voxels();
    const auto& voxel=voxels[bridge->lattice().index(3,0,0)];
    check(voxel.flux.x==0.125 && voxel.flux.y==0.25 && voxel.flux.z==0.5,"extreme valid int coordinates wrap safely before stencil arithmetic");
    std::cout<<checks<<" command checks, "<<failures<<" failures\n";
    return failures ? 1 : 0;
}
