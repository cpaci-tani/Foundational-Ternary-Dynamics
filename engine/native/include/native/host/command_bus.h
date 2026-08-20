#pragma once
//
// host/command_bus.h — the scale-generic command FIFO.
//
// Mirrors the salvaged native/command_queue.h mechanism (mutex + append +
// drain, monotonic sequence) but carries the composed ScaleCommand (§4.2)
// instead of the flat Scale-0 UiCommand. Header-only so the seam adds no new
// translation unit at R1 step 1; the coalesce refinement from CommandQueue can
// fold in later without changing this interface.
//
#include "native/model/commands.h"

#include <cstdint>
#include <mutex>
#include <utility>
#include <vector>

namespace ftd::native {

struct BusCommand {
    std::uint64_t seq = 0;
    ScaleCommand  command;
};

class CommandBus {
public:
    std::uint64_t push(ScaleCommand command) {
        std::lock_guard<std::mutex> lock(mu_);
        const std::uint64_t seq = next_seq_++;
        pending_.push_back(BusCommand{seq, std::move(command)});
        return seq;
    }

    std::vector<BusCommand> drain() {
        std::lock_guard<std::mutex> lock(mu_);
        std::vector<BusCommand> out;
        out.swap(pending_);
        return out;
    }

private:
    std::mutex mu_;
    std::vector<BusCommand> pending_;
    std::uint64_t next_seq_ = 1;
};

}  // namespace ftd::native
