#pragma once
#include "ftd/ws_protocol.h"
#include <stdexcept>
#include <string>
#include <vector>
namespace ws_test {
struct Platform {
    Platform() {
#ifdef _WIN32
        WSADATA data{}; if (WSAStartup(MAKEWORD(2,2), &data)) throw std::runtime_error("WSAStartup");
#endif
    }
    ~Platform() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};
struct Pair {
    SOCKET server=INVALID_SOCKET, client=INVALID_SOCKET;
    explicit Pair(bool constrained = false) {
        SOCKET listener=::socket(AF_INET,SOCK_STREAM,0);
        if(listener==INVALID_SOCKET) throw std::runtime_error("socket");
        sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        if(::bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address)) || ::listen(listener,1)) {
            closesocket(listener); throw std::runtime_error("bind/listen");
        }
#ifdef _WIN32
        int length=sizeof(address);
#else
        socklen_t length=sizeof(address);
#endif
        if(::getsockname(listener,reinterpret_cast<sockaddr*>(&address),&length)) { closesocket(listener); throw std::runtime_error("getsockname"); }
        client=::socket(AF_INET,SOCK_STREAM,0);
        if (client == INVALID_SOCKET) { closesocket(listener); throw std::runtime_error("client socket"); }
        // Set before connect: Winsock may negotiate a large receive window if
        // this is changed only after the TCP connection has been established.
        if (constrained) checked_buffer(client, SO_RCVBUF);
        if(::connect(client,reinterpret_cast<sockaddr*>(&address),sizeof(address))) {
            closesocket(client); closesocket(listener); throw std::runtime_error("connect");
        }
        server=::accept(listener,nullptr,nullptr); closesocket(listener);
        if(server==INVALID_SOCKET) { closesocket(client); throw std::runtime_error("accept"); }
        if (constrained) checked_buffer(server, SO_SNDBUF);
    }
    ~Pair() { if(server!=INVALID_SOCKET) closesocket(server); if(client!=INVALID_SOCKET) closesocket(client); }
    void half_close_client() {
#ifdef _WIN32
        ::shutdown(client,SD_SEND);
#else
        ::shutdown(client,SHUT_WR);
#endif
    }
    void close_client() { closesocket(client); client=INVALID_SOCKET; }
    static void checked_buffer(SOCKET socket, int option) {
        const int bytes=1024;
        if (::setsockopt(socket,SOL_SOCKET,option,reinterpret_cast<const char*>(&bytes),sizeof(bytes)))
            throw std::runtime_error("setsockopt test buffer");
        int actual=0;
#ifdef _WIN32
        int length=sizeof(actual);
#else
        socklen_t length=sizeof(actual);
#endif
        if (::getsockopt(socket,SOL_SOCKET,option,reinterpret_cast<char*>(&actual),&length) || actual<=0)
            throw std::runtime_error("getsockopt test buffer");
    }
};
inline std::vector<std::uint8_t> client_frame(std::uint8_t opcode,std::size_t bytes) {
    std::vector<std::uint8_t> frame{static_cast<std::uint8_t>(0x80|opcode)};
    if(bytes<126) frame.push_back(static_cast<std::uint8_t>(0x80|bytes));
    else if(bytes<65536) { frame.push_back(0xfe); frame.push_back(static_cast<std::uint8_t>(bytes>>8)); frame.push_back(static_cast<std::uint8_t>(bytes)); }
    else { frame.push_back(0xff); for(int i=7;i>=0;--i) frame.push_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(bytes)>>(8*i))); }
    const std::uint8_t mask[4]={1,17,99,241};
    frame.insert(frame.end(),mask,mask+4);
    for(std::size_t i=0;i<bytes;++i) frame.push_back(static_cast<std::uint8_t>(i%251)^mask[i%4]);
    return frame;
}
}
