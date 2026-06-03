import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys

def main():
    print("Launching Vacuum Energy Looping Animation...")
    fig = plt.figure(figsize=(10, 10), facecolor='black')
    ax = fig.add_subplot(111, projection='3d')
    ax.set_facecolor('black')
    ax.set_axis_off()
    ax.set_title("Discrete Brillouin Zone: Vacuum Energy Modes", color='white', fontsize=18)

    # Generate a cloud of points representing the momentum modes up to the discrete cutoff
    N = 1500
    phi = np.random.uniform(0, 2*np.pi, N)
    costheta = np.random.uniform(-1, 1, N)
    u = np.random.uniform(0, 1, N)
    theta = np.arccos(costheta)
    r = np.pi * np.cbrt(u) # max radius is pi (Brillouin zone edge)

    x = r * np.sin(theta) * np.cos(phi)
    y = r * np.sin(theta) * np.sin(phi)
    z = r * np.cos(theta)

    # Calculate discrete lattice energy omega
    sx, sy, sz = np.sin(x/2), np.sin(y/2), np.sin(z/2)
    omega = 2.0 * np.sqrt(sx**2 + sy**2 + sz**2)

    # Plot the modes colored by their energy density
    scatter = ax.scatter(x, y, z, c=omega, cmap='plasma', s=20, alpha=0.8)

    def update(frame):
        # Rotate the camera continuously
        ax.view_init(elev=20 + 10 * np.sin(np.radians(frame)), azim=frame % 360)
        
        # Create a quantum fluctuation pulsing effect
        pulse = 20 + 10 * np.sin(frame / 10.0 + omega * 2)
        scatter._sizes = pulse
        return scatter,

    ani = animation.FuncAnimation(fig, update, frames=360, interval=40, blit=False)
    plt.show()

if __name__ == "__main__":
    main()
