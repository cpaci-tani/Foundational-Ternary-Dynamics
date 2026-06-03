import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def main():
    print("Launching Yang-Mills Mass Gap (Confinement) Looping Animation...")
    fig = plt.figure(figsize=(10, 10), facecolor='black')
    ax = fig.add_subplot(111, projection='3d')
    ax.set_facecolor('black')
    
    GRID = 16
    voxels = np.zeros((GRID, GRID, GRID), dtype=bool)
    colors = np.empty(voxels.shape, dtype=object)

    mid = GRID // 2
    z = mid

    def update(frame):
        ax.clear()
        ax.set_facecolor('black')
        ax.set_axis_off()
        ax.set_title("Yang-Mills Confinement: Topological Flux Area Law", color='white', fontsize=18)
        
        # Expanding Wilson loop radius
        R = (frame % (mid - 2)) + 2
        
        voxels.fill(False)
        colors.fill(None)
        
        # Draw perimeter (the "quarks")
        for i in range(mid - R, mid + R + 1):
            voxels[i, mid - R, z] = True
            voxels[i, mid + R, z] = True
            voxels[mid - R, i, z] = True
            voxels[mid + R, i, z] = True
            
            colors[i, mid - R, z] = '#FF3333'
            colors[i, mid + R, z] = '#FF3333'
            colors[mid - R, i, z] = '#FF3333'
            colors[mid + R, i, z] = '#FF3333'
            
        # Draw area (the "string tension flux" holding them together)
        for ix in range(mid - R + 1, mid + R):
            for iy in range(mid - R + 1, mid + R):
                voxels[ix, iy, z] = True
                colors[ix, iy, z] = '#3357FF'
                
        # Draw the area law text
        ax.text2D(0.5, 0.05, f"Wilson Loop Area: {(2*R)**2} | String Tension Exponential Falloff", 
                  transform=ax.transAxes, color='white', fontsize=12, ha='center')
                
        ax.voxels(voxels, facecolors=colors, edgecolor='black', linewidth=0.8, alpha=0.9)
        
        # Gentle rotation
        ax.view_init(elev=35, azim=45 + (frame * 2) % 360)
        return ax,

    ani = animation.FuncAnimation(fig, update, frames=100, interval=250, blit=False)
    plt.show()

if __name__ == "__main__":
    main()
