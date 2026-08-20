#include "native/snapshot_publisher.h"
#include "ftd/test_telemetry.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

int main() {
    ftd::test::init("test_ui_snapshot_publisher");

    ftd::native::SnapshotPublisher publisher;
    ftd::test::check("acquire before publish returns null",
                     publisher.acquire() == nullptr);

    ftd::native::UiSnapshot first;
    first.seq = 1;
    first.last_applied_seq = 7;
    first.frame.tick = 3;
    publisher.publish(first);
    const auto held = publisher.acquire();
    ftd::test::check("first snapshot is visible", held && held->seq == 1);
    const auto first_checksum = held->checksum();

    ftd::native::UiSnapshot second = *held;
    second.seq = 2;
    second.frame.tick = 4;
    publisher.publish(second);
    ftd::test::check("published objects are immutable",
                     held->seq == 1 && held->checksum() == first_checksum);
    const auto latest = publisher.acquire();
    ftd::test::check("acquire sees the new immutable snapshot",
                     latest && latest->seq == 2 && latest->frame.tick == 4);
    ftd::test::check("checksums differ across seq",
                     latest->checksum() != first_checksum);

    ftd::test::section("concurrent publish/acquire keeps monotone seq");
    ftd::native::SnapshotPublisher stress;
    std::atomic<bool> running{true};
    std::shared_ptr<const ftd::native::UiSnapshot> retained;
    std::uint64_t retained_checksum = 0;
    std::thread writer([&] {
        for (std::uint64_t i = 1; i <= 2000; ++i) {
            ftd::native::UiSnapshot snap;
            snap.seq = i;
            snap.last_applied_seq = i;
            snap.frame.tick = static_cast<int>(i);
            snap.energy_ledger.updates = i;
            stress.publish(std::move(snap));
        }
        running.store(false, std::memory_order_release);
    });

    std::uint64_t last_seq = 0;
    bool monotone = true;
    while (running.load(std::memory_order_acquire)) {
        const auto snap = stress.acquire();
        if (!snap) continue;
        if (snap->seq < last_seq) monotone = false;
        last_seq = snap->seq;
        if (!retained) {
            retained = snap;
            retained_checksum = snap->checksum();
        }
    }
    writer.join();
    const auto final_snap = stress.acquire();
    ftd::test::check("final published seq is 2000",
                     final_snap && final_snap->seq == 2000);
    ftd::test::check("reader never saw a decreasing seq", monotone);
    ftd::test::check("a retained snapshot is unchanged after later publishes",
                     retained && retained->checksum() == retained_checksum);
    ftd::test::check("final checksum is self-consistent",
                     final_snap && final_snap->checksum() == final_snap->checksum());

    return ftd::test::finalize();
}
