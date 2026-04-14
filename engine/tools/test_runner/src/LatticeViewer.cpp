// ============================================================================
// LatticeViewer.cpp — implementation
// ============================================================================

#include "LatticeViewer.h"

#include "LatticeViewer_shaders.h"

#include <QByteArray>
#include <QDateTime>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QString>
#include <QVector3D>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ftd::testrunner {

namespace {

// Base point size (in pixels at w=1.0). Scaled in the vertex shader via
// 1/w so distant voxels shrink naturally.
constexpr float kPointSizeBase = 48.0f;

// Clamp helper (C++17 has std::clamp but we spell it out for clarity).
inline float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

}  // namespace

// ============================================================================
// Construction / destruction
// ============================================================================

LatticeViewer::LatticeViewer(QWidget* parent) : QOpenGLWidget(parent) {
    // Ask for a depth buffer + multisample for smoother edges; the
    // QSurfaceFormat on the parent widget normally picks this up, but
    // setting it here is defensive for Phase 4.
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    setFormat(fmt);

    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
}

LatticeViewer::~LatticeViewer() {
    if (m_glInitialized) {
        makeCurrent();
        if (m_voxelVbo.isCreated()) m_voxelVbo.destroy();
        if (m_voxelVao.isCreated()) m_voxelVao.destroy();
        if (m_lineVbo.isCreated()) m_lineVbo.destroy();
        if (m_lineVao.isCreated()) m_lineVao.destroy();
        delete m_voxelProgram;
        delete m_lineProgram;
        m_voxelProgram = nullptr;
        m_lineProgram = nullptr;
        doneCurrent();
    }
}

// ============================================================================
// Public ingestion / controls
// ============================================================================

void LatticeViewer::ingestSnapshot(const QVariantMap& evt) {
    // Required fields.
    const int L      = evt.value(QStringLiteral("L")).toInt();
    const int stride = std::max(1, evt.value(QStringLiteral("stride")).toInt());
    const int tick   = evt.value(QStringLiteral("tick")).toInt();
    const QString fmtStr = evt.value(QStringLiteral("format")).toString();
    const QString dataB64 = evt.value(QStringLiteral("data")).toString();

    if (L <= 0 || dataB64.isEmpty()) {
        return;
    }
    // Phase 4 only understands the Phase 2a "b64-int8" format. Silently
    // ignore unknown formats so future extensions don't crash.
    if (!fmtStr.isEmpty() && fmtStr != QStringLiteral("b64-int8")) {
        return;
    }

    const QByteArray raw =
        QByteArray::fromBase64(dataB64.toUtf8());
    const int Ls = L / stride;
    if (Ls <= 0) return;
    const std::size_t expected =
        static_cast<std::size_t>(Ls) * Ls * Ls;
    if (raw.size() <= 0) return;
    if (static_cast<std::size_t>(raw.size()) != expected) {
        // Size mismatch — refuse to mis-index into a malformed grid.
        // We don't Q_ASSERT here because telemetry may arrive from flaky
        // tests; just skip the frame.
        return;
    }

    LatticeFrame& slot = m_ring[m_head];
    slot.tick = tick;
    slot.L = L;
    slot.stride = stride;
    slot.voxels.assign(
        reinterpret_cast<const std::int8_t*>(raw.constData()),
        reinterpret_cast<const std::int8_t*>(raw.constData()) + raw.size());
    slot.timestampMs = QDateTime::currentMSecsSinceEpoch();

    m_head = (m_head + 1) % kRingCapacity;
    if (m_count < kRingCapacity) m_count += 1;

    // If the user is actively scrubbing back, holding the scrub offset
    // constant effectively pins the displayed frame — which is what we
    // want. Otherwise always snap to head.
    if (!m_paused && m_scrubOffset == 0) {
        update();
    } else if (m_paused) {
        // Paused: repaint only if scrub offset is 0 (they may want to
        // peek at incoming frames even while paused — but generally
        // pause means "freeze").
    } else {
        update();
    }
}

void LatticeViewer::clear() {
    for (auto& f : m_ring) {
        f.tick = -1;
        f.L = 0;
        f.stride = 1;
        f.voxels.clear();
        f.timestampMs = 0;
    }
    m_head = 0;
    m_count = 0;
    m_scrubOffset = 0;
    m_uploadedFrameTick = -1;
    m_uploadedVertexCount = 0;
    if (m_glInitialized) update();
}

void LatticeViewer::setPaused(bool p) {
    m_paused = p;
}

void LatticeViewer::setScrubOffset(int framesBack) {
    const int maxOff = std::max(0, m_count - 1);
    m_scrubOffset = std::max(0, std::min(framesBack, maxOff));
    if (m_glInitialized) update();
}

