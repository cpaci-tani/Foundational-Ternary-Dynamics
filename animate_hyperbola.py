import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import scipy.special as sp
import os

print("Setting up animation...")

g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

N = 600
frames = 250

hx = np.zeros(N)
hy = np.zeros(N)

# We have 600 points.
# 0-299: x < 0
hx[0:300:2] = np.linspace(-10, -0.2, 150) # top left -> magenta
hy[0:300:2] = -c / hx[0:300:2]

hx[1:300:2] = np.linspace(-10, -0.2, 150) # bottom left -> cyan
hy[1:300:2] = c / hx[1:300:2]

# 300-599: x > 0
hx[300:600:2] = np.linspace(0.2, 10, 150) # top right -> cyan
hy[300:600:2] = c / hx[300:600:2]

hx[301:600:2] = np.linspace(0.2, 10, 150) # bottom right -> magenta
hy[301:600:2] = -c / hx[301:600:2]

c_arr = []
for i in range(N):
    if i < N//2:
        if i % 2 == 0: c_arr.append('#FF00FF') # magenta
        else: c_arr.append('#00FFFF') # cyan
    else:
        if i % 2 == 0: c_arr.append('#00FFFF') # cyan
        else: c_arr.append('#FF00FF') # magenta

fig, ax = plt.subplots(figsize=(10, 10))
fig.patch.set_facecolor('black')
ax.set_facecolor('black')

# Small solid dots
scatter = ax.scatter(np.zeros(N), np.zeros(N), s=8, c=c_arr, edgecolors='none', zorder=5)

ax.set_xlim(-12, 12)
ax.set_ylim(-12, 12)
ax.axis('off')

# Faint axes and diagonal asymptotes
ax.axhline(0, color='gray', linestyle='-', alpha=0.3, zorder=1)
ax.axvline(0, color='gray', linestyle='-', alpha=0.3, zorder=1)
ax.plot([-12, 12], [-12, 12], color='gray', linestyle='-', alpha=0.15, zorder=1)
ax.plot([-12, 12], [12, -12], color='gray', linestyle='-', alpha=0.15, zorder=1)

base_x = np.linspace(-10, 10, N)

def update(frame):
    if frame < 20:
        # Dot
        x = np.zeros(N)
        y = np.zeros(N)
    elif frame < 50:
        # Line expanding
        t = (frame - 20) / 30.0
        t_smooth = t * t * (3 - 2 * t)
        x = base_x * t_smooth
        y = np.zeros(N)
    elif frame < 140:
        # Sine wave
        A = 3.0
        if frame < 70:
            A = 3.0 * ((frame - 50) / 20.0)
        phi = (frame - 50) * 0.15
        x = base_x
        y = A * np.sin(base_x * np.pi / 5 - phi)
    elif frame < 210:
        # Scattering to hyperbola
        t = (frame - 140) / 70.0
        t_smooth = t * t * (3 - 2 * t)
        start_x = base_x
        phi_freeze = (139 - 50) * 0.15
        start_y = 3.0 * np.sin(base_x * np.pi / 5 - phi_freeze)
        
        x = start_x * (1 - t_smooth) + hx * t_smooth
        y = start_y * (1 - t_smooth) + hy * t_smooth
    else:
        # Hold hyperbola
        x = hx
        y = hy

    scatter.set_offsets(np.c_[x, y])
    return scatter,

print("Rendering animation (this may take a minute)...")
ani = FuncAnimation(fig, update, frames=frames, blit=True)

artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_file = os.path.join(artifact_dir, "hyperbola_evolution.gif")

ani.save(output_file, writer=PillowWriter(fps=25))
print(f"Animation saved to {output_file}")
