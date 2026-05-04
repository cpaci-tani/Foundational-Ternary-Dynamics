"""Generate self-contained HTML visualization of Phase B.3 cluster dynamics.

Reads JSON output from `engine/build/Release/dump_visualization_data.exe`
and produces an HTML file with:
- 3D scatter of voxel coordinates at multiple ticks (interactive)
- Cluster size trajectories (n_total vs tick)
- Resonance landscape heatmap (A vs L)

Output: dissemination/interactive/phase_b3_cluster_dynamics.html

Uses Plotly via CDN — no Python plotly dependency needed.
"""
from __future__ import annotations
import json
import sys
from pathlib import Path

INPUT = Path("phase_b3_viz_data.json")
OUTPUT = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/phase_b3_cluster_dynamics.html")

def main():
    if not INPUT.exists():
        print(f"ERROR: {INPUT} not found. Run dump_visualization_data.exe > {INPUT} first.")
        sys.exit(1)
    with INPUT.open() as f:
        data = json.load(f)

    OUTPUT.parent.mkdir(exist_ok=True, parents=True)
    html = build_html(data)
    OUTPUT.write_text(html, encoding="utf-8")
    print(f"Wrote: {OUTPUT}")
    print(f"  Spatial snapshots: {len(data.get('spatial', {}))}")
    print(f"  Trajectories:      {len(data.get('trajectories', {}))}")
    print(f"  Resonance rows:    {len(data.get('resonance', {}))}")

def build_html(data: dict) -> str:
    return f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>FTD Phase B.3 — Cluster Dynamics Visualization</title>
