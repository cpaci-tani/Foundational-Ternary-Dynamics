#include "ui/ftd_slice_element.h"

#include <algorithm>
#include <cmath>

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/RenderBox.h>
#include <RmlUi/Core/RenderManager.h>

namespace ftd::native::ui {
namespace {

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

// Piecewise-linear colour ramp. ramp 0 = sequential (dark→blue→teal→green→amber,
// a viridis-like map for magnitude fields); ramp 1 = diverging (blue→dark→red,
// for signed fields normalised symmetric about the mid).
Rml::Colourb colormap(float t, int ramp) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerp = [](int a, int b, float f) {
        return static_cast<Rml::byte>(a + (b - a) * f + 0.5f);
    };
    struct Stop { float t; int r, g, b; };
    static const Stop kSeq[] = {
        {0.00f, 13, 20, 40}, {0.25f, 47, 80, 140}, {0.50f, 46, 150, 160},
        {0.75f, 120, 200, 110}, {1.00f, 230, 200, 90},
    };
    static const Stop kDiv[] = {
        {0.00f, 90, 140, 210}, {0.50f, 22, 30, 46}, {1.00f, 224, 106, 106},
    };
    const Stop* s = (ramp == 1) ? kDiv : kSeq;
    const int n = (ramp == 1) ? 3 : 5;
    for (int i = 1; i < n; ++i) {
        if (t <= s[i].t) {
            const float f = (t - s[i - 1].t) / (s[i].t - s[i - 1].t);
            return Rml::Colourb(lerp(s[i - 1].r, s[i].r, f), lerp(s[i - 1].g, s[i].g, f),
                                lerp(s[i - 1].b, s[i].b, f), 255);
        }
    }
    return Rml::Colourb(s[n - 1].r, s[n - 1].g, s[n - 1].b, 255);
}

}  // namespace

FtdSliceElement::FtdSliceElement(const Rml::String& tag, const SliceRegistry* registry)
    : Rml::Element(tag), registry_(registry) {}

void FtdSliceElement::set_registry(const SliceRegistry* registry) {
    if (registry_ == registry) return;
    registry_ = registry;
    built_ = false;
}

void FtdSliceElement::OnRender() {
    Rml::RenderManager* rm = GetRenderManager();
    if (!rm) return;

    const Rml::RenderBox box = GetRenderBox(Rml::BoxArea::Content);
    const Rml::Vector2f origin = box.GetFillOffset();
    const Rml::Vector2f size = box.GetFillSize();
    if (size.x < 2.0f || size.y < 2.0f) return;

    const SliceBinding* binding = registry_ ? registry_->find(GetId()) : nullptr;
    std::uint64_t sig = 0;
    if (binding && binding->grid) sig = binding->grid->generation() + 1u;

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

void FtdSliceElement::rebuild_geometry(Rml::Vector2f origin, Rml::Vector2f size,
                                       const SliceBinding* binding) {
    Rml::Mesh mesh = geometry_.Release(Rml::Geometry::ReleaseMode::ClearMesh);
    mesh.vertices.clear();
    mesh.indices.clear();

    const float opacity = GetComputedValues().opacity();
    const float x0 = origin.x, y0 = origin.y, w = size.x, h = size.y;
    const SliceGrid* g = binding ? binding->grid : nullptr;

    if (!g || g->empty()) {
        // Faint placeholder so an unbound / pre-data slice still reads as a tile.
        add_quad(mesh, {x0, y0}, {x0 + w, y0}, {x0 + w, y0 + h}, {x0, y0 + h},
                 Rml::Colourb(18, 27, 43, 255).ToPremultiplied(opacity));
        geometry_ = GetRenderManager()->MakeGeometry(std::move(mesh));
        return;
    }

    const int W = g->w(), H = g->h();
    float mn = g->mn(), mx = g->mx();
    float span = mx - mn;
    if (span < 1e-12f) span = 1.0f;
    const float inv = 1.0f / span;
    const int ramp = binding->ramp;
    const float cw = w / static_cast<float>(W);
    const float ch = h / static_cast<float>(H);

    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            const float t = std::clamp((g->at(i, j) - mn) * inv, 0.0f, 1.0f);
            const Rml::ColourbPremultiplied c = colormap(t, ramp).ToPremultiplied(opacity);
            const float cx = x0 + cw * static_cast<float>(i);
            const float cy = y0 + ch * static_cast<float>(j);
            // +0.6 overlap hides seams from fractional cell widths.
            add_quad(mesh, {cx, cy}, {cx + cw + 0.6f, cy}, {cx + cw + 0.6f, cy + ch + 0.6f},
                     {cx, cy + ch + 0.6f}, c);
        }
    }
    geometry_ = GetRenderManager()->MakeGeometry(std::move(mesh));
}

Rml::ElementPtr FtdSliceInstancer::InstanceElement(Rml::Element* /*parent*/,
                                                   const Rml::String& tag,
                                                   const Rml::XMLAttributes& /*attributes*/) {
    return Rml::ElementPtr(new FtdSliceElement(tag, registry_));
}

void FtdSliceInstancer::ReleaseElement(Rml::Element* element) { delete element; }

}  // namespace ftd::native::ui
