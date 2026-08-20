#pragma once

#include <cstdint>
#include <string>

namespace ftd::native_desktop {

struct ApplyResult {
    std::uint64_t sequence = 0;
    bool ok = true;
    int error_code = 0;
    std::string message;
};

enum class ObservationStatus {
    Ready = 0,
    PendingAfterHostUpload = 1,
    Rejected = 2
};

struct ObservationResult {
    std::uint64_t sequence = 0;
    ObservationStatus status = ObservationStatus::Ready;
    std::string message;
};

enum class ReloadStatus {
    Success = 0,
    UnknownScenario = 1,
    ValidationRejected = 2,
    BackendRecreationFailed = 3,
    InteropReimportRequired = 4
};

struct ReloadResult {
    ReloadStatus status = ReloadStatus::Success;
    std::string message;
};

struct TickResult {
    bool ok = true;
    std::string message;
};

struct LoopControl {
    bool pause = false;
    bool run = false;
    int pending_steps = 0;
};

}  // namespace ftd::native_desktop
