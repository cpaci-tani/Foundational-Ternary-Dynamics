# Copilot instructions (pbr_pedagogy)

## Big picture
- This repo mixes **an interactive simulation app** (monorepo under `packages/`) with **a publication pipeline** (`manuscript/` Quarto book) and **research/verification scripts** (`simulations/`).
- The authoritative long-form model/spec is in `CLAUDE.md`; the publication-ready narrative lives in `manuscript/`.
- `symmetry-of-zero/` is a **separate side project** (Next.js) and is not part of the main `packages/*` app.

## Key subsystems & where to edit
- **Frontend (Vite + React + R3F/Three)**: `packages/frontend/`
  - Chapter visuals live in `packages/frontend/src/scenes/` and reusable 3D bits in `packages/frontend/src/components/scene/`.
  - Chapter → specialized scene routing is data-driven via `packages/frontend/src/data/sceneRegistry.ts` and used by `packages/frontend/src/scenes/ChapterScene.tsx`.
  - Real-time sim data arrives over WebSocket; client parsing is in `packages/frontend/src/hooks/useWebSocket.ts`.
- **Backend (FastAPI + WebSocket stream)**: `packages/backend/`
  - App entrypoint is `packages/backend/main.py` (WS at `/ws`, content loaded from top-level `content/`).
  - The current physics engine is a Python bridge in `packages/backend/simulation/physics_engine.py`.
- **Physics core (Rust → WASM)**: `packages/physics-core/`
  - WASM entrypoints are in `packages/physics-core/src/lib.rs` (exports `create_universe`).
- **Content layer (JSON + schemas)**: `content/` and `schemas/`
  - Content is structured JSON (chapters/concepts/experiments/narration) and is loaded by `packages/backend/simulation/content_loader.py`.
  - Prefer updating schemas + audit scripts when changing content shape.
- **Manuscript (Quarto book)**: `manuscript/` (see `manuscript/_quarto.yml`).

## Critical workflows (known-good commands)
- Install everything: `npm run install:all`
- Run app (frontend + backend): `npm run dev` (frontend http://localhost:3000, backend http://localhost:8000)
- Build WASM + frontend: `npm run build` (or `npm run build:physics`)
- Build the manuscript (Quarto): `cd manuscript && quarto render`
- Preview the manuscript locally: `cd manuscript && quarto preview`
- Frontend tests: `cd packages/frontend && npm test` (Vitest) and `npm run test:e2e` (Playwright)
- Backend tests: `cd packages/backend && pytest`
- Physics-core tests: `cd packages/physics-core && cargo test`

## Side project: symmetry-of-zero
- Run the Next.js app: `cd symmetry-of-zero && npm install && npm run dev`

## Repo-specific conventions to follow
- **Chapter IDs** are string-like decimals (`"1.2"`, `"8.1"`) and are used across routing + content.
- **Scene routing**: add/update mappings in `packages/frontend/src/data/sceneRegistry.ts` rather than reintroducing large switches.
- **Playback/time**: chapter time comes from the Director system (`packages/frontend/src/director/*`) and is consumed by `ChapterScene` via `sceneTime`.
- **WebSocket binary protocol is coupled**:
  - Frontend expects the binary layout documented in `packages/frontend/src/hooks/useWebSocket.ts`.
  - If you change backend particle serialization, update the frontend parser in lockstep.
  - WS URL is configurable via `VITE_WS_URL` (defaults to `ws://localhost:8000/ws`).

## Content tooling
- Validate cross-references and required fields with `python scripts/audit_json.py`.
- Generate missing stubs (books/experiments/concepts) with `python scripts/generate_missing_content.py`.
