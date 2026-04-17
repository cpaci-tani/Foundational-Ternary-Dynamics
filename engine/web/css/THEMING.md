# FTD Dashboard Theming & UI Principles

This document formalizes the interactive design aesthetic and structural guidelines for the `engine/web/css` directory. Every visual element within the HUD layer floating above the 3D `<canvas>` must conform to these physical-metaphor principles.

## 1. Spatial Glassmorphism (Z-Depth Layering)
The primary aesthetic is "Spatial Glassmorphism." Because the simulation engine is generating a dark expanse of deep space (or high-contrast physical geometries), the UI HUD relies on translucent, frosted plates rather than opaque, flat color overlays. 

### Core Mechanics
*   **The Backdrop Filter:** All floating UI panels (e.g. `viewport-overlays.css`, `workspace-tabs.css`) MUST utilize a `backdrop-filter: blur() saturate()` rule.
*   **Blur Thresholds:**
    *   **Low tier (`blur(8px)`)**: Intended for small, transient wrappers (e.g. tooltip ribbons, segmented control boundaries).
    *   **Mid tier (`blur(16px)`)**: Intended for overlay panels and main viewport control grilles.
    *   **High tier (`blur(24px)`)**: Reserved for application-level shell containers like the settings modal or large data-inspection panels.
*   **Borders defining planes**: Glass UI depends entirely on subpixel edge reflection. Every frosted panel must pair with an inset, subtle translucent border (e.g., `border: 1px solid rgba(255, 255, 255, 0.1)`) accompanied by an organic drop-shadow to separate the glass plane from the 3D camera bounds.

## 2. Dynamic Interaction & Micro-Animations
The application must feel "alive" and instantaneously responsive to data flows and user interactions, adhering to the wow-factor mandated by modern dynamic designs.

*   **Elevating the Z-Axis:** Elements like the `.tb-btn` buttons, `cards`, or sliders do not just change background colors on state `:hover`. They must visibly lift towards the user `transform: translateY(-2px)` to indicate interactivity mapped down into a `var(--shadow-md)` shadow escalation.
*   **Active State Glowing:** Using the standard accent token (`var(--accent-glow)`), active selection states require a diffuse box-shadow perimeter glow. 
*   **Transition Phrasing:** The `tokens.css` transition variables map everything linearly. All interactive elements must bind to `transition: all var(--dur-fast) var(--ease-out)` ensuring snap elasticity rather than linear fade lagging.

## 3. Typography & Hierarchy
All raw string text must be formatted under a structured variable tree governed by high-readability screen fonts.
*   **Primary Headings**: Defaults to the premium `var(--font-heading)` (`Outfit`, `Inter`).
*   **UI Readouts**: Scientific numbers use the specific `var(--font-mono)` configuration (`JetBrains Mono`, `Fira Code`, `Cascadia`) ensuring tabular lining alignment so telemetry diagnostics do not jitter every frame.

## 4. Theme Inheritance Model (The Ladder)
The visual schema cascades through a specific hierarchy defined inside `tokens.css`:
1.  **Deep Backgrounds** (`--bg-deep`)
2.  **Surfaces** (`--bg-surface` for primary glass)
3.  **Elevated/Raised Elements** (`--bg-card`, `--bg-elevated` for modals and interactive blocks)

When writing new component files inside `css/ui/components/` or `css/ui/primitives/`, **you must never hardcode hex colors**. Always map to the semantic ladders defined under `:root`. This guarantees flawless light-mode flips and high-contrast accessibility swaps transparently.
