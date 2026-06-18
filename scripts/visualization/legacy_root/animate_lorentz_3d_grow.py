import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import scipy.special as sp
from mpl_toolkits.mplot3d import Axes3D
import os

print("Setting up refined 3D Lorentz animation...")

def ease_in_out_cubic(t):
    return 4 * t**3 if t < 0.5 else 1 - (-2 * t + 2)**3 / 2

g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

N = 1000
frames = 400

target_r = np.zeros(N)
target_z = np.zeros(N)
target_theta_2d = np.zeros(N)

golden_angle = np.pi * (3 - np.sqrt(5))
target_theta_3d = np.arange(N) * golden_angle

# Group 1: Cyan (One Sheet) -> spacelike
target_z[0:500] = np.linspace(-12, 12, 500)
target_r[0:500] = np.sqrt(target_z[0:500]**2 + c)
target_theta_2d[0:250] = 0.0
target_theta_2d[250:500] = np.pi

# Group 2: Magenta (Two Sheets) -> timelike
r_mag = np.linspace(0, 12, 250)
target_r[500:750] = r_mag
target_z[500:750] = np.sqrt(r_mag**2 + c)
target_theta_2d[500:625] = 0.0
target_theta_2d[625:750] = np.pi

target_r[750:1000] = r_mag
target_z[750:1000] = -np.sqrt(r_mag**2 + c)
target_theta_2d[750:875] = 0.0
target_theta_2d[875:1000] = np.pi

c_arr = ['#00FFFF'] * 500 + ['#FF00FF'] * 500

fig = plt.figure(figsize=(10, 10))
fig.patch.set_facecolor('black')
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('black')
ax.grid(False)
ax.set_axis_off()

ax.set_xlim([-12, 12])
ax.set_ylim([-12, 12])
ax.set_zlim([-12, 12])

# Lightcone
u = np.linspace(-12, 12, 100)
ax.plot(u, np.zeros(100), u, color='gray', alpha=0.4, zorder=1)
ax.plot(u, np.zeros(100), -u, color='gray', alpha=0.4, zorder=1)

scatter = ax.scatter([0]*N, [0]*N, [0]*N, s=12, c=c_arr, edgecolors='none', alpha=0.9, zorder=5)

def update(frame):
    # Camera Logic (Smooth sweeping orbit)
    if frame < 150:
        ax.view_init(elev=0, azim=-90)
    else:
        t_cam = (frame - 150) / 249.0
        p_elev = ease_in_out_cubic(min(t_cam * 2.0, 1.0))
        elev = 30 * p_elev
        azim = -90 + 180 * ease_in_out_cubic(t_cam)
        ax.view_init(elev=elev, azim=azim)

    if frame < 20:
        x, y, z = np.zeros(N), np.zeros(N), np.zeros(N)
    elif frame < 50:
        # Origin dot splits into 4 vertices
        t = (frame - 20) / 30.0
        t_smooth = ease_in_out_cubic(t)

        r_cyan = np.sqrt(c) * t_smooth
        z_cyan = np.zeros(500)

        r_mag_start = np.zeros(500)
        z_mag_start = np.sign(target_z[500:1000]) * np.sqrt(c) * t_smooth

        curr_r = np.concatenate([np.full(500, r_cyan), r_mag_start])
        curr_z = np.concatenate([z_cyan, z_mag_start])

        x = curr_r * np.cos(target_theta_2d)
        y = np.zeros(N)
        z = curr_z
    elif frame < 150:
        # Vertices grow outward drawing the 2D hyperbolas
        t = (frame - 50) / 100.0
        t_smooth = ease_in_out_cubic(t)

        z_cyan = target_z[0:500] * t_smooth
        r_cyan = np.sqrt(z_cyan**2 + c)

        r_mag_grow = target_r[500:1000] * t_smooth
        z_mag_grow = np.sign(target_z[500:1000]) * np.sqrt(r_mag_grow**2 + c)

        curr_r = np.concatenate([r_cyan, r_mag_grow])
        curr_z = np.concatenate([z_cyan, z_mag_grow])

        x = curr_r * np.cos(target_theta_2d)
        y = np.zeros(N)
        z = curr_z
    elif frame < 180:
        # Hold 2D shape before 3D burst
        x = target_r * np.cos(target_theta_2d)
        y = np.zeros(N)
        z = target_z
    elif frame < 310:
        # Burst into 3D continuous ultrahypersymmetry
        t = (frame - 180) / 130.0
        t_smooth = ease_in_out_cubic(t)

        current_theta = target_theta_2d * (1 - t_smooth) + target_theta_3d * t_smooth
        x = target_r * np.cos(current_theta)
        y = target_r * np.sin(current_theta)
        z = target_z
    else:
        # Hold 3D shape, camera completes rotation
        x = target_r * np.cos(target_theta_3d)
        y = target_r * np.sin(target_theta_3d)
        z = target_z

    scatter._offsets3d = (x, y, z)
    return scatter,

print("Rendering 3D animation (this will take ~1-2 minutes)...")
ani = FuncAnimation(fig, update, frames=frames, blit=False)

artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_file = os.path.join(artifact_dir, "lorentz_3d_evolution_grow.gif")

ani.save(output_file, writer=PillowWriter(fps=25))
print(f"3D Animation saved to {output_file}")
