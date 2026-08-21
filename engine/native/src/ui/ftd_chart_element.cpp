#include "ui/ftd_chart_element.h"

#include <algorithm>
#include <cmath>

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/RenderBox.h>
#include <RmlUi/Core/RenderManager.h>

namespace ftd::native::ui {
namespace {

// Append a filled quad (two triangles) to `mesh`. Positions are already in the
// element's border-box-relative space; the UI pipeline binds a 1x1 white default
// for untextured geometry (texture handle 0), so tex coords are unused and the
// vertex colour is the final colour.
void add_quad(Rml::Mesh& mesh, Rml::Vector2f a, Rml::Vector2f b, Rml::Vector2f c,
              Rml::Vector2f d, Rml::ColourbPremultiplied colour) {
    const int base = static_cast<int>(mesh.vertices.size());
    mesh.vertices.push_back(Rml::Vertex{a, colour, Rml::Vector2f(0.0f, 0.0f)});
    mesh.vertices.push_back(Rml::Vertex{b, colour, Rml::Vector2f(0.0f, 0.0f)});
    mesh.vertices.push_back(Rml::Vertex{c, colour, Rml::Vector2f(0.0f, 0.0f)});
    mesh.vertices.push_back(Rml::Vertex{d, colour, Rml::Vector2f(0.0f, 0.0f)});
    mesh.indices.push_back(base + 0);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 0);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 3);
}

// One series as a thin polyline, autoscaled to its OWN min/max over the plot box.
// Vertical-thickness band per segment (fine for mostly-horizontal telemetry — a
// v1 line, not a mitred one). Mixed-unit channels stay legible because each is
// normalised independently.
void add_series_stroke(Rml::Mesh& mesh, const ChartSeries& s, float x0, float y0,
                       float w, float h, Rml::ColourbPremultiplied stroke) {
    const std::size_t n = s.size();
    if (n < 2) return;

    float lo = 0.0f, hi = 0.0f;
    s.range(lo, hi);
    // Autoscale with a 6% margin so the trace never glues to the top/bottom; a
    // flat series (hi == lo) centres on the mid-line.
    float span = hi - lo;
    if (span < 1e-9f) {
        const float pad = (std::fabs(hi) > 1e-6f) ? std::fabs(hi) * 0.5f : 1.0f;
        lo -= pad;
        hi += pad;
        span = hi - lo;
    } else {
        const float margin = span * 0.06f;
        lo -= margin;
        hi += margin;
        span = hi - lo;
    }

    const float inv_span = 1.0f / span;
    const float dx = w / static_cast<float>(n - 1);
    const float stroke_half = 1.0f;  // ~2px line

    auto sample_xy = [&](std::size_t i) -> Rml::Vector2f {
        const float v = s.at(i);
        const float x = x0 + dx * static_cast<float>(i);
        const float t = (v - lo) * inv_span;  // 0..1 (lo..hi)
        const float y = y0 + (1.0f - std::clamp(t, 0.0f, 1.0f)) * h;
        return Rml::Vector2f(x, y);
    };

    Rml::Vector2f prev = sample_xy(0);
    for (std::size_t i = 1; i < n; ++i) {
        const Rml::Vector2f cur = sample_xy(i);
        add_quad(mesh, {prev.x, prev.y - stroke_half}, {cur.x, cur.y - stroke_half},
                 {cur.x, cur.y + stroke_half}, {prev.x, prev.y + stroke_half}, stroke);
        prev = cur;
    }
}

}  // namespace

FtdChartElement::FtdChartElement(const Rml::String& tag, const ChartRegistry* registry)
    : Rml::Element(tag), registry_(registry) {}

void FtdChartElement::set_registry(const ChartRegistry* registry) {
    if (registry_ == registry) return;
    registry_ = registry;
    built_ = false;  // force a rebuild next render
}

void FtdChartElement::OnRender() {
    Rml::RenderManager* rm = GetRenderManager();
    if (!rm) return;

    // Content box (border+padding inset already folded in, exactly as ElementImage
    // does): positions are relative to the border box, rendered at the border
    // origin's absolute offset.
    const Rml::RenderBox box = GetRenderBox(Rml::BoxArea::Content);
    const Rml::Vector2f origin = box.GetFillOffset();
    const Rml::Vector2f size = box.GetFillSize();
    if (size.x < 2.0f || size.y < 2.0f) return;

    const ChartBinding* binding = registry_ ? registry_->find(GetId()) : nullptr;

    // Combined signature over every series' generation: rebuild only when a series
    // actually advanced (or the box resized).
    std::uint64_t sig = 0;
    if (binding) {
        for (const ChartSeriesRef& ref : binding->series) {
            if (ref.series) sig = sig * 1000003ull + ref.series->generation() + 1u;
        }
    }

    if (!built_ || sig != built_signature_ || size.x != built_w_ || size.y != built_h_) {
        rebuild_geometry(origin, size, binding);
        built_ = true;
        built_signature_ = sig;
        built_w_ = size.x;
        built_h_ = size.y;
    }

    if (static_cast<bool>(geometry_))
        geometry_.Render(GetAbsoluteOffset(Rml::BoxArea::Border));
}

void FtdChartElement::rebuild_geometry(Rml::Vector2f origin, Rml::Vector2f size,
                                       const ChartBinding* binding) {
    // Reuse the backing mesh storage across rebuilds.
    Rml::Mesh mesh = geometry_.Release(Rml::Geometry::ReleaseMode::ClearMesh);
    mesh.vertices.clear();
    mesh.indices.clear();

    const Rml::ComputedValues& computed = GetComputedValues();
    const float opacity = computed.opacity();
    const Rml::ColourbPremultiplied baseline =
        Rml::Colourb(120, 140, 170, 255).ToPremultiplied(0.55f * opacity);

    const float x0 = origin.x;
    const float y0 = origin.y;
    const float w = size.x;
    const float h = size.y;
    const float y_bottom = y0 + h;

    // A faint baseline along the bottom of the plot area (1px), always drawn so an
    // empty/flat chart still reads as a chart.
    add_quad(mesh, {x0, y_bottom - 1.0f}, {x0 + w, y_bottom - 1.0f},
             {x0 + w, y_bottom}, {x0, y_bottom}, baseline);

    if (binding) {
        for (const ChartSeriesRef& ref : binding->series) {
            if (!ref.series) continue;
            const Rml::ColourbPremultiplied stroke = ref.color.ToPremultiplied(opacity);
            add_series_stroke(mesh, *ref.series, x0, y0, w, h, stroke);
        }
    }

    if (mesh.indices.empty()) {
        geometry_ = Rml::Geometry();  // nothing to draw (keeps operator bool false)
        return;
    }
    geometry_ = GetRenderManager()->MakeGeometry(std::move(mesh));
}

// ── Instancer ────────────────────────────────────────────────────────────────
Rml::ElementPtr FtdChartInstancer::InstanceElement(Rml::Element* /*parent*/,
                                                   const Rml::String& tag,
                                                   const Rml::XMLAttributes& /*attributes*/) {
    return Rml::ElementPtr(new FtdChartElement(tag, registry_));
}

void FtdChartInstancer::ReleaseElement(Rml::Element* element) { delete element; }

}  // namespace ftd::native::ui
