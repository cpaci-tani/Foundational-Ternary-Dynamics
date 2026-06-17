import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, FFMpegWriter
import scipy.special as sp
from mpl_toolkits.mplot3d import Axes3D
import os

print("Setting up the mathematically rigorous Epic Video...")

def ease_in_out_cubic(t):
    return 4 * t**3 if t < 0.5 else 1 - (-2 * t + 2)**3 / 2

g_star = sp.gamma(0.25) / sp.gamma(0.75)
c = g_star

def is_prime(n):
    if n <= 1: return False
    if n <= 3: return True
    if n % 2 == 0 or n % 3 == 0: return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0: return False
        i += 6
    return True

g_primes = []
for x in range(-40, 41):
    for z in range(-40, 41):
        if x == 0 and z == 0: continue
        if x != 0 and z != 0:
            if is_prime(x**2 + z**2):
                g_primes.append((x, z))
        else:
            p = abs(x) if z == 0 else abs(z)
            if p % 4 == 3 and is_prime(p):
                g_primes.append((x, z))

g_primes = [(x,z) for x,z in g_primes if np.sqrt(x**2+z**2) <= 40]
gp_x = np.array([p[0] for p in g_primes], dtype=float)
gp_z = np.array([p[1] for p in g_primes], dtype=float)
num_gp = len(g_primes)

N = 2000 # points for continuous hyperbolas
frames = 900

target_r = np.zeros(N)
target_z = np.zeros(N)
target_theta_2d = np.zeros(N)

golden_angle = np.pi * (3 - np.sqrt(5))
target_theta_3d = np.arange(N) * golden_angle

# Cyan (One Sheet)
target_z[0:1000] = np.linspace(-40, 40, 1000)
target_r[0:1000] = np.sqrt(target_z[0:1000]**2 + c)
target_theta_2d[0:500] = 0.0
target_theta_2d[500:1000] = np.pi

# Magenta (Two Sheets)
r_mag = np.linspace(0, 40, 500)
target_r[1000:1500] = r_mag
target_z[1000:1500] = np.sqrt(r_mag**2 + c)
target_theta_2d[1000:1250] = 0.0
target_theta_2d[1250:1500] = np.pi

target_r[1500:2000] = r_mag
target_z[1500:2000] = -np.sqrt(r_mag**2 + c)
target_theta_2d[1500:1750] = 0.0
target_theta_2d[1750:2000] = np.pi

c_arr = ['#00FFFF'] * 1000 + ['#FF00FF'] * 1000
indices = np.zeros(N, dtype=int)
indices[0::2] = np.arange(0, 1000)
indices[1::2] = np.arange(1000, 2000)
target_r = target_r[indices]
target_z = target_z[indices]
target_theta_2d = target_theta_2d[indices]
target_theta_3d = target_theta_3d[indices]
c_arr = [c_arr[i] for i in indices]

# Target 2D coordinates for the hyperbolas
hyp_x_2d = target_r * np.cos(target_theta_2d)
hyp_y_2d = np.zeros(N)
hyp_z_2d = target_z

fig = plt.figure(figsize=(12, 8))
fig.patch.set_facecolor('black')
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('black')
ax.grid(False)
ax.set_axis_off()

ax.set_xlim([-40, 40])
ax.set_ylim([-40, 40])
ax.set_zlim([-40, 40])

# Lightcone
u = np.linspace(-40, 40, 150)
lc1, = ax.plot(u, np.zeros(150), u, color='gray', alpha=0.0, zorder=1)
lc2, = ax.plot(u, np.zeros(150), -u, color='gray', alpha=0.0, zorder=1)

# Scatters
scatter_gp = ax.scatter([0]*num_gp, [0]*num_gp, [0]*num_gp, s=6, c='white', edgecolors='none', alpha=1.0, zorder=4)
scatter_hyp = ax.scatter(hyp_x_2d, hyp_y_2d, hyp_z_2d, s=8, c=c_arr, edgecolors='none', alpha=0.0, zorder=5)

# Text overlay
text_overlay = fig.text(0.5, 0.05, "", color='white', fontsize=18, ha='center', va='bottom', family='sans-serif', fontweight='light')
title_overlay = fig.text(0.5, 0.90, "", color='white', fontsize=24, ha='center', va='top', family='sans-serif', fontweight='bold')

