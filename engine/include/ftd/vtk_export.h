#pragma once
/**
 * Native ParaView/VTK research export for RenderBridge snapshots.
 *
 * Writes dependency-free VTK XML ASCII artifacts:
 *   - ImageData (.vti) for full lattice fields
 *   - PolyData (.vtp) for particles and cluster centroids
 *   - ParaView collections (.pvd) for time series playback
 *
 * This exporter is an observation layer only. It does not modify engine state
 * except for the normal host synchronization performed by RenderBridge accessors.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "cluster_tracker.h"
#include "render_bridge.h"

namespace ftd {
namespace sciviz {

struct ExportOptions {
    std::string output_dir = "output";
    std::string run_name = "ftd_research_export";
    int frame_interval = 1;
    int spatial_stride = 1;

    bool export_fields = true;
    bool export_particles = true;
    bool export_operators = true;
    bool export_forces = true;
    bool export_dual_fields = true;
    bool export_interaction_fields = true;
    bool export_clusters = true;
};

class ResearchExportSession {
public:
    explicit ResearchExportSession(ExportOptions options);

    bool record(const RenderBridge& bridge);
    bool finalize();

    const ExportOptions& options() const { return options_; }
    int frame_count() const { return static_cast<int>(frames_.size()); }

private:
    struct FrameRecord {
        int frame = 0;
        int tick = 0;
        double time = 0.0;
        std::string fields_file;
        std::string particles_file;
        std::string clusters_file;
    };

    bool ensure_directories();
    bool write_field_frame(const RenderBridge& bridge, int frame_index, std::string& relative_file) const;
    bool write_particle_frame(const RenderBridge& bridge, int frame_index, std::string& relative_file) const;
    bool write_cluster_frame(const RenderBridge& bridge, int frame_index, std::string& relative_file) const;
    bool append_diagnostics(const RenderBridge& bridge, int frame_index);
    bool write_pvd(const std::string& relative_path,
                   const std::string& vtk_type,
                   const std::vector<std::string>& files) const;
    bool write_manifest() const;
    bool write_cluster_tracks() const;

    ExportOptions options_;
    std::vector<FrameRecord> frames_;
    ClusterTracker cluster_tracker_;
    bool diagnostics_header_written_ = false;
};

bool export_research_snapshot(const RenderBridge& bridge,
                              const ExportOptions& options);

}  // namespace sciviz
}  // namespace ftd
