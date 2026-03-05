#!/usr/bin/env python3
"""
Interactive 3D Lemniscate-Alpha with Five Sacred Harmonics + Audio Tone Generators

The Lemniscate-Alpha curve extended into 3D with interactive audio controls
to hear each of the five sacred frequencies.

Uses Plotly for 3D visualization and Web Audio API for tone generation.
"""

import numpy as np
import plotly.graph_objects as go
from math import gamma

# =============================================================================
# LEMNISCATE-ALPHA CURVE DEFINITION
# =============================================================================

FREQS = np.array([1, 2, 4, 8, 16])
X_AMPS = np.array([1.0, 0.5, 0.5, 2/5, 1/16])
Y_AMPS = np.array([1.0, -0.5, 0.5, -7/20, 1/16])

G_STAR = (np.sqrt(2) * gamma(0.25)**2) / (2 * np.pi)
ALPHA = 1/137.036
PHI = (1 + np.sqrt(5)) / 2

HARMONICS = {
    'Schumann': {'freq': 7.83, 'color': '#8B5CF6', 'z_offset': 0, 'desc': 'Earth resonance - binaural beat base'},
    'Pyramid': {'freq': 110, 'color': '#EC4899', 'z_offset': 1, 'desc': 'Chamber resonance - A2 note'},
    'OM': {'freq': 136.1, 'color': '#F97316', 'z_offset': 2, 'desc': 'Cosmic OM - C#3 approximation'},
    'Natural_A': {'freq': 432, 'color': '#EAB308', 'z_offset': 3, 'desc': 'Verdi tuning - A4'},
    'Miracle': {'freq': 528, 'color': '#22C55E', 'z_offset': 4, 'desc': 'Solfeggio MI - C5 approximation'},
}

def lemniscate_alpha(t, scale=1.0):
    x = np.zeros_like(t)
    y = np.zeros_like(t)
    for j in range(5):
        x += X_AMPS[j] * np.cos(FREQS[j] * t)
        y += Y_AMPS[j] * np.sin(FREQS[j] * t)
    return x * scale, y * scale

def modulate_curve_3d(t, harmonic_freq, z_base=0, mod_amplitude=0.15):
    x, y = lemniscate_alpha(t, scale=1.0)
    normalized_freq = harmonic_freq / 7.83
    z = z_base + mod_amplitude * np.sin(normalized_freq * t)
    return x, y, z

