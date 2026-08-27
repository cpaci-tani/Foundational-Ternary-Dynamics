/**
 * FTD WebSocket Server
 *
 * Standalone executable bridging the native engine to the web dashboard.
 * Protocol modules live in ws_server_{commands,binary,telemetry,runtime}.cpp.
 */

#include "ws_server_internal.h"

int main(int argc, char* argv[]) {
    return ftd::ws_server_detail::run_server(argc, argv);
}
