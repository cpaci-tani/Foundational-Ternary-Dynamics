"""
4K Looping Animation: TRD Mandelbrot-Consciousness Boundary

Creates a seamless loop zooming into the fractal boundary where
meta-sLoop (consciousness) configurations concentrate.

Resolution: 3840x2160 (4K UHD)
Output: MP4 video with seamless loop

Author: 2026-01-21
"""

import torch
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
import os
from pathlib import Path
import time
from tqdm import tqdm

# Configuration
RESOLUTION = (3840, 2160)  # 4K UHD
FPS = 60
DURATION = 10  # seconds for full loop
N_FRAMES = FPS * DURATION
MAX_ITER = 1000
OUTPUT_DIR = Path("investigation_results/animation_frames")
OUTPUT_VIDEO = "investigation_results/trd_consciousness_4k.mp4"

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(f"Device: {device}")
if torch.cuda.is_available():
    print(f"GPU: {torch.cuda.get_device_name(0)}")
    print(f"VRAM: {torch.cuda.get_device_properties(0).total_memory / 1e9:.1f} GB")


def create_colormap():
    """Deep space fractal colormap with consciousness-themed colors."""
    colors = [
        (0.02, 0.02, 0.08),   # Deep void
        (0.05, 0.05, 0.20),   # Dark blue
        (0.10, 0.15, 0.40),   # Navy
        (0.15, 0.30, 0.55),   # Ocean blue
        (0.20, 0.50, 0.60),   # Teal
        (0.30, 0.65, 0.50),   # Sea green
        (0.50, 0.75, 0.35),   # Lime
        (0.75, 0.80, 0.20),   # Yellow-green
        (0.95, 0.75, 0.15),   # Gold
        (1.00, 0.55, 0.10),   # Orange
        (0.95, 0.35, 0.15),   # Red-orange
        (0.80, 0.20, 0.30),   # Crimson
        (0.60, 0.10, 0.45),   # Magenta
        (0.40, 0.08, 0.50),   # Purple
        (0.25, 0.05, 0.40),   # Deep purple
        (0.10, 0.02, 0.20),   # Near black
    ]
    return LinearSegmentedColormap.from_list('consciousness', colors, N=1024)


def compute_trd_mandelbrot_gpu(width, height, center_x, center_y, zoom,
                                max_iter=500, K_B=0.8, color_offset=0.0):
    """
    GPU-accelerated TRD-modulated Mandelbrot computation.

    The K_B threshold creates consciousness-relevant boundary structure.
    """
    # Compute aspect-corrected range
    aspect = width / height
    half_width = 2.0 / zoom
    half_height = half_width / aspect

    x_min, x_max = center_x - half_width, center_x + half_width
    y_min, y_max = center_y - half_height, center_y + half_height

    x = torch.linspace(x_min, x_max, width, device=device)
    y = torch.linspace(y_min, y_max, height, device=device)
    X, Y = torch.meshgrid(x, y, indexing='xy')

    c_real = X
    c_imag = Y
    z_real = torch.zeros_like(c_real)
    z_imag = torch.zeros_like(c_imag)

    escape_time = torch.full((height, width), max_iter, device=device, dtype=torch.float32)
    escaped = torch.zeros(height, width, dtype=torch.bool, device=device)

    for n in range(max_iter):
        r = torch.sqrt(z_real**2 + z_imag**2)

        # TRD threshold modulation - creates consciousness boundary
        modulation = torch.tanh((r - K_B) * 3)

        # z^2 + c with modulation
        z_real_new = z_real**2 - z_imag**2
        z_imag_new = 2 * z_real * z_imag

        z_real = z_real_new + c_real * (1 + 0.2 * modulation)
        z_imag = z_imag_new + c_imag * (1 + 0.2 * modulation)

        r2 = z_real**2 + z_imag**2
        newly_escaped = (r2 > 4) & ~escaped

        # Smooth coloring with offset for animation
        log_zn = torch.log(r2 + 1e-10) / 2
        nu = torch.log(log_zn / np.log(2) + 1e-10) / np.log(2)
        smooth_n = n + 1 - nu + color_offset
        escape_time[newly_escaped] = smooth_n[newly_escaped]
        escaped = escaped | (r2 > 4)

    return escape_time.cpu().numpy()


def ease_in_out(t):
    """Smooth easing function for zoom transitions."""
    return t * t * (3 - 2 * t)


