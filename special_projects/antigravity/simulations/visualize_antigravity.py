"""
Antigravity Visualization: The Flux Shield (Motion)
===================================================

Generates an animation showing the "Vacuum Flux" interacting with the Resonant Hull.

Legend:
- Background Color: Gravitational Potential (Gradient = Force).
- Blue Dots: Shield Nodes.
- Effect: At resonance, the gradient curves AROUND the shield, leaving the center constant.
- Motion: The craft translates through the field, surfing the bubble.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Configure
GRID_SIZE = 100
RESONANCE_FREQ = 8.0 # rad/s in sim units

def run_animation():
    # Setup Grid
    x = np.linspace(-10, 10, GRID_SIZE)
    y = np.linspace(-10, 10, GRID_SIZE)
    X, Y = np.meshgrid(x, y)
    
    # Shield Geometry (Circle)
    R_shield = 2.5
    theta_shield = np.linspace(0, 2*np.pi, 20)
    shield_x = R_shield * np.cos(theta_shield)
    shield_y = R_shield * np.sin(theta_shield)
    
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Initial Field (Linear Gradient = Constant Force)
    # Phi = g * x
    g0 = 0.5
    
    ax.set_aspect('equal')
    ax.set_facecolor('black')
    
    TOTAL_FRAMES = 150
    
    def update(frame):
        t = frame * 0.1
        
        # Phase 1: Spin up (Frames 0-30)
        # Phase 2: Lock and Hold (Frames 30-50)
        # Phase 3: Move (Frames 50-150)
        
        if frame < 30:
            current_freq = min(RESONANCE_FREQ, t * 2.0)
            center_x = 0.0
            center_y = 0.0
        elif frame < 50:
            current_freq = RESONANCE_FREQ
            center_x = 0.0
            center_y = 0.0
        else:
            current_freq = RESONANCE_FREQ
            # Move Right and Up
            move_phase = (frame - 50) * 0.1
            center_x = 5.0 * np.sin(move_phase * 0.5)
            center_y = 2.0 * np.sin(move_phase)
            
        # Update Shield Geometry
        current_shield_x = shield_x + center_x
        current_shield_y = shield_y + center_y
        
        # Resonance Shielding Logic
        detuning = abs(current_freq - RESONANCE_FREQ)
        coupling = 1.0 - np.exp(-detuning**2 / 0.5) 
        
        # Calculate Field
        # Base: External Gradient
        Phi_base = g0 * X
        
        # Dipole moves with the craft
        # Relative coordinates to center
        X_rel = X - center_x
        Y_rel = Y - center_y
        R_sq = X_rel**2 + Y_rel**2
        R_sq[R_sq < 1.0] = 1.0 
        
        P_strength = (1.0 - coupling) * 5.0
        Phi_dipole = - P_strength * X_rel / (R_sq**1.5)
        
        interior_mask = R_sq < R_shield**2
        
        Phi_total = Phi_base + Phi_dipole
        
        # Flat interior moving with craft
        # Local potential avg at current center_x
        Phi_flat = g0 * center_x 
        Phi_total[interior_mask] = coupling * Phi_base[interior_mask] + (1-coupling)*Phi_flat
        
        # Update Plot
        ax.clear()
        
        # 1. Background Potential (Scalar)
        levels = np.linspace(-8, 8, 30)
        ax.contourf(X, Y, Phi_total, levels=levels, cmap='magma', alpha=0.6)
        
        # 2. Flux Field (Vector) - The "Flow" of Gravity
        # Calculate gradient
        # Note: numpy gradient order is (axis 0=y, axis 1=x)
        dy, dx = np.gradient(-Phi_total)
        
        # Decimate for visibility (plot every 5th arrow)
        skip = 5
        ax.quiver(X[::skip, ::skip], Y[::skip, ::skip], 
                  dx[::skip, ::skip], dy[::skip, ::skip], 
                  color='cyan', alpha=0.4, scale=20, headwidth=3)
        
        # 3. Draw Shield
        pulse = 0.5 + 0.5*np.sin(current_freq * t)
        hull_color = 'lime' if coupling < 0.1 else 'white'
        ax.plot(current_shield_x, current_shield_y, 'o', color=hull_color, markersize=3)
        ax.plot([0, center_x], [0, center_y], 'r:', alpha=0.3) # Path
        
        # 4. Telemetry HUD
        status = "STANDBY"
        if frame < 30: status = "CHARGING"
        elif frame < 50: status = "LOCKING"
        elif frame < 150: status = "TRANSLATING"
        
        telemetry = (
            f"SYSTEM STATUS: {status}\n"
            f"----------------------\n"
            f"Drive Freq:  {current_freq:.2f} THz\n"
            f"Resonance:   {detuning:.3f}\n"
            f"G-Coupling:  {coupling*100:.1f} %\n"
            f"Velocity:    {(g0*center_x):.1f} c\n"
            f"Local Grad:  {(coupling*g0):.4f}"
        )
        
        # Add text box
        props = dict(boxstyle='round', facecolor='black', alpha=0.8)
        ax.text(0.02, 0.98, telemetry, transform=ax.transAxes, fontsize=9,
                verticalalignment='top', color='lime', bbox=props, fontname='monospace')
        
        ax.set_title(f"Visualizing the Flux Exclusion Principle", color='white')
        
        ax.set_xlim(-8, 8)
        ax.set_ylim(-8, 8)
        ax.set_xticks([])
        ax.set_yticks([])
        
    ani = animation.FuncAnimation(fig, update, frames=np.arange(0, TOTAL_FRAMES), interval=50)
    
    # Save
    print("Generating animation (HUD + Vectors)...")
    save_path = 'antigravity_hud.gif'
    ani.save(save_path, writer='pillow', fps=15)
    print(f"Saved to {save_path}")

if __name__ == "__main__":
    run_animation()
