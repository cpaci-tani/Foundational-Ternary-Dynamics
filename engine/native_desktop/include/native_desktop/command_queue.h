#pragma once

#include "native_desktop/ui_command.h"

#include <cstdint>
#include <mutex>
#include <vector>

namespace ftd::native_desktop {

class CommandSink {
public:
    virtual ~CommandSink() = default;
    virtual std::uint64_t push(UiCommand command) = 0;
};

struct QueuedCommand {
    std::uint64_t seq = 0;
    UiCommand command;
};

class CommandQueue : public CommandSink {
public:
    std::uint64_t push(UiCommand command) override;
    std::vector<QueuedCommand> drain();

private:
    std::mutex mu_;
    std::vector<QueuedCommand> pending_;
    std::uint64_t next_seq_ = 1;
};

}  // namespace ftd::native_desktop
