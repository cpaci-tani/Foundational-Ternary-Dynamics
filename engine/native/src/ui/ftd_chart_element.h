// FtdChartElement — the first FTD instrument widget: a custom RmlUi element
// (<ftd-chart>) that plots a scalar time-series through the engine's own D3D12 UI
// pipeline (native/docs/SPEC_NATIVE_UI_RMLUI.md §4 telemetry model + §5 charts).
//
// RmlUi Core is renderer-agnostic and lays out RML/RCSS, but it cannot plot a
// data series — so a chart is the one place a custom element is required. This
// element subclasses Rml::Element and, in OnRender(), builds a filled-area mesh
// from a ring buffer over its RCSS-computed content box, hands it to the
// RenderManager (MakeGeometry), and renders it — the mesh flows through the same
// Rml::RenderInterface (our RmlD3D12Renderer) as every other UI draw, so the
// series composites into the presenter's command list with the rest of the shell.
//
// Data contract (snapshot-only): the element never reads the engine. It holds a
// const pointer to a ChartSeries ring buffer that the app owns and fills on the
// GUI thread (one scalar per published snapshot). Both the push and the
// OnRender read happen on the GUI thread, so no locking is needed. The element
// treats the buffer strictly read-only — it reads the snapshot, it does not
// drive physics.
#pragma once

#include <cstddef>
#include <cstdint>
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

// The <ftd-chart> element. Renders `series_` as a filled-area line over the
// element's content box, autoscaled to the visible min/max, stroked in the
// element's RCSS-computed `color` (frame/background come from RCSS). A null
// series (or an empty one) draws nothing — the RCSS frame still shows — so the
// element is safe to instance without data (e.g. the headless smoke test).
class FtdChartElement : public Rml::Element {
public:
    FtdChartElement(const Rml::String& tag, const ChartSeries* series);

    void set_series(const ChartSeries* series);

protected:
    void OnRender() override;

private:
    void rebuild_geometry(Rml::Vector2f origin, Rml::Vector2f size);

    const ChartSeries* series_ = nullptr;
    Rml::Geometry geometry_;
    bool built_ = false;
    std::uint64_t built_generation_ = 0;
    float built_w_ = -1.0f;
    float built_h_ = -1.0f;
};

// Instancer for <ftd-chart>. Carries the app's ring-buffer pointer and stamps it
// onto every element it instances, so the app binds the data source once at
// registration (Rml::Factory::RegisterElementInstancer) with no per-element
// wiring. A null series is valid — instanced elements simply draw their frame.
class FtdChartInstancer : public Rml::ElementInstancer {
public:
    explicit FtdChartInstancer(const ChartSeries* series) : series_(series) {}

    Rml::ElementPtr InstanceElement(Rml::Element* parent, const Rml::String& tag,
                                    const Rml::XMLAttributes& attributes) override;
    void ReleaseElement(Rml::Element* element) override;

private:
    const ChartSeries* series_;
};

}  // namespace ftd::native::ui
