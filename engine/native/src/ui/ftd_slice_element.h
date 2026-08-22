// FtdSliceElement — a custom RmlUi element (<ftd-slice>) that draws a 2D field
// slice as a colour-mapped heatmap, through the engine's own D3D12 UI pipeline.
// It is the 2D companion to <ftd-chart>: same instancer + id-keyed registry +
// MakeGeometry/Geometry::Render pattern, but it emits one colour-mapped quad per
// grid cell instead of polylines.
//
// Data contract (snapshot-only, GUI thread): the element never reads the engine.
// The app owns a SliceGrid (a W×H float buffer filled from the Scale-0 snapshot's
// adapter-computed field slice) and binds it into a SliceRegistry keyed by the
// element's `id`; the element resolves its own grid at render time. Read-only.
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

// A W×H scalar grid + its value range, owned by the app on the GUI thread and read
// read-only by the slice element. `generation()` bumps on every set()/clear() so
// the element can skip rebuilding an unchanged mesh.
class SliceGrid {
public:
    // Replace the grid from a contiguous row-major (w*h) float buffer + its range.
    void set(int w, int h, const float* data, float mn, float mx) {
        w_ = (w > 0) ? w : 0;
        h_ = (h > 0) ? h : 0;
        mn_ = mn;
        mx_ = mx;
        const std::size_t n = static_cast<std::size_t>(w_) * static_cast<std::size_t>(h_);
        data_.assign(data, data + (data ? n : 0));
        ++generation_;
    }
    void clear() { w_ = h_ = 0; data_.clear(); ++generation_; }

    int w() const { return w_; }
    int h() const { return h_; }
    float mn() const { return mn_; }
    float mx() const { return mx_; }
    bool empty() const { return data_.empty() || w_ <= 0 || h_ <= 0; }
    std::uint64_t generation() const { return generation_; }
    // Cell value; caller keeps i in [0,w), j in [0,h).
    float at(int i, int j) const {
        return data_[static_cast<std::size_t>(j) * static_cast<std::size_t>(w_) + i];
    }

private:
    int w_ = 0, h_ = 0;
    float mn_ = 0.0f, mx_ = 0.0f;
    std::vector<float> data_;
    std::uint64_t generation_ = 0;
};

// One <ftd-slice>'s data source: an app-owned grid (may be null → blank) + a
// colormap id (0 sequential / 1 diverging — see ftd_slice_element.cpp::colormap).
struct SliceBinding {
    const SliceGrid* grid = nullptr;
    int ramp = 0;
};

// Maps a <ftd-slice>'s `id` → its binding. The app fills it once at setup; each
// element resolves its own binding by GetId() at render time.
class SliceRegistry {
public:
    SliceBinding& binding(const Rml::String& id) { return map_[id]; }
    const SliceBinding* find(const Rml::String& id) const {
        const auto it = map_.find(id);
        return it == map_.end() ? nullptr : &it->second;
    }

private:
    std::unordered_map<Rml::String, SliceBinding> map_;
};

// The <ftd-slice> element: resolves its grid from `registry_` by the element's
// `id`, then fills its content box with a W×H colour-mapped heatmap. A null
// registry, unknown id, or empty grid draws a faint placeholder — so the element
// is safe to instance without data (the headless smoke test).
class FtdSliceElement : public Rml::Element {
public:
    FtdSliceElement(const Rml::String& tag, const SliceRegistry* registry);
    void set_registry(const SliceRegistry* registry);

protected:
    void OnRender() override;

private:
    void rebuild_geometry(Rml::Vector2f origin, Rml::Vector2f size,
                          const SliceBinding* binding);

    const SliceRegistry* registry_ = nullptr;
    Rml::Geometry geometry_;
    bool built_ = false;
    std::uint64_t built_signature_ = 0;
    float built_w_ = -1.0f;
    float built_h_ = -1.0f;
};

class FtdSliceInstancer : public Rml::ElementInstancer {
public:
    explicit FtdSliceInstancer(const SliceRegistry* registry) : registry_(registry) {}
    Rml::ElementPtr InstanceElement(Rml::Element* parent, const Rml::String& tag,
                                    const Rml::XMLAttributes& attributes) override;
    void ReleaseElement(Rml::Element* element) override;

private:
    const SliceRegistry* registry_;
};

}  // namespace ftd::native::ui