def generate_zoom_path(n_frames):
    """
    Generate a looping zoom path that visits interesting boundary regions.

    The path zooms into a boundary region and back out for seamless loop.
    """
    # Target: zoom into the boundary near the main cardioid
    # This is where meta-sLoop configurations concentrate

    # Interesting point on the boundary (cusp of main cardioid)
    target_x = -0.75
    target_y = 0.0

    # Alternative: Seahorse valley (very complex boundary)
    # target_x = -0.745
    # target_y = 0.113

    # Zoom range: start at overview, zoom to 100x
    zoom_min = 1.0
    zoom_max = 50.0

    centers_x = []
    centers_y = []
    zooms = []
    color_offsets = []

    for i in range(n_frames):
        # Progress through the loop [0, 1]
        t = i / n_frames

        # Smooth zoom in and out (sin wave for seamless loop)
        zoom_t = (1 - np.cos(2 * np.pi * t)) / 2  # 0 -> 1 -> 0
        zoom = zoom_min * (1 - zoom_t) + zoom_max * zoom_t

        # Interpolate center toward target as we zoom
        center_x = -0.5 * (1 - zoom_t) + target_x * zoom_t
        center_y = 0.0 * (1 - zoom_t) + target_y * zoom_t

        # Color cycling for visual interest
        color_offset = t * 50  # Shift color palette

        centers_x.append(center_x)
        centers_y.append(center_y)
        zooms.append(zoom)
        color_offsets.append(color_offset)

    return centers_x, centers_y, zooms, color_offsets


def render_frame(frame_idx, center_x, center_y, zoom, color_offset, cmap):
    """Render a single 4K frame."""
    width, height = RESOLUTION

    # Compute fractal
    escape_time = compute_trd_mandelbrot_gpu(
        width, height, center_x, center_y, zoom,
        max_iter=MAX_ITER, K_B=0.8, color_offset=color_offset
    )

    # Normalize for colormap
    escape_normalized = escape_time / MAX_ITER

    # Apply colormap
    colored = cmap(escape_normalized)

    # Convert to uint8
    frame = (colored[:, :, :3] * 255).astype(np.uint8)

    return frame


def main():
    print("=" * 70)
    print("TRD CONSCIOUSNESS BOUNDARY - 4K ANIMATION")
    print(f"Resolution: {RESOLUTION[0]}x{RESOLUTION[1]}")
    print(f"Duration: {DURATION}s at {FPS}fps = {N_FRAMES} frames")
    print("=" * 70)

    # Create output directory
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    # Generate zoom path
    print("\nGenerating zoom path...")
    centers_x, centers_y, zooms, color_offsets = generate_zoom_path(N_FRAMES)

    # Create colormap
    cmap = create_colormap()

    # Render frames
    print(f"\nRendering {N_FRAMES} frames...")
    start_time = time.time()

    for i in tqdm(range(N_FRAMES), desc="Rendering"):
        frame = render_frame(
            i, centers_x[i], centers_y[i], zooms[i], color_offsets[i], cmap
        )

        # Save frame
        frame_path = OUTPUT_DIR / f"frame_{i:05d}.png"
        plt.imsave(str(frame_path), frame)

    render_time = time.time() - start_time
    print(f"\nRendering complete in {render_time:.1f}s ({render_time/N_FRAMES:.2f}s/frame)")

    # Compile video with ffmpeg
    print("\nCompiling video with ffmpeg...")
    import subprocess

    ffmpeg_cmd = [
        'ffmpeg', '-y',
        '-framerate', str(FPS),
        '-i', str(OUTPUT_DIR / 'frame_%05d.png'),
        '-c:v', 'libx264',
        '-preset', 'slow',
        '-crf', '18',
        '-pix_fmt', 'yuv420p',
        '-movflags', '+faststart',
        OUTPUT_VIDEO
    ]

    try:
        subprocess.run(ffmpeg_cmd, check=True, capture_output=True)
        print(f"\nVideo saved to: {OUTPUT_VIDEO}")

        # Get file size
        video_size = os.path.getsize(OUTPUT_VIDEO) / (1024 * 1024)
        print(f"File size: {video_size:.1f} MB")

    except FileNotFoundError:
        print("\nffmpeg not found. Frames saved to:", OUTPUT_DIR)
        print("To compile manually, run:")
        print(f"  ffmpeg -framerate {FPS} -i {OUTPUT_DIR}/frame_%05d.png -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p {OUTPUT_VIDEO}")
    except subprocess.CalledProcessError as e:
        print(f"\nffmpeg error: {e}")
        print("Frames saved to:", OUTPUT_DIR)

    # Cleanup frames (optional)
    # for f in OUTPUT_DIR.glob("*.png"):
    #     f.unlink()

    print("\n" + "=" * 70)
    print("ANIMATION COMPLETE")
    print("=" * 70)


if __name__ == "__main__":
    main()