void LatticeViewer::onSnapshotEvent(const QString& testName,
                                    const QVariantMap& evt) {
    if (!m_activeTest.isEmpty() && testName != m_activeTest) {
        return;
    }
    ingestSnapshot(evt);
}

void LatticeViewer::attachTest(const QString& testName) {
    m_activeTest = testName;
    clear();
}

void LatticeViewer::detachTest() {
    m_activeTest.clear();
}

// ============================================================================
// Frame selection
// ============================================================================

const LatticeFrame* LatticeViewer::currentFrame() const {
    if (m_count == 0) return nullptr;
    // The most recent frame lives at (m_head - 1); m_scrubOffset walks back
    // from there.
    int idx = (m_head - 1 - m_scrubOffset + kRingCapacity) % kRingCapacity;
    if (idx < 0 || idx >= kRingCapacity) return nullptr;
    return &m_ring[idx];
}

// ============================================================================
// OpenGL lifecycle
// ============================================================================

void LatticeViewer::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---- voxel program ----
    // NOTE: not parented to `this` on purpose — we destroy these manually
    // in the dtor (inside makeCurrent/doneCurrent), so letting Qt's
    // parent-child ownership re-delete them would be a use-after-free.
    m_voxelProgram = new QOpenGLShaderProgram();
    if (!m_voxelProgram->addShaderFromSourceCode(
            QOpenGLShader::Vertex, shaders::kVoxelVertex)) {
        qWarning("LatticeViewer: voxel vertex shader compile failed: %s",
                 qPrintable(m_voxelProgram->log()));
    }
    if (!m_voxelProgram->addShaderFromSourceCode(
            QOpenGLShader::Fragment, shaders::kVoxelFragment)) {
        qWarning("LatticeViewer: voxel fragment shader compile failed: %s",
                 qPrintable(m_voxelProgram->log()));
    }
    if (!m_voxelProgram->link()) {
        qWarning("LatticeViewer: voxel program link failed: %s",
                 qPrintable(m_voxelProgram->log()));
    }

    // ---- line program ----
    // Same non-parented policy as voxelProgram; manual destruction in dtor.
    m_lineProgram = new QOpenGLShaderProgram();
    if (!m_lineProgram->addShaderFromSourceCode(
            QOpenGLShader::Vertex, shaders::kLineVertex)) {
        qWarning("LatticeViewer: line vertex shader compile failed: %s",
                 qPrintable(m_lineProgram->log()));
    }
    if (!m_lineProgram->addShaderFromSourceCode(
            QOpenGLShader::Fragment, shaders::kLineFragment)) {
        qWarning("LatticeViewer: line fragment shader compile failed: %s",
                 qPrintable(m_lineProgram->log()));
    }
    if (!m_lineProgram->link()) {
        qWarning("LatticeViewer: line program link failed: %s",
                 qPrintable(m_lineProgram->log()));
    }

    // ---- voxel VBO / VAO (dynamic, uploaded per-frame) ----
    m_voxelVao.create();
    m_voxelVao.bind();

    m_voxelVbo.create();
    m_voxelVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_voxelVbo.bind();

    // Interleaved layout (16 bytes per vertex):
    //   floats [0..2]: position (x, y, z)
    //   int    [3]   : state (-1, 0, +1) — packed as int32 for alignment
    const int kStride = 4 * sizeof(float);  // 3 floats + 1 int32 = 16 bytes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, kStride, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(
        1, 1, GL_INT, kStride, reinterpret_cast<const void*>(3 * sizeof(float)));

    m_voxelVbo.release();
    m_voxelVao.release();

    // ---- line VBO / VAO (static bbox + axes) ----
    m_lineVao.create();
    m_lineVao.bind();
    m_lineVbo.create();
    m_lineVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_lineVbo.bind();

    const int kLineStride = 6 * sizeof(float);  // pos3 + color3
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, kLineStride, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, kLineStride,
        reinterpret_cast<const void*>(3 * sizeof(float)));

    m_lineVbo.release();
    m_lineVao.release();

    buildStaticGeometry();

    m_glInitialized = true;
}