def get_text(frame):
    if frame < 60:
        return "ACT I: THE ORIGIN (0D)", "A dimensionless point of pure mathematical potential."
    elif frame < 200:
        return "THE INTEGER LATTICE", "Breaking into discrete prime density—The Gaussian Primes."
    elif frame < 300:
        return "THE CONTINUOUS BOUNDS", "The primes naturally respect the bounds of the D4 G* Hyperbolas."
    elif frame < 400:
        return "THE ABSTRACTION", "From discrete arithmetic to continuous geometry."
    elif frame < 550:
        return "ACT II: LORENTZ INVARIANCE", "Sweeping the 2D plane into a continuous 3D spacetime volume."
    elif frame < 650:
        return "THE LIGHTCONE", "The absolute causal boundary of the universe."
    else:
        return "ACT III: ULTRAHYPERSYMMETRY", "The Spacelike and Timelike manifolds of the cosmos."

def update(frame):
    title, subtitle = get_text(frame)
    title_overlay.set_text(title)
    text_overlay.set_text(subtitle)
    
    # Camera Logic
    if frame < 400:
        ax.view_init(elev=0, azim=-90)
    else:
        t_cam = (frame - 400) / 499.0
        p_elev = ease_in_out_cubic(min(t_cam * 1.5, 1.0))
        elev = 30 * p_elev
        azim = -90 + 360 * ease_in_out_cubic(t_cam)
        ax.view_init(elev=elev, azim=azim)

    # Lightcone Logic
    if frame > 550:
        alpha = min((frame - 550) / 100.0 * 0.4, 0.4)
        lc1.set_alpha(alpha)
        lc2.set_alpha(alpha)

    # Gaussian Primes Logic
    if frame < 60:
        scatter_gp._offsets3d = (np.zeros(num_gp), np.zeros(num_gp), np.zeros(num_gp))
        scatter_gp.set_alpha(1.0)
    elif frame < 120:
        t = (frame - 60) / 60.0
        t_smooth = 1 - (1 - t)**3 # aggressive pop out
        x = gp_x * t_smooth
        y = np.zeros(num_gp)
        z = gp_z * t_smooth
        scatter_gp._offsets3d = (x, y, z)
        scatter_gp.set_alpha(1.0)
    elif frame < 300:
        scatter_gp._offsets3d = (gp_x, np.zeros(num_gp), gp_z)
        scatter_gp.set_alpha(1.0)
    elif frame < 400:
        # Fade out primes
        alpha = max(1.0 - (frame - 300) / 100.0, 0.0)
        scatter_gp.set_alpha(alpha)
    else:
        scatter_gp.set_alpha(0.0)

    # Continuous Hyperbolas Logic
    if frame < 200:
        scatter_hyp.set_alpha(0.0)
    elif frame < 300:
        # Fade in hyperbolas
        alpha = min((frame - 200) / 100.0 * 0.9, 0.9)
        scatter_hyp.set_alpha(alpha)
        scatter_hyp._offsets3d = (hyp_x_2d, hyp_y_2d, hyp_z_2d)
    elif frame < 400:
        # Hold 2D
        scatter_hyp.set_alpha(0.9)
        scatter_hyp._offsets3d = (hyp_x_2d, hyp_y_2d, hyp_z_2d)
    elif frame < 550:
        # Burst to 3D
        t = (frame - 400) / 150.0
        t_smooth = ease_in_out_cubic(t)
        current_theta = target_theta_2d * (1 - t_smooth) + target_theta_3d * t_smooth
        x = target_r * np.cos(current_theta)
        y = target_r * np.sin(current_theta)
        z = target_z
        scatter_hyp._offsets3d = (x, y, z)
        scatter_hyp.set_alpha(0.9)
    else:
        x = target_r * np.cos(target_theta_3d)
        y = target_r * np.sin(target_theta_3d)
        z = target_z
        scatter_hyp._offsets3d = (x, y, z)
        scatter_hyp.set_alpha(0.9)

    return scatter_gp, scatter_hyp, lc1, lc2, title_overlay, text_overlay

print("Rendering Rigorous Epic Video (this will take ~3-5 minutes)...")
ani = FuncAnimation(fig, update, frames=frames, blit=False)

artifact_dir = r"C:\Users\cpaci\.gemini\antigravity\brain\520f9887-149c-4d9b-bba7-d534bf7b3d1d"
os.makedirs(artifact_dir, exist_ok=True)
output_file = os.path.join(artifact_dir, "the_arrow_and_the_ratio.mp4")

writer = FFMpegWriter(fps=30, metadata=dict(artist='Antigravity'), bitrate=5000)
ani.save(output_file, writer=writer)
print(f"Video saved to {output_file}")
