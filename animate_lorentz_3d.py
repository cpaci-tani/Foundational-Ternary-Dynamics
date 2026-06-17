import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import scipy.special as sp
from mpl_toolkits.mplot3d import Axes3D
import os

print("Setting up 3D Lorentz animation...")

g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

N = 1000
frames = 400

target_r = np.zeros(N)
target_z = np.zeros(N)
target_theta_2d = np.zeros(N)

golden_angle = np.pi * (3 - np.sqrt(5))
target_theta_3d = np.arange(N) * golden_angle

# Group 1: Cyan (One Sheet) -> spacelike outside lightcone
target_z[0:500] = np.linspace(-12, 12, 500)
target_r[0:500] = np.sqrt(target_z[0:500]**2 + c)
target_theta_2d[0:250] = 0.0
target_theta_2d[250:500] = np.pi

# Group 2: Magenta (Two Sheets) -> timelike inside lightcone
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

# Interleave the dots so the sine wave looks mathematically mixed
indices = np.zeros(N, dtype=int)
indices[0::2] = np.arange(0, 500)
indices[1::2] = np.arange(500, 1000)

target_r = target_r[indices]
target_z = target_z[indices]
target_theta_2d = target_theta_2d[indices]
target_theta_3d = target_theta_3d[indices]
c_arr = [c_arr[i] for i in indices]

fig = plt.figure(figsize=(10, 10))
fig.patch.set_facecolor('black')
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('black')
ax.grid(False)
ax.set_axis_off()

ax.set_xlim([-12, 12])
ax.set_ylim([-12, 12])
ax.set_zlim([-12, 12])

# Plot 2D Lightcone asymptotes (x=z, x=-z)
u = np.linspace(-12, 12, 100)
ax.plot(u, np.zeros(100), u, color='gray', alpha=0.4, zorder=1)
ax.plot(u, np.zeros(100), -u, color='gray', alpha=0.4, zorder=1)

scatter = ax.scatter([0]*N, [0]*N, [0]*N, s=12, c=c_arr, edgecolors='none', alpha=0.9, zorder=5)

base_x = np.linspace(-10, 10, N)

def ease_in_out_cubic(t):
    return 4 * t**3 if t < 0.5 else 1 - (-2 * t + 2)**3 / 2

def update(frame):
    # Camera Logic
    if frame < 220:
        ax.view_init(elev=0, azim=-90)
    else:
        t_cam = (frame - 220) / 179.0
        # Very smooth ease-out for camera
        p_smooth = 1 - (1 - t_cam)**3
        ax.view_init(elev=0 + 25 * p_smooth, azim=-90 + 60 * p_smooth)

    # Position Logic
    if frame < 20:
        x, y, z = np.zeros(N), np.zeros(N), np.zeros(N)
    elif frame < 50:
        t = (frame - 20) / 30.0
        t_smooth = ease_in_out_cubic(t)
        x, y, z = base_x * t_smooth, np.zeros(N), np.zeros(N)
    elif frame < 130:
        A = 3.0 if frame >= 70 else 3.0 * ease_in_out_cubic((frame - 50) / 20.0)
        phi = (frame - 50) * 0.15
        x, y, z = base_x, np.zeros(N), A * np.sin(base_x * np.pi / 5 - phi)
    elif frame < 200:
        t = (frame - 130) / 70.0
        t_smooth = ease_in_out_cubic(t)
        start_x = base_x
        phi_freeze = (129 - 50) * 0.15
        start_z = 3.0 * np.sin(base_x * np.pi / 5 - phi_freeze)
        
        target_x_2d = target_r * np.cos(target_theta_2d)
        
        x = start_x * (1 - t_smooth) + target_x_2d * t_smooth
        y = np.zeros(N)
        z = start_z * (1 - t_smooth) + target_z * t_smooth
    elif frame < 230:
        x, y, z = target_r * np.cos(target_theta_2d), np.zeros(N), target_z
    elif frame < 360:
        # Longer, smoother 3D burst (130 frames = 5+ seconds)
        t = (frame - 230) / 130.0
        t_smooth = ease_in_out_cubic(t)
        
        current_theta = target_theta_2d * (1 - t_smooth) + target_theta_3d * t_smooth
        x = target_r * np.cos(current_theta)
        y = target_r * np.sin(current_theta)
        z = target_z
    else:
        x = target_r * np.cos(target_theta_3d)
        y = target_r * np.sin(target_theta_3d)
        z = target_z

    scatter._offsets3d = (x, y, z)
    return scatter,

print("Rendering 3D animation (this will take ~1-2 minutes)...")
ani = FuncAnimation(fig, update, frames=frames, blit=False)

artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_file = os.path.join(artifact_dir, "lorentz_3d_evolution.gif")

ani.save(output_file, writer=PillowWriter(fps=25))
print(f"3D Animation saved to {output_file}")
