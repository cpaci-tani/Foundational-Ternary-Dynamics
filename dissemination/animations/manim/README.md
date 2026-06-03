# FTD Manim Animation Pack

This folder contains Manim source scenes for short FTD explainer animations.
The first pack is deliberately structural: it visualizes the model primitives and
engine cycle without upgrading epistemic claims.

## Storyboard

1. `TernaryVoxelLanguage`
   - Shows the two-layer ontology: continuous flux `J(v,t)` and discrete state
     `s(v,t) in {-1,0,+1}`.
   - Uses `[AXIOM]` framing for the ternary state vocabulary.

2. `MooreNeighborhoodLocality`
   - Shows the 3x3x3 Moore block and highlights that updates read the 26
     neighbors around a center voxel.
   - Uses `[AXIOM]` framing for local causality.

3. `EngineTickCycle`
   - Shows the engine tick as an instrumented cycle:
     `phase_read -> phase_write -> gauss_project -> phase_forces -> phase_movement`.
   - Uses `[ENGINE]` framing, not derivation language.

4. `FTDCoreTrailer`
   - A compact combined trailer scene for quick social/video previews.

## Rendering

These scenes were verified with Manim Community v0.19.0.

Render a quick preview with `uv`:

```powershell
uv run --no-project --with manim==0.19.0 python -m manim -pql --media_dir dissemination\media\manim dissemination\animations\manim\ftd_core_scenes.py FTDCoreTrailer
```

Or install Manim in an isolated local environment:

```powershell
python -m venv .venv-manim
.\.venv-manim\Scripts\python -m pip install -U pip
.\.venv-manim\Scripts\python -m pip install -r dissemination\animations\manim\requirements.txt
.\.venv-manim\Scripts\python -m manim -pql --media_dir dissemination\media\manim dissemination\animations\manim\ftd_core_scenes.py FTDCoreTrailer
```

Render individual high-quality scenes:

```powershell
.\.venv-manim\Scripts\python -m manim -pqh --media_dir dissemination\media\manim dissemination\animations\manim\ftd_core_scenes.py TernaryVoxelLanguage
.\.venv-manim\Scripts\python -m manim -pqh --media_dir dissemination\media\manim dissemination\animations\manim\ftd_core_scenes.py MooreNeighborhoodLocality
.\.venv-manim\Scripts\python -m manim -pqh --media_dir dissemination\media\manim dissemination\animations\manim\ftd_core_scenes.py EngineTickCycle
```

Generated videos are build artifacts and are ignored by the repository.