def create_audio_html():
    """Generate the HTML with embedded audio controls using Web Audio API."""

    # Color mapping for buttons
    colors = {
        'Schumann': '#8B5CF6',
        'Pyramid': '#EC4899',
        'OM': '#F97316',
        'Natural_A': '#EAB308',
        'Miracle': '#22C55E'
    }

    html = """
<!DOCTYPE html>
<html>
<head>
    <title>Lemniscate-Alpha: Five Sacred Harmonics</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            background: #0d1117;
            color: white;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            min-height: 100vh;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 {
            text-align: center;
            margin-bottom: 5px;
            font-size: 28px;
        }
        .subtitle {
            text-align: center;
            color: #8b949e;
            font-family: monospace;
            margin-bottom: 20px;
            font-size: 14px;
        }
        .audio-panel {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .audio-panel h2 {
            font-size: 18px;
            margin-bottom: 15px;
            color: #c9d1d9;
        }
        .tone-buttons {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 15px;
        }
        .tone-btn {
            padding: 12px 20px;
            border: 2px solid;
            border-radius: 8px;
            background: transparent;
            color: white;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.2s;
            min-width: 140px;
        }
        .tone-btn:hover {
            transform: scale(1.05);
        }
        .tone-btn.active {
            color: #0d1117;
        }
        .tone-btn.schumann { border-color: #8B5CF6; }
        .tone-btn.schumann:hover, .tone-btn.schumann.active { background: #8B5CF6; }
        .tone-btn.pyramid { border-color: #EC4899; }
        .tone-btn.pyramid:hover, .tone-btn.pyramid.active { background: #EC4899; }
        .tone-btn.om { border-color: #F97316; }
        .tone-btn.om:hover, .tone-btn.om.active { background: #F97316; }
        .tone-btn.natural-a { border-color: #EAB308; }
        .tone-btn.natural-a:hover, .tone-btn.natural-a.active { background: #EAB308; }
        .tone-btn.miracle { border-color: #22C55E; }
        .tone-btn.miracle:hover, .tone-btn.miracle.active { background: #22C55E; }

        .stop-btn {
            padding: 12px 30px;
            border: 2px solid #f85149;
            border-radius: 8px;
            background: transparent;
            color: #f85149;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.2s;
        }
        .stop-btn:hover {
            background: #f85149;
            color: white;
        }

        .chord-btn {
            padding: 12px 20px;
            border: 2px solid #58a6ff;
            border-radius: 8px;
            background: transparent;
            color: #58a6ff;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.2s;
        }
        .chord-btn:hover {
            background: #58a6ff;
            color: #0d1117;
        }

        .volume-control {
            display: flex;
            align-items: center;
            gap: 15px;
            margin-top: 15px;
        }
        .volume-control label {
            color: #8b949e;
            font-size: 14px;
        }
        .volume-slider {
            -webkit-appearance: none;
            width: 200px;
            height: 6px;
            border-radius: 3px;
            background: #30363d;
            outline: none;
        }
        .volume-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: #58a6ff;
            cursor: pointer;
        }

        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 10px;
            margin-top: 15px;
        }
        .info-card {
            background: #0d1117;
            border: 1px solid #30363d;
            border-radius: 8px;
            padding: 12px;
            font-size: 13px;
        }
        .info-card .freq {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 5px;
        }
        .info-card .desc {
            color: #8b949e;
        }
        .info-card.schumann .freq { color: #8B5CF6; }
        .info-card.pyramid .freq { color: #EC4899; }
        .info-card.om .freq { color: #F97316; }
        .info-card.natural-a .freq { color: #EAB308; }
        .info-card.miracle .freq { color: #22C55E; }

        #plotly-container {
            width: 100%;
            height: 600px;
            border-radius: 12px;
            overflow: hidden;
        }

        .waveform-indicator {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-top: 10px;
        }
        .wave-type {
            padding: 6px 12px;
            border: 1px solid #30363d;
            border-radius: 4px;
            background: transparent;
            color: #8b949e;
            cursor: pointer;
            font-size: 12px;
        }
        .wave-type.active {
            background: #30363d;
            color: white;
        }

        .now-playing {
            background: #1f2937;
            border-radius: 8px;
            padding: 10px 15px;
            margin-top: 15px;
            display: none;
        }
        .now-playing.visible {
            display: block;
        }
        .now-playing-text {
            font-size: 14px;
            color: #9ca3af;
        }
        .now-playing-freq {
            font-size: 24px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>The Lemniscate-Alpha with Five Sacred Harmonics</h1>
        <p class="subtitle">G* = """ + f"{G_STAR:.4f}" + """ | 1/α = 137.036 | φ = """ + f"{PHI:.4f}" + """</p>

        <div class="audio-panel">
            <h2>🎵 Tone Generator</h2>
            <div class="tone-buttons">
                <button class="tone-btn schumann" onclick="playTone(7.83, 'Schumann', this)">
                    Schumann<br><small>7.83 Hz</small>
                </button>
                <button class="tone-btn pyramid" onclick="playTone(110, 'Pyramid', this)">
                    Pyramid<br><small>110 Hz</small>
                </button>
                <button class="tone-btn om" onclick="playTone(136.1, 'OM', this)">
                    OM<br><small>136.1 Hz</small>
                </button>
                <button class="tone-btn natural-a" onclick="playTone(432, '432 Hz', this)">
                    Natural A<br><small>432 Hz</small>
                </button>
                <button class="tone-btn miracle" onclick="playTone(528, 'Miracle', this)">
                    Miracle<br><small>528 Hz</small>
                </button>
            </div>

            <div class="tone-buttons">
                <button class="chord-btn" onclick="playChord([110, 136.1, 432])">
                    Play Triad (110 + 136 + 432)
                </button>
                <button class="chord-btn" onclick="playChord([7.83, 110, 136.1, 432, 528])">
                    Play All Five
                </button>
                <button class="chord-btn" onclick="playBinauralBeat()">
                    Binaural Beat (432 ± 7.83)
                </button>
                <button class="stop-btn" onclick="stopAll()">
                    ⬛ Stop All
                </button>
            </div>

            <div class="volume-control">
                <label>Volume:</label>
                <input type="range" class="volume-slider" id="volume" min="0" max="100" value="30" onchange="updateVolume(this.value)">
                <span id="volume-display">30%</span>
            </div>

            <div class="waveform-indicator">
                <span style="color: #8b949e; font-size: 13px;">Waveform:</span>
                <button class="wave-type active" onclick="setWaveform('sine', this)">Sine</button>
                <button class="wave-type" onclick="setWaveform('triangle', this)">Triangle</button>
                <button class="wave-type" onclick="setWaveform('square', this)">Square</button>
                <button class="wave-type" onclick="setWaveform('sawtooth', this)">Sawtooth</button>
            </div>

            <div class="now-playing" id="now-playing">
                <span class="now-playing-text">Now Playing:</span>
                <span class="now-playing-freq" id="now-playing-freq"></span>
            </div>
        </div>

        <div class="info-grid">
            <div class="info-card schumann">
                <div class="freq">7.83 Hz — Schumann</div>
                <div class="desc">Earth's electromagnetic resonance. Too low to hear directly; rendered as binaural beat or amplitude modulation. Associated with meditation and grounding.</div>
            </div>
            <div class="info-card pyramid">
                <div class="freq">110 Hz — Pyramid</div>
                <div class="desc">Resonant frequency of the King's Chamber. Musical note A2. Used in ancient ritual spaces for altered states.</div>
            </div>
            <div class="info-card om">
                <div class="freq">136.1 Hz — OM</div>
                <div class="desc">The "cosmic tone" — Earth's orbital frequency scaled up. Approximately C#3. The sound of planetary motion.</div>
            </div>
            <div class="info-card natural-a">
                <div class="freq">432 Hz — Natural A</div>
                <div class="desc">Verdi's A. 432 × 4 = 1728 (j-invariant). Said to be mathematically consistent with the universe. Note A4.</div>
            </div>
            <div class="info-card miracle">
                <div class="freq">528 Hz — Miracle</div>
                <div class="desc">Solfeggio "MI" frequency. DNA repair tone. 528/432 = 1.222... (near 11/9). Approximately C5.</div>
            </div>
        </div>

        <div id="plotly-container"></div>
    </div>

    <script>
        // Web Audio API setup
        let audioCtx = null;
        let oscillators = [];
        let gainNode = null;
        let currentVolume = 0.3;
        let currentWaveform = 'sine';

        function initAudio() {
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
                gainNode = audioCtx.createGain();
                gainNode.connect(audioCtx.destination);
                gainNode.gain.value = currentVolume;
            }
            if (audioCtx.state === 'suspended') {
                audioCtx.resume();
            }
        }

        function playTone(freq, name, btn) {
            initAudio();

            // For Schumann (7.83 Hz), create amplitude modulation since it's subsonic
            if (freq < 20) {
                playSubsonicTone(freq, name);
                return;
            }

            const osc = audioCtx.createOscillator();
            osc.type = currentWaveform;
            osc.frequency.value = freq;

            const oscGain = audioCtx.createGain();
            oscGain.gain.value = 0;
            oscGain.gain.linearRampToValueAtTime(1, audioCtx.currentTime + 0.1);

            osc.connect(oscGain);
            oscGain.connect(gainNode);
            osc.start();

            oscillators.push({ osc, gain: oscGain, freq, name });

            updateNowPlaying();

            // Visual feedback
            if (btn) {
                btn.classList.add('active');
            }
        }

        function playSubsonicTone(freq, name) {
            // Create a carrier tone modulated at the subsonic frequency
            const carrier = audioCtx.createOscillator();
            carrier.type = 'sine';
            carrier.frequency.value = 200; // Carrier at 200 Hz

            const modulator = audioCtx.createOscillator();
            modulator.type = 'sine';
            modulator.frequency.value = freq;

            const modGain = audioCtx.createGain();
            modGain.gain.value = 0.5;

            const oscGain = audioCtx.createGain();
            oscGain.gain.value = 0;
            oscGain.gain.linearRampToValueAtTime(1, audioCtx.currentTime + 0.1);

            modulator.connect(modGain);
            modGain.connect(oscGain.gain);

            carrier.connect(oscGain);
            oscGain.connect(gainNode);

            carrier.start();
            modulator.start();

            oscillators.push({
                osc: carrier,
                modulator: modulator,
                gain: oscGain,
                freq,
                name,
                isSubsonic: true
            });

            updateNowPlaying();

            document.querySelector('.tone-btn.schumann').classList.add('active');
        }

        function playChord(freqs) {
            stopAll();
            setTimeout(() => {
                freqs.forEach((freq, i) => {
                    setTimeout(() => {
                        const name = getNameForFreq(freq);
                        playTone(freq, name, null);
                    }, i * 100);
                });
            }, 100);
        }

        function playBinauralBeat() {
            initAudio();
            stopAll();

            setTimeout(() => {
                // Left ear: 432 Hz
                const oscL = audioCtx.createOscillator();
                oscL.type = 'sine';
                oscL.frequency.value = 432 - 3.915; // Half the beat freq lower

                // Right ear: 432 + 7.83 Hz
                const oscR = audioCtx.createOscillator();
                oscR.type = 'sine';
                oscR.frequency.value = 432 + 3.915; // Half the beat freq higher

                // Create stereo panner
                const panL = audioCtx.createStereoPanner();
                panL.pan.value = -1;
                const panR = audioCtx.createStereoPanner();
                panR.pan.value = 1;

                const gainL = audioCtx.createGain();
                gainL.gain.value = 0;
                gainL.gain.linearRampToValueAtTime(1, audioCtx.currentTime + 0.1);

                const gainR = audioCtx.createGain();
                gainR.gain.value = 0;
                gainR.gain.linearRampToValueAtTime(1, audioCtx.currentTime + 0.1);

                oscL.connect(gainL);
                gainL.connect(panL);
                panL.connect(gainNode);

                oscR.connect(gainR);
                gainR.connect(panR);
                panR.connect(gainNode);

                oscL.start();
                oscR.start();

                oscillators.push(
                    { osc: oscL, gain: gainL, freq: 428.085, name: 'Binaural L' },
                    { osc: oscR, gain: gainR, freq: 435.915, name: 'Binaural R' }
                );

                document.getElementById('now-playing').classList.add('visible');
                document.getElementById('now-playing-freq').textContent =
                    'Binaural Beat: 7.83 Hz (use headphones!)';
                document.getElementById('now-playing-freq').style.color = '#8B5CF6';
            }, 100);
        }

        function getNameForFreq(freq) {
            const names = {
                7.83: 'Schumann',
                110: 'Pyramid',
                136.1: 'OM',
                432: '432 Hz',
                528: 'Miracle'
            };
            return names[freq] || freq + ' Hz';
        }

        function stopAll() {
            oscillators.forEach(item => {
                if (item.gain) {
                    item.gain.gain.linearRampToValueAtTime(0, audioCtx.currentTime + 0.1);
                }
                setTimeout(() => {
                    item.osc.stop();
                    if (item.modulator) item.modulator.stop();
                }, 150);
            });
            oscillators = [];

            // Remove active states
            document.querySelectorAll('.tone-btn').forEach(btn => btn.classList.remove('active'));

            document.getElementById('now-playing').classList.remove('visible');
        }

        function updateVolume(val) {
            currentVolume = val / 100;
            if (gainNode) {
                gainNode.gain.value = currentVolume;
            }
            document.getElementById('volume-display').textContent = val + '%';
        }

        function setWaveform(type, btn) {
            currentWaveform = type;
            document.querySelectorAll('.wave-type').forEach(b => b.classList.remove('active'));
            btn.classList.add('active');

            // Update any playing oscillators
            oscillators.forEach(item => {
                if (!item.isSubsonic) {
                    item.osc.type = type;
                }
            });
        }

        function updateNowPlaying() {
            const playing = oscillators.map(o => o.name || o.freq + ' Hz').join(' + ');
            const colors = {
                'Schumann': '#8B5CF6',
                'Pyramid': '#EC4899',
                'OM': '#F97316',
                '432 Hz': '#EAB308',
                'Miracle': '#22C55E'
            };

            document.getElementById('now-playing').classList.add('visible');
            const freqEl = document.getElementById('now-playing-freq');
            freqEl.textContent = playing;

            // Set color based on first playing tone
            const firstName = oscillators[0]?.name;
            freqEl.style.color = colors[firstName] || '#58a6ff';
        }
    </script>

    PLOTLY_CHART_PLACEHOLDER

</body>
</html>
"""
    return html