<script src="https://cdn.plot.ly/plotly-2.35.2.min.js" charset="utf-8"></script>
<style>
  body {{ font-family: Georgia, serif; max-width: 1400px; margin: 24px auto; padding: 0 24px; background: #fafafa; color: #222; }}
  h1, h2, h3 {{ color: #1a3a52; }}
  .row {{ display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-bottom: 32px; }}
  .row3 {{ display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 16px; margin-bottom: 32px; }}
  .plot {{ background: white; border: 1px solid #ddd; padding: 8px; border-radius: 4px; }}
  .caption {{ font-size: 0.9em; color: #555; padding: 0 8px 8px; line-height: 1.4; }}
  .key-finding {{ background: #fff8e1; border-left: 4px solid #f9a825; padding: 12px 16px; margin: 16px 0; }}
  .retraction {{ background: #ffebee; border-left: 4px solid #c62828; padding: 12px 16px; margin: 16px 0; }}
  hr {{ border: 0; border-top: 1px solid #ccc; margin: 32px 0; }}
  code {{ background: #f0f0f0; padding: 2px 6px; border-radius: 3px; font-family: 'Consolas', monospace; }}
</style>
</head>
<body>

<h1>FTD Phase B.3 — Cluster Dynamics Visualization</h1>
<p>Direct visual rendering of the engine's actual output during the Phase B.3 boundary investigation.
Generated 2026-05-04. Uses <code>+color_forces +triad_binding</code> toggle config throughout.
Drag to rotate 3D plots; hover for values.</p>

<div class="key-finding">
<strong>What to look for:</strong> these visualizations directly show what the numerical summaries
abstracted away. Spatial geometry of the "n=8 stable" cluster reveals it's actually a multi-cluster
artifact (not a single 8-voxel BCC orbit). The flooding cascade looks dramatically different from
the matter-conserving soliton. The resonance landscape shows the user-intuited resonance windows
shifting with L.
</div>

<hr>

<h2>§1 — Spatial configurations: what the cluster actually IS</h2>

<p>The numerical summary "n=8 stable across L=32, 48, 64 at A=5.75" is misleading. Here's what
the actual voxel positions look like:</p>

<div class="row3">
  <div class="plot"><div id="spatial_n4"></div>
    <div class="caption"><strong>L=32, A=5.0 (stable n=4)</strong><br>The n=4 stable cluster at L=32. Should look
      like a partial SC face-axis configuration around lattice center (16,16,16): 4 of 6 face-adjacent neighbors lit.</div>
  </div>
  <div class="plot"><div id="spatial_n8_l32"></div>
    <div class="caption"><strong>L=32, A=5.75 ("n=8")</strong><br>The "L-invariant n=8 BREAKTHROUGH" candidate.
      Look for: are these 8 voxels at cube corners? Or are they split into multiple sub-clusters
      (some near center, some at lattice boundary)?</div>
  </div>
  <div class="plot"><div id="spatial_n8_l48"></div>
    <div class="caption"><strong>L=48, A=5.75</strong><br>Same amplitude, larger lattice. The "L-invariance"
      claim was that this should look the same as L=32. Does it?</div>
  </div>
</div>

<div class="row">
  <div class="plot"><div id="spatial_n8_l64"></div>
    <div class="caption"><strong>L=64, A=5.75</strong><br>L=64 confirmation. If the n=8 cluster were
      a real BCC corner orbit, all three L values should show 8 voxels at cube-corner positions.</div>
  </div>
  <div class="plot"><div id="spatial_a10_soliton"></div>
    <div class="caption"><strong>L=32, A=10 (SOLITON candidate)</strong><br>A=10 was claimed as a "soliton"
      with directed motion. Watch the cluster centroid drift across snapshots (50, 100, 200, 300 ticks).</div>
  </div>
</div>

<h3>The flooding cascade — A=7 deterministic flood-onset at tick 210</h3>
<p>This sequence visualizes the cleanest deterministic finding: A=7 +color+triad clusters persist
EXACTLY 210 ticks then cascade to flooding. Watch the transition between snapshots at tick 200 (still bound)
and tick 220-250 (cascade) and tick 400 (lattice flooded).</p>
<div class="row3">
  <div class="plot"><div id="spatial_a7_t50"></div><div class="caption"><strong>tick 50 — bound</strong></div></div>
  <div class="plot"><div id="spatial_a7_t200"></div><div class="caption"><strong>tick 200 — bound (just before flood)</strong></div></div>
  <div class="plot"><div id="spatial_a7_t220"></div><div class="caption"><strong>tick 220 — cascade onset</strong></div></div>
</div>
<div class="row3">
  <div class="plot"><div id="spatial_a7_t250"></div><div class="caption"><strong>tick 250 — cascade growing</strong></div></div>
  <div class="plot"><div id="spatial_a7_t400"></div><div class="caption"><strong>tick 400 — lattice flooded (~92%)</strong></div></div>
  <div class="plot"></div>
</div>

<hr>

<h2>§2 — Cluster size trajectories: see when it floods</h2>

<p>Cluster size <em>n_total</em> over time. Log-scale Y-axis to compare matter-conserving (n ≈ const)
to runaway flooding (n → 30,000+).</p>

<div class="plot"><div id="trajectories" style="height:500px;"></div>
  <div class="caption">
    Trajectories of total manifested voxel count over 600 ticks. <strong>Look for:</strong> the
    "stable" amplitudes (A=5, A=10) maintain near-constant n; the "transient" amplitudes (A=7,
    A=5.75) start near-constant then sharply transition to flooding around tick 210-300.
  </div>
</div>

<hr>

<h2>§3 — Resonance landscape: how stability shifts with L</h2>

<p>Heatmap of cluster size at end-of-run for each (A, L) combination. <strong>The user's intuition</strong>:
"stability resolves around resonance and resonance shifts with size". Look for: bands of
matter-conserving regimes (small <em>n</em>) shifting in A as L grows, separated by flooding
regions (n → 30,000+).</p>

<div class="plot"><div id="resonance_heatmap" style="height:500px;"></div>
  <div class="caption">
    Color = log₁₀(n_final). Dark = trivial bound (n=1) or stable (n ≈ 4-15). Bright yellow/red =
    flooded (n ≈ 30,000-240,000). The boundary between dark and bright traces the resonance windows.
  </div>
</div>

<div class="plot"><div id="resonance_lines" style="height:500px;"></div>
  <div class="caption">
    Same data shown as line plots: cluster size at end of run vs amplitude, one line per L value.
    The peaks (flooded amplitudes) and valleys (stable amplitudes) shift in A as L changes.
  </div>
</div>

<hr>

<h2>§4 — Engine self-portrait: what the toggle configs actually do</h2>

<div class="key-finding">
The cleanest observation from these visualizations: <strong>the engine doesn't produce localized
particle-like bound states</strong>. It produces matter-conserving propagating wavepackets ("solitons"),
metastable transients with deterministic flood timing, and runaway lattice nucleation. The visual
verification of multi-cluster splits at "n=8" was the key falsification of the "BCC corner orbit"
hypothesis — see §1 panels at L=32/48/64.
</div>

<script>
const data = {json.dumps(data, separators=(",", ":"))};

function makeScatter3D(divId, snap, L, title) {{
  const coords = snap.coords || [];
  if (coords.length === 0) {{
    Plotly.newPlot(divId, [{{type:'scatter3d', x:[],y:[],z:[],mode:'markers'}}],
      {{title:title+' (empty)', height:400, scene:{{xaxis:{{range:[0,L]}},yaxis:{{range:[0,L]}},zaxis:{{range:[0,L]}}}}}}, {{responsive:true}});
    return;
  }}
  const colors = coords.map(c => c.s > 0 ? '#1976d2' : '#d32f2f');
  Plotly.newPlot(divId, [{{
    type: 'scatter3d',
    x: coords.map(c => c.x),
    y: coords.map(c => c.y),
    z: coords.map(c => c.z),
    mode: 'markers',
    marker: {{size: 4, color: colors, opacity: 0.85}},
    text: coords.map(c => `(${{c.x}},${{c.y}},${{c.z}}) s=${{c.s}}`),
    hoverinfo: 'text'
  }}], {{
    title: title + ` (n=${{coords.length}})`,
    height: 400,
    margin: {{l:0,r:0,t:30,b:0}},
    scene: {{
      xaxis: {{range:[0,L], title:'x'}},
      yaxis: {{range:[0,L], title:'y'}},
      zaxis: {{range:[0,L], title:'z'}},
      aspectmode: 'cube'
    }}
  }}, {{responsive:true}});
}}

// §1 spatial snapshots
const sp = data.spatial || {{}};
function snapAt(key, tick) {{
  const entry = sp[key];
  if (!entry) return null;
  for (const s of entry.snapshots) if (s.tick === tick) return s;
  return entry.snapshots[entry.snapshots.length-1];
}}
function L_of(key) {{ return sp[key] ? sp[key].L : 32; }}

makeScatter3D('spatial_n4', snapAt('L32_A5_stable_n4', 50), L_of('L32_A5_stable_n4'),
              'L=32 A=5.0 t=50 (bound)');
makeScatter3D('spatial_n8_l32', snapAt('L32_A5p75_n8_artifact', 50), L_of('L32_A5p75_n8_artifact'),
              'L=32 A=5.75 t=50 (bound state)');
makeScatter3D('spatial_n8_l48', snapAt('L48_A5p75', 50), L_of('L48_A5p75'),
              'L=48 A=5.75 t=50 (bound state)');
makeScatter3D('spatial_n8_l64', snapAt('L64_A5p75', 50), L_of('L64_A5p75'),
              'L=64 A=5.75 t=50 (bound state)');
makeScatter3D('spatial_a10_soliton', snapAt('L32_A10_soliton', 100), L_of('L32_A10_soliton'),
              'L=32 A=10 t=100 (soliton)');

// A=7 cascade
makeScatter3D('spatial_a7_t50', snapAt('L32_A7_flood_cascade', 50), 32, 'A=7 t=50');
makeScatter3D('spatial_a7_t200', snapAt('L32_A7_flood_cascade', 200), 32, 'A=7 t=200');
makeScatter3D('spatial_a7_t220', snapAt('L32_A7_flood_cascade', 220), 32, 'A=7 t=220');
makeScatter3D('spatial_a7_t250', snapAt('L32_A7_flood_cascade', 250), 32, 'A=7 t=250');
makeScatter3D('spatial_a7_t400', snapAt('L32_A7_flood_cascade', 400), 32, 'A=7 t=400');

// §2 trajectories
const traj = data.trajectories || {{}};
const trajTraces = [];
for (const [key, entry] of Object.entries(traj)) {{
  trajTraces.push({{
    x: entry.trajectory.map(p => p[0]),
    y: entry.trajectory.map(p => p[1]),
    mode: 'lines',
    name: `L=${{entry.L}} A=${{entry.A}}`,
    line: {{width: 2}}
  }});
}}
Plotly.newPlot('trajectories', trajTraces, {{
  title: 'Cluster size n_total over time',
  xaxis: {{title: 'tick'}},
  yaxis: {{title: 'n_manifested', type: 'log', range: [0, 6]}},
  height: 500,
  hovermode: 'x unified'
}}, {{responsive:true}});

// §3 resonance heatmap
const res = data.resonance || {{}};
const Ls = [32, 48, 64];
const A_vals = (res.L_32 || []).map(r => r.A);
const Z = Ls.map(L => {{
  const row = res[`L_${{L}}`] || [];
  return row.map(r => Math.log10(Math.max(1, r.n_final)));
}});

Plotly.newPlot('resonance_heatmap', [{{
  type: 'heatmap',
  x: A_vals,
  y: Ls,
  z: Z,
  colorscale: 'Viridis',
  colorbar: {{title: 'log₁₀(n_final)'}}
}}], {{
  title: 'Resonance landscape: cluster size at tick 300 vs (A, L)',
  xaxis: {{title: 'A / K_GENESIS'}},
  yaxis: {{title: 'L', tickvals: Ls}},
  height: 500
}}, {{responsive:true}});

// §3 line plot
const resTraces = [];
for (const L of Ls) {{
  const row = res[`L_${{L}}`] || [];
  resTraces.push({{
    x: row.map(r => r.A),
    y: row.map(r => Math.max(1, r.n_final)),
    mode: 'lines+markers',
    name: `L=${{L}}`
  }});
}}
Plotly.newPlot('resonance_lines', resTraces, {{
  title: 'Cluster size at tick 300 vs amplitude (per L)',
  xaxis: {{title: 'A / K_GENESIS'}},
  yaxis: {{title: 'n_final', type: 'log'}},
  height: 500,
  hovermode: 'x unified'
}}, {{responsive:true}});
</script>

</body>
</html>
"""

if __name__ == "__main__":
    main()