void LatticeViewer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void LatticeViewer::paintGL() {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QMatrix4x4 mvp = projectionMatrix() * viewMatrix();

    // ---- bounding box + axes ----
    if (m_lineProgram && m_bboxVertexCount > 0) {
        m_lineProgram->bind();
        m_lineProgram->setUniformValue("uMVP", mvp);
        m_lineVao.bind();
        glDrawArrays(GL_LINES, 0, m_bboxVertexCount);
        m_lineVao.release();
        m_lineProgram->release();
    }

    // ---- voxel points ----
    uploadFrameIfNeeded();

    if (m_voxelProgram && m_uploadedVertexCount > 0) {
        m_voxelProgram->bind();
        m_voxelProgram->setUniformValue("uMVP", mvp);
        m_voxelProgram->setUniformValue("uPointSize", kPointSizeBase);
        m_voxelVao.bind();
        glDrawArrays(GL_POINTS, 0, m_uploadedVertexCount);
        m_voxelVao.release();
        m_voxelProgram->release();
    }
}

// ============================================================================
// Static geometry (bbox + axes)
// ============================================================================

void LatticeViewer::buildStaticGeometry() {
    // 12 edges of the unit cube at [-1, 1]^3, plus three axes in red/green/
    // blue from origin to the respective unit vector.
    std::vector<float> verts;
    verts.reserve(6 * (24 + 6));

    auto pushLine = [&verts](float x0, float y0, float z0,
                             float x1, float y1, float z1,
                             float r, float g, float b) {
        verts.push_back(x0); verts.push_back(y0); verts.push_back(z0);
        verts.push_back(r);  verts.push_back(g);  verts.push_back(b);
        verts.push_back(x1); verts.push_back(y1); verts.push_back(z1);
        verts.push_back(r);  verts.push_back(g);  verts.push_back(b);
    };

    const float a = -1.0f;
    const float b = 1.0f;
    const float bcR = 0.35f, bcG = 0.35f, bcB = 0.40f;

    // Bottom square (y = a).
    pushLine(a, a, a, b, a, a, bcR, bcG, bcB);
    pushLine(b, a, a, b, a, b, bcR, bcG, bcB);
    pushLine(b, a, b, a, a, b, bcR, bcG, bcB);
    pushLine(a, a, b, a, a, a, bcR, bcG, bcB);

    // Top square (y = b).
    pushLine(a, b, a, b, b, a, bcR, bcG, bcB);
    pushLine(b, b, a, b, b, b, bcR, bcG, bcB);
    pushLine(b, b, b, a, b, b, bcR, bcG, bcB);
    pushLine(a, b, b, a, b, a, bcR, bcG, bcB);

    // Vertical edges.
    pushLine(a, a, a, a, b, a, bcR, bcG, bcB);
    pushLine(b, a, a, b, b, a, bcR, bcG, bcB);
    pushLine(b, a, b, b, b, b, bcR, bcG, bcB);
    pushLine(a, a, b, a, b, b, bcR, bcG, bcB);

    // Axis lines (from origin outward to +1 in each axis).
    pushLine(0, 0, 0, 1, 0, 0, 0.90f, 0.25f, 0.25f);  // X = red
    pushLine(0, 0, 0, 0, 1, 0, 0.25f, 0.90f, 0.25f);  // Y = green
    pushLine(0, 0, 0, 0, 0, 1, 0.25f, 0.45f, 0.95f);  // Z = blue

    m_bboxVertexCount = static_cast<int>(verts.size() / 6);

    m_lineVao.bind();
    m_lineVbo.bind();
    m_lineVbo.allocate(
        verts.data(),
        static_cast<int>(verts.size() * sizeof(float)));
    m_lineVbo.release();
    m_lineVao.release();
}

// ============================================================================
// Per-frame upload
// ============================================================================

