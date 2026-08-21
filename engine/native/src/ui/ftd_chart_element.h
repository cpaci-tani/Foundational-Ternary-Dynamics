// FtdChartElement — the FTD instrument widget: a custom RmlUi element
// (<ftd-chart>) that plots one or more scalar time-series through the engine's own
// D3D12 UI pipeline (native/docs/SPEC_NATIVE_UI_RMLUI.md §4 telemetry model + §5
// charts).
//
// RmlUi Core is renderer-agnostic and lays out RML/RCSS, but it cannot plot a
// data series — so a chart is the one place a custom element is required. This
// element subclasses Rml::Element and, in OnRender(), builds a stroked-line mesh
// (one polyline per series, each in its own colour, each autoscaled to its own
// visible min/max) over its RCSS-computed content box, hands it to the
// RenderManager (MakeGeometry), and renders it — the mesh flows through the same
// Rml::RenderInterface (our RmlD3D12Renderer) as every other UI draw, so the
// series composite into the presenter's command list with the rest of the shell.
//
// Data contract (snapshot-only): the element never reads the engine. Each series
// is a const pointer to a ChartSeries ring buffer the app owns and fills on the
// GUI thread (one scalar per published snapshot). Multiple named series are held
// per element in a ChartRegistry, keyed by the element's `id`; the element looks
// itself up at render time (registration-order independent). Both the push and
// the OnRender read happen on the GUI thread, so no locking is needed. The
// element treats the buffers strictly read-only — it reads the snapshot, it does
// not drive physics.
#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Geometry.h>
#include <RmlUi/Core/Types.h>

namespace ftd::native::ui {

// A small fixed-capacity ring of scalar samples (oldest → newest). Owned by the
// app on the GUI thread; the chart element reads it read-only. `generation()`
// bumps on every push/clear so the element can skip rebuilding an unchanged mesh.
class ChartSeries {
public:
    explicit ChartSeries(std::size_t capacity = 240)
        : buf_(capacity == 0 ? 1 : capacity, 0.0f) {}

    void push(float v) {
        buf_[head_] = v;
        head_ = (head_ + 1) % buf_.size();
        if (count_ < buf_.size()) ++count_;
        ++generation_;
    }

    // Drop all samples (e.g. on a scale switch, so Scale-0 energy is never
    // plotted next to Scale-1 energy). Bumps the generation.
    void clear() {
        head_ = 0;
        count_ = 0;
        ++generation_;
    }

    std::size_t size() const { return count_; }
    std::size_t capacity() const { return buf_.size(); }
    std::uint64_t generation() const { return generation_; }

    // Sample i in [0, size()): 0 = oldest still retained, size()-1 = newest.
    float at(std::size_t i) const {
        const std::size_t start = (head_ + buf_.size() - count_) % buf_.size();
        return buf_[(start + i) % buf_.size()];
    }

    // Min/max over the retained samples. Returns false if empty.
    bool range(float& lo, float& hi) const {
        if (count_ == 0) return false;
        lo = hi = at(0);
        for (std::size_t i = 1; i < count_; ++i) {
            const float v = at(i);
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        return true;
    }

private:
    std::vector<float> buf_;
    std::size_t head_ = 0;   // next write slot
    std::size_t count_ = 0;  // retained samples
    std::uint64_t generation_ = 0;
};

// One named, coloured series drawn inside a <ftd-chart>. `series` is app-owned
// (may be null → skipped); `color` is the trace stroke colour (mirror the RCSS
// legend-chip colour so the legend and the trace agree).
struct ChartSeriesRef {
    const ChartSeries* series = nullptr;
    Rml::Colourb color{106, 168, 224, 255};  // default accent blue (#6aa8e0)
};

// The set of series one <ftd-chart> draws.
struct ChartBinding {
    std::vector<ChartSeriesRef> series;
};

// Maps a <ftd-chart>'s `id` → its series set. The app fills this once at setup;
// each element resolves its own binding by GetId() at render time, so the app
// never wires individual elements and registration order does not matter.
class ChartRegistry {
public:
    ChartBinding& binding(const Rml::String& id) { return map_[id]; }
    const ChartBinding* find(const Rml::String& id) const {
        const auto it = map_.find(id);
        return it == map_.end() ? nullptr : &it->second;
    }

private:
    std::unordered_map<Rml::String, ChartBinding> map_;
};

// The <ftd-chart> element. Resolves its series set from `registry_` by the
// element's `id`, then renders each series as a thin autoscaled polyline in its
// own colour over the content box (each series normalised to its OWN min/max, so
// mixed-unit telemetry channels stay individually legible), plus a faint
// baseline. A null registry, an unknown id, or an empty series set draws only the
// baseline — so the element is safe to instance without data (e.g. the headless
// smoke test).
class FtdChartElement : public Rml::Element {
public:
    FtdChartElement(const Rml::String& tag, const ChartRegistry* registry);

    void set_registry(const ChartRegistry* registry);

protected:
    void OnRender() override;

private:
    void rebuild_geometry(Rml::Vector2f origin, Rml::Vector2f size,
                          const ChartBinding* binding);

    const ChartRegistry* registry_ = nullptr;
    Rml::Geometry geometry_;
    bool built_ = false;
    std::uint64_t built_signature_ = 0;
    float built_w_ = -1.0f;
    float built_h_ = -1.0f;
};

// Instancer for <ftd-chart>. Carries the app's chart registry and stamps it onto
// every element it instances, so the app binds the data source once at
// registration (Rml::Factory::RegisterElementInstancer) with no per-element
// wiring. A null registry is valid — instanced elements simply draw their frame.
class FtdChartInstancer : public Rml::ElementInstancer {
public:
    explicit FtdChartInstancer(const ChartRegistry* registry) : registry_(registry) {}

    Rml::ElementPtr InstanceElement(Rml::Element* parent, const Rml::String& tag,
                                    const Rml::XMLAttributes& attributes) override;
    void ReleaseElement(Rml::Element* element) override;

private:
    const ChartRegistry* registry_;
};

}  // namespace ftd::native::ui
