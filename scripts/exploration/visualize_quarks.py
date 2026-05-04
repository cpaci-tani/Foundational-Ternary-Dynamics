"""Generate HTML visualization of quark/color behavior on the FTD lattice.

Reads JSON output from `engine/build/Release/dump_quark_data.exe` and produces
interactive 3D scatter visualization with voxels color-coded by their `color`
field (R/G/B per Moore Layer Theorem).
"""
from __future__ import annotations
import json
from pathlib import Path

INPUT = Path("quark_viz_data.json")
OUTPUT = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/phase_b3_quarks.html")

def main():
    with INPUT.open() as f:
        data = json.load(f)
    OUTPUT.parent.mkdir(exist_ok=True, parents=True)
    OUTPUT.write_text(build_html(data), encoding="utf-8")
    print(f"Wrote: {OUTPUT}")
    print(f"  Experiments: {len(data['experiments'])}")

def build_html(data: dict) -> str:
    return f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>FTD Quark/Color Behavior on Lattice</title>
<script src="https://cdn.plot.ly/plotly-2.35.2.min.js" charset="utf-8"></script>
<style>
  body {{ font-family: Georgia, serif; max-width: 1500px; margin: 24px auto; padding: 0 24px; background: #fafafa; color: #222; line-height: 1.5; }}
  h1, h2, h3 {{ color: #1a3a52; }}
  .row {{ display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 16px; margin-bottom: 32px; }}
  .row2 {{ display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-bottom: 32px; }}
  .row4 {{ display: grid; grid-template-columns: 1fr 1fr 1fr 1fr; gap: 12px; margin-bottom: 24px; }}
  .plot {{ background: white; border: 1px solid #ddd; padding: 8px; border-radius: 4px; }}
  .caption {{ font-size: 0.9em; color: #555; padding: 0 8px 8px; line-height: 1.4; }}
  .key-finding {{ background: #fff8e1; border-left: 4px solid #f9a825; padding: 12px 16px; margin: 16px 0; }}
  .surprise {{ background: #e8f5e9; border-left: 4px solid #43a047; padding: 12px 16px; margin: 16px 0; }}
  hr {{ border: 0; border-top: 1px solid #ccc; margin: 32px 0; }}
  .legend {{ display: flex; gap: 16px; align-items: center; padding: 8px 16px; background: white; border: 1px solid #ddd; border-radius: 4px; margin-bottom: 16px; }}
  .swatch {{ display: inline-block; width: 14px; height: 14px; border-radius: 50%; margin-right: 4px; vertical-align: middle; }}
  .red {{ background: #e53935; }} .green {{ background: #43a047; }} .blue {{ background: #1e88e5; }} .gray {{ background: #888; }}
</style>
</head>
<body>

<h1>FTD Quark / Color Behavior on the Lattice</h1>
<p>Each voxel carries a color ∈ {{0=colorless, 1=R, 2=G, 3=B}} assigned at genesis from the dominant flux axis
(per Moore Layer Theorem: 3 colors = 3 spatial dimensions). This visualization shows direct engine output
of quark-like and baryon-like configurations under various toggle settings.</p>

<div class="legend">
  <div><span class="swatch red"></span><strong>R</strong> (color=1, flux dominant in x)</div>
  <div><span class="swatch green"></span><strong>G</strong> (color=2, flux dominant in y)</div>
  <div><span class="swatch blue"></span><strong>B</strong> (color=3, flux dominant in z)</div>
  <div><span class="swatch gray"></span>colorless</div>
  <div>● = matter (s=+1), ◆ = antimatter (s=-1)</div>
</div>

<div class="surprise">
<strong>Key surprise from this data:</strong> the engine is NOT color-symmetric despite the Moore
Layer Theorem stating R/G/B should be equivalent (3 colors ↔ 3 spatial axes). Pure +x flux gives
n=4 stable cluster (R), pure +y gives n=2 (G), pure +z gives n=3 (B). The engine's update order
or dual-substrate L/R chirality breaks color symmetry at the dynamics level.
</div>

<h2>§1 — Single-color quarks</h2>
<p>Inject pure flux along one axis. The dominant component determines color.</p>

<div class="row">
  <div class="plot"><div id="red_quark" style="height:450px;"></div>
    <div class="caption"><strong>R quark</strong>: pure +x flux at center, A=5·K_GENESIS.
      Settles to n=4 stable (4 manifested voxels around lattice center).</div>
  </div>
  <div class="plot"><div id="green_quark" style="height:450px;"></div>
    <div class="caption"><strong>G quark</strong>: pure +y flux. Settles to <strong>n=2</strong>.
      Smaller than R! Engine asymmetry.</div>
  </div>
  <div class="plot"><div id="blue_quark" style="height:450px;"></div>
    <div class="caption"><strong>B quark</strong>: pure +z flux. Settles to <strong>n=3</strong>.
      Different again!</div>
  </div>
</div>

<h3>Time evolution of R quark</h3>
<div class="row4">
  <div class="plot"><div id="red_t0" style="height:280px;"></div><div class="caption">t=0 (post-injection)</div></div>
  <div class="plot"><div id="red_t30" style="height:280px;"></div><div class="caption">t=30</div></div>
  <div class="plot"><div id="red_t100" style="height:280px;"></div><div class="caption">t=100</div></div>
  <div class="plot"><div id="red_t300" style="height:280px;"></div><div class="caption">t=300</div></div>
</div>

<hr>

<h2>§2 — Diagonal flux: which color wins?</h2>
<p>Inject flux along x=y (45° in xy-plane). The competition determines color assignment.</p>

<div class="row2">
  <div class="plot"><div id="diagonal_xy" style="height:450px;"></div>
    <div class="caption"><strong>Diagonal +x+y flux</strong>: when fx = fy, which axis wins?
      Engine picks first (x → R, by genesis logic <code>if (fx ≥ fy && fx ≥ fz) color=1</code>).
      Settles to single voxel (n=1) — flux too distributed for nucleation.</div>
  </div>
  <div class="plot"><div id="rgb_symmetric" style="height:450px;"></div>
    <div class="caption"><strong>Symmetric +x+y+z flux</strong>: R/G/B amplitudes equal at each component.
      Same as diagonal — first axis (x) wins, n=4 stable. Identical to pure-R configuration.</div>
  </div>
</div>

<hr>

<h2>§3 — Quark-antiquark "meson" (R + R̄)</h2>

<div class="row2">
  <div class="plot"><div id="meson_RRbar" style="height:450px;"></div>
    <div class="caption"><strong>R quark (matter) at +x; R-antiquark (antimatter) at -x</strong>,
      separated by 6 voxels. They persist (n=11 at t=300) without annihilating. Watch the cluster
      stretch between them — the engine's analog of meson confinement?</div>
  </div>
  <div class="plot"><div id="two_R_quarks" style="height:450px;"></div>
    <div class="caption"><strong>Two same-color R quarks</strong> (both matter), separated by 6 voxels.
      Stable at n=21 — cluster halos overlap and persist. Different from meson because no antiquark.</div>
  </div>
</div>

<hr>

<h2>§4 — Three-quark "baryon" (R + G + B)</h2>

<p>The natural FTD baryon analog: three quarks with different colors at vertices of a triangle.
The total is "color-neutral" (R+G+B). Three toggle configurations:</p>

<div class="row">
  <div class="plot"><div id="baryon_RGB" style="height:450px;"></div>
    <div class="caption"><strong>RGB + color_forces (default ON)</strong>:
      <strong>FLOODS to n=29,613</strong> by t=300. The three-quark configuration triggers cascade
      under color_forces. Color "confinement" doesn't bind here — it destabilizes.</div>
  </div>
  <div class="plot"><div id="baryon_RGB_with_strong" style="height:450px;"></div>
    <div class="caption"><strong>RGB + color_forces + strong_force</strong>:
      Strong-force toggle DECAYS to n=2. The Yukawa short-range force annihilates the cluster.</div>
  </div>
  <div class="plot"><div id="baryon_RGB_no_color_forces" style="height:450px;"></div>
    <div class="caption"><strong>RGB without color_forces (engine defaults)</strong>:
      Collapses to n=1 (single voxel persists). Without color_forces, the 3 separate quarks
      cannot maintain themselves.</div>
  </div>
</div>

<hr>

<h2>§5 — What this data shows</h2>

<div class="key-finding">
<strong>Most striking visual finding</strong>: the R/G/B cluster sizes are different (4, 2, 3) for
identical-magnitude flux injections along different axes. This breaks the supposed "3 colors = 3
spatial axes" symmetry of the Moore Layer Theorem at the engine dynamics level. Possible causes:
<ul>
  <li>The 18-point Laplacian stencil's coefficients differ across (face, edge, corner) neighbor sets</li>
  <li>The dual-substrate (L/R chirality) interacts asymmetrically with color assignment</li>
  <li>The genesis tie-breaker rule <code>if (fx ≥ fy)</code> systematically favors x</li>
</ul>
This is a real engine-side asymmetry that should be flagged. The Moore Layer Theorem might be
stating an ALGEBRAIC equivalence that the dynamical engine breaks.
</div>

<div class="key-finding">
<strong>Second striking finding</strong>: the "baryon RGB" with color_forces FLOODS to 30k voxels.
The naive expectation was that R+G+B would be color-neutral and bind tightly (analog of nuclear
binding). Instead, the color_forces toggle <em>destabilizes</em> the three-quark configuration.
This suggests color_forces in the engine is more like color-CHARGE-COUPLING (drives expansion)
than color-CONFINEMENT (drives binding).
</div>

<script>
const data = {json.dumps(data, separators=(",", ":"))};

const COLOR_MAP = {{
  0: '#888888',  // colorless
  1: '#e53935',  // R
  2: '#43a047',  // G
  3: '#1e88e5'   // B
}};

function plotVoxels(divId, snap, L, title) {{
  const coords = (snap && snap.coords) || [];
  if (coords.length === 0) {{
    Plotly.newPlot(divId, [{{type:'scatter3d',x:[],y:[],z:[],mode:'markers'}}],
      {{title: title+' (empty)', height:450, scene:{{xaxis:{{range:[0,L]}},yaxis:{{range:[0,L]}},zaxis:{{range:[0,L]}}}}}},
      {{responsive:true}});
    return;
  }}
  // Group by color × state
  const groups = {{}};
  for (const c of coords) {{
    const key = c.s + '_' + c.c;
    if (!groups[key]) groups[key] = {{x:[], y:[], z:[], hover:[]}};
    groups[key].x.push(c.x);
    groups[key].y.push(c.y);
    groups[key].z.push(c.z);
    groups[key].hover.push(`(${{c.x}},${{c.y}},${{c.z}}) s=${{c.s}} c=${{c.c}}<br>flux=(${{c.fx.toFixed(2)}},${{c.fy.toFixed(2)}},${{c.fz.toFixed(2)}})`);
  }}
  const traces = [];
  for (const [key, g] of Object.entries(groups)) {{
    const [s, c] = key.split('_').map(Number);
    const colorName = ['','R','G','B'][c] || 'colorless';
    const stateName = s > 0 ? 'q' : 'q̄';
    const symbol = s > 0 ? 'circle' : 'diamond';
    traces.push({{
      type:'scatter3d',
      x: g.x, y: g.y, z: g.z,
      mode: 'markers',
      marker: {{
        size: coords.length < 50 ? 8 : (coords.length < 500 ? 4 : 2),
        color: COLOR_MAP[c],
        symbol: symbol,
        opacity: 0.85,
        line: {{color: '#222', width: 0.5}}
      }},
      text: g.hover,
      hoverinfo: 'text',
      name: `${{colorName}} ${{stateName}} (n=${{g.x.length}})`
    }});
  }}
  Plotly.newPlot(divId, traces, {{
    title: title + ` (n=${{coords.length}})`,
    height: 450,
    margin: {{l:0,r:0,t:30,b:0}},
    scene: {{
      xaxis: {{range:[0,L], title:'x'}},
      yaxis: {{range:[0,L], title:'y'}},
      zaxis: {{range:[0,L], title:'z'}},
      aspectmode: 'cube'
    }},
    showlegend: true,
    legend: {{x: 0, y: 1, font: {{size: 10}}}}
  }}, {{responsive:true}});
}}

const exp = data.experiments || {{}};
function snapAt(key, tick) {{
  const e = exp[key];
  if (!e) return null;
  for (const s of e.snapshots) if (s.tick === tick) return s;
  return e.snapshots[e.snapshots.length-1];
}}
function L_of(key) {{ return exp[key] ? exp[key].L : 32; }}

plotVoxels('red_quark', snapAt('red_quark', 300), L_of('red_quark'), 'R quark (pure +x flux) t=300');
plotVoxels('green_quark', snapAt('green_quark', 300), L_of('green_quark'), 'G quark (pure +y flux) t=300');
plotVoxels('blue_quark', snapAt('blue_quark', 300), L_of('blue_quark'), 'B quark (pure +z flux) t=300');

plotVoxels('red_t0', snapAt('red_quark', 0), 32, 't=0');
plotVoxels('red_t30', snapAt('red_quark', 30), 32, 't=30');
plotVoxels('red_t100', snapAt('red_quark', 100), 32, 't=100');
plotVoxels('red_t300', snapAt('red_quark', 300), 32, 't=300');

plotVoxels('diagonal_xy', snapAt('diagonal_xy', 300), 32, 'Diagonal +x+y t=300');
plotVoxels('rgb_symmetric', snapAt('rgb_symmetric', 300), 32, 'Symmetric +x+y+z t=300');

plotVoxels('meson_RRbar', snapAt('meson_RRbar', 300), 32, 'R + R̄ meson t=300');
plotVoxels('two_R_quarks', snapAt('two_R_quarks', 300), 32, 'Two R quarks t=300');

plotVoxels('baryon_RGB', snapAt('baryon_RGB', 300), 32, 'RGB baryon + color_forces t=300');
plotVoxels('baryon_RGB_with_strong', snapAt('baryon_RGB_with_strong', 300), 32, 'RGB + color + strong t=300');
plotVoxels('baryon_RGB_no_color_forces', snapAt('baryon_RGB_no_color_forces', 300), 32, 'RGB defaults only t=300');
</script>

</body>
</html>
"""

if __name__ == "__main__":
    main()
