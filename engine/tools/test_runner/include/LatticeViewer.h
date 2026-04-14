// ============================================================================
// LatticeViewer.h — live 3D voxel viewer (QOpenGLWidget, OpenGL 3.3 core)
// ============================================================================
//
// Phase 4 of the FTD Test Bench. Consumes `snapshot` NDJSON events from the
// Phase 2a telemetry protocol and renders the decoded int8 voxel grid as a
// point cloud with an arcball-driven camera.
//
// Ring buffer holds up to `kRingCapacity` frames (~5 s at 60 Hz) so users can
// pause and scrub back a few seconds into history. The GPU upload only fires
// when the currently displayed frame's `tick` differs from what's already in
// the VBO, so steady-state replay is cheap.
//
// Snapshot format (see engine/include/ftd/test_telemetry.h):
//   {"event":"snapshot", "tick":N, "L":N, "stride":N, "format":"b64-int8",
//    "data":"..."}
// The `data` field is a base64-encoded flat int8 array of length
// `(L/stride)^3` with values in {-1, 0, +1}. Layout is x-fastest:
//   index = (z*Ls + y)*Ls + x    where Ls = L/stride.
//
// No field-line rendering in Phase 4 — `FieldLines` ships as a stub for the
// future vector-field telemetry case. Only voxel states exist in the current
// snapshot protocol.
// ----------------------------------------------------------------------------

#pragma once

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QString>
#include <QVariantMap>
#include <QVector3D>
#include <array>
#include <cstdint>
#include <vector>

class QOpenGLShaderProgram;
class QMouseEvent;
class QWheelEvent;

namespace ftd::testrunner {

struct LatticeFrame {
    int tick = -1;
    int L = 0;
    int stride = 1;
    std::vector<std::int8_t> voxels;  // size = (L/stride)^3 = Ls^3
    std::int64_t timestampMs = 0;
};

class LatticeViewer : public QOpenGLWidget,
                     protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit LatticeViewer(QWidget* parent = nullptr);
    ~LatticeViewer() override;

    // Primary ingestion: decodes the base64 voxel grid, pushes a new
    // LatticeFrame into the ring, and triggers a repaint. Safe to call
    // before `initializeGL()` — the VBO upload is deferred until paintGL().
    void ingestSnapshot(const QVariantMap& event);

    // Clear all state (called when a new test starts, or when stopping).
    void clear();

    // Manual camera / playback controls.
    void setPaused(bool p);
    bool isPaused() const { return m_paused; }

    // 0 = live head, >0 = N frames behind. Clamped to [0, m_count - 1].
    void setScrubOffset(int framesBack);
    int scrubOffset() const { return m_scrubOffset; }

    int frameCount() const { return m_count; }

public slots:
    // Slot form of ingestSnapshot, wired to TestRunner::snapshotReceived.
    // Filters on `m_activeTest` if set; accepts any test when empty.
    void onSnapshotEvent(const QString& testName, const QVariantMap& evt);

    // Restrict ingestion to snapshots from a single test. Empty string
    // clears the filter (default).
    void attachTest(const QString& testName);
    void detachTest();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    // Ring buffer: up to 300 frames (~5s @ 60Hz).
    static constexpr int kRingCapacity = 300;

    std::array<LatticeFrame, kRingCapacity> m_ring{};
    int m_head = 0;        // next slot to write
    int m_count = 0;       // total frames currently in ring
    int m_scrubOffset = 0;
    bool m_paused = false;

    QString m_activeTest;  // filter; empty = accept all

    // Camera (arcball).
    QVector3D m_cameraTarget{0.0f, 0.0f, 0.0f};
    float m_cameraDistance = 3.0f;
    float m_cameraYaw = 0.4f;      // radians, rotation around Y (world up)
    float m_cameraPitch = 0.3f;    // radians, rotation around X
    QPoint m_lastMousePos;

    // GL objects. Created in initializeGL, destroyed with makeCurrent/doneCurrent
    // in the destructor.
    QOpenGLShaderProgram* m_voxelProgram = nullptr;
    QOpenGLShaderProgram* m_lineProgram = nullptr;
    QOpenGLBuffer m_voxelVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_voxelVao;
    QOpenGLBuffer m_lineVbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_lineVao;

    int m_uploadedFrameTick = -1;   // which frame's data is currently in m_voxelVbo
    int m_uploadedVertexCount = 0;  // how many points were uploaded
    int m_bboxVertexCount = 0;      // static line vertex count (bbox + axes)

    bool m_glInitialized = false;   // guards paintGL against races

    // Helpers.
    void buildStaticGeometry();              // bbox + axes line buffer
    void uploadFrameIfNeeded();              // push current frame voxels to VBO
    const LatticeFrame* currentFrame() const;

    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewMatrix() const;
};

}  // namespace ftd::testrunner