void LatticeViewer::uploadFrameIfNeeded() {
    const LatticeFrame* f = currentFrame();
    if (!f || f->L <= 0 || f->voxels.empty()) {
        m_uploadedVertexCount = 0;
        return;
    }

    // Compose a cheap per-frame "identity" — include the ring index as a
    // signature so two frames with the same logical tick (possible across
    // separate runs) still trigger an upload.
    if (f->tick == m_uploadedFrameTick && m_uploadedVertexCount > 0) {
        return;
    }

    const int Ls = f->L / std::max(1, f->stride);
    if (Ls <= 0) {
        m_uploadedVertexCount = 0;
        return;
    }

    // Convert grid coordinate [0, Ls-1] → normalized [-1, 1]. The cell
    // center offset (+0.5) keeps the point cloud visually centered in the
    // bounding box regardless of Ls.
    const float invHalf = 2.0f / static_cast<float>(Ls);
    std::vector<float> interleaved;
    interleaved.reserve(f->voxels.size() * 4);

    const int LsSquared = Ls * Ls;
    for (int z = 0; z < Ls; ++z) {
        for (int y = 0; y < Ls; ++y) {
            for (int x = 0; x < Ls; ++x) {
                const std::size_t idx =
                    static_cast<std::size_t>(z) * LsSquared +
                    static_cast<std::size_t>(y) * Ls +
                    static_cast<std::size_t>(x);
                const std::int8_t s = f->voxels[idx];
                if (s == 0) continue;  // skip void voxels

                const float px = -1.0f + (static_cast<float>(x) + 0.5f) * invHalf;
                const float py = -1.0f + (static_cast<float>(y) + 0.5f) * invHalf;
                const float pz = -1.0f + (static_cast<float>(z) + 0.5f) * invHalf;

                interleaved.push_back(px);
                interleaved.push_back(py);
                interleaved.push_back(pz);
                // Pack int into a float slot via type-punning through the
                // interleaved buffer; the GL attribute is declared as int
                // and we used glVertexAttribIPointer, so the underlying
                // bits must be the exact int bit-pattern.
                const std::int32_t stateInt = static_cast<std::int32_t>(s);
                float stateAsFloat = 0.0f;
                static_assert(sizeof(stateInt) == sizeof(stateAsFloat),
                              "int32/float32 size mismatch");
                std::memcpy(&stateAsFloat, &stateInt, sizeof(stateInt));
                interleaved.push_back(stateAsFloat);
            }
        }
    }

    const int vertexCount = static_cast<int>(interleaved.size() / 4);
    m_voxelVao.bind();
    m_voxelVbo.bind();
    m_voxelVbo.allocate(
        interleaved.data(),
        static_cast<int>(interleaved.size() * sizeof(float)));
    m_voxelVbo.release();
    m_voxelVao.release();

    m_uploadedFrameTick = f->tick;
    m_uploadedVertexCount = vertexCount;
}

// ============================================================================
// Camera
// ============================================================================

QMatrix4x4 LatticeViewer::projectionMatrix() const {
    QMatrix4x4 p;
    const float aspect =
        (height() > 0) ? (static_cast<float>(width()) / height()) : 1.0f;
    p.perspective(45.0f, aspect, 0.01f, 100.0f);
    return p;
}

QMatrix4x4 LatticeViewer::viewMatrix() const {
    // Spherical camera around m_cameraTarget.
    const float cosPitch = std::cos(m_cameraPitch);
    const float sinPitch = std::sin(m_cameraPitch);
    const float cosYaw = std::cos(m_cameraYaw);
    const float sinYaw = std::sin(m_cameraYaw);

    const QVector3D eye(
        m_cameraTarget.x() + m_cameraDistance * cosPitch * sinYaw,
        m_cameraTarget.y() + m_cameraDistance * sinPitch,
        m_cameraTarget.z() + m_cameraDistance * cosPitch * cosYaw);

    QMatrix4x4 v;
    v.lookAt(eye, m_cameraTarget, QVector3D(0, 1, 0));
    return v;
}

// ============================================================================
// Mouse interaction
// ============================================================================

void LatticeViewer::mousePressEvent(QMouseEvent* e) {
    m_lastMousePos = e->pos();
}

void LatticeViewer::mouseMoveEvent(QMouseEvent* e) {
    const QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();

    if (e->buttons() & Qt::LeftButton) {
        // Arcball rotate.
        const float dx = static_cast<float>(delta.x()) * 0.01f;
        const float dy = static_cast<float>(delta.y()) * 0.01f;
        m_cameraYaw += dx;
        m_cameraPitch += dy;
        const float limit = qDegreesToRadians(89.0f);
        m_cameraPitch = clampf(m_cameraPitch, -limit, limit);
        update();
    } else if (e->buttons() & (Qt::RightButton | Qt::MiddleButton)) {
        // Pan target in screen-aligned axes. Recover right/up from the
        // current view matrix.
        const QMatrix4x4 v = viewMatrix();
        const QVector3D right(v(0, 0), v(0, 1), v(0, 2));
        const QVector3D up(v(1, 0), v(1, 1), v(1, 2));
        const float panScale = 0.0025f * m_cameraDistance;
        m_cameraTarget -= right * (delta.x() * panScale);
        m_cameraTarget += up * (delta.y() * panScale);
        update();
    }
}

void LatticeViewer::wheelEvent(QWheelEvent* e) {
    // 1 notch = 120 y. Zoom in/out by a factor of 1.1 per notch.
    const int notches = e->angleDelta().y() / 120;
    if (notches == 0) return;
    const float factor = std::pow(0.9f, static_cast<float>(notches));
    m_cameraDistance = clampf(m_cameraDistance * factor, 0.2f, 50.0f);
    update();
}

}  // namespace ftd::testrunner