def create_3d_visualization():
    """Create the 3D Plotly figure."""

    t = np.linspace(0, 2*np.pi, 2000)

    fig = go.Figure()

    # Add each harmonic as a 3D trace
    for name, props in HARMONICS.items():
        freq = props['freq']
        color = props['color']
        z_offset = props['z_offset']

        amp = 0.3 * (7.83 / freq) ** 0.3
        x, y, z = modulate_curve_3d(t, freq, z_base=z_offset, mod_amplitude=amp)

        display_name = name.replace('_', ' ')
        fig.add_trace(go.Scatter3d(
            x=x, y=y, z=z,
            mode='lines',
            name=f'{display_name} ({freq} Hz)',
            line=dict(color=color, width=5),
            hovertemplate=f'{display_name}<br>%{{x:.2f}}, %{{y:.2f}}, %{{z:.2f}}<extra></extra>'
        ))

    # Base curve
    x_base, y_base = lemniscate_alpha(t)
    fig.add_trace(go.Scatter3d(
        x=x_base, y=y_base, z=np.zeros_like(t) - 0.5,
        mode='lines',
        name='Base Lemniscate-Alpha',
        line=dict(color='white', width=2),
        opacity=0.3
    ))

    # Origin marker
    fig.add_trace(go.Scatter3d(
        x=[0], y=[0], z=[2],
        mode='markers+text',
        name='Origin',
        marker=dict(size=10, color='#fbbf24', symbol='diamond'),
        text=['Origin'],
        textposition='top center',
        textfont=dict(color='#fbbf24', size=12)
    ))

    # Layout
    fig.update_layout(
        scene=dict(
            xaxis=dict(
                title='X',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white'
            ),
            yaxis=dict(
                title='Y',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white'
            ),
            zaxis=dict(
                title='Harmonic Layer',
                backgroundcolor='#0d1117',
                gridcolor='#30363d',
                showbackground=True,
                zerolinecolor='#30363d',
                color='white',
                ticktext=['Schumann', 'Pyramid', 'OM', '432 Hz', 'Miracle'],
                tickvals=[0, 1, 2, 3, 4]
            ),
            bgcolor='#0d1117',
            camera=dict(eye=dict(x=1.5, y=1.5, z=1.2))
        ),
        paper_bgcolor='#0d1117',
        plot_bgcolor='#0d1117',
        legend=dict(
            x=0.02, y=0.98,
            bgcolor='rgba(22, 27, 34, 0.8)',
            bordercolor='#30363d',
            font=dict(color='white')
        ),
        margin=dict(l=0, r=0, t=10, b=0),
        height=600
    )

    return fig

def main():
    print("Creating interactive 3D visualization with audio...")

    # Create the plotly figure
    fig = create_3d_visualization()

    # Get the plotly HTML (just the div and script)
    plotly_html = fig.to_html(full_html=False, include_plotlyjs='cdn')

    # Create the full HTML with audio controls
    html_template = create_audio_html()

    # Insert the plotly chart
    full_html = html_template.replace('PLOTLY_CHART_PLACEHOLDER', plotly_html)

    # Save
    output_file = 'lemniscate_alpha_3d_audio.html'
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(full_html)

    print(f"Saved: {output_file}")
    print()
    print("Features:")
    print("  - Interactive 3D visualization (rotate, zoom, pan)")
    print("  - Click tone buttons to hear each frequency")
    print("  - Play chords (multiple frequencies together)")
    print("  - Binaural beat mode (use headphones!)")
    print("  - Adjustable volume and waveform type")
    print()
    print("Note: 7.83 Hz (Schumann) is subsonic, so it's rendered as")
    print("      amplitude modulation on a 200 Hz carrier tone.")

    return output_file

if __name__ == "__main__":
    output = main()

    # Open in browser
    import webbrowser
    import os
    webbrowser.open('file://' + os.path.realpath(output))
