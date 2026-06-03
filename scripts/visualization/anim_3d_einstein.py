import numpy as np
import sys

try:
    import pyvista as pv
except ImportError:
    print("PyVista is required. Run: python -m pip install pyvista")
    sys.exit(1)

def create_base_einstein_monotile(offset=(0, 0, 0)):
    blocks = [
        (0,0,0), (1,0,0), (0,1,0), (0,0,1), 
        (1,1,0), (2,1,0), (1,0,-1), (-1,1,1)
    ]
    mesh = pv.MultiBlock()
    for dx, dy, dz in blocks:
        mesh.append(pv.Cube(center=(offset[0] + dx, offset[1] + dy, offset[2] + dz)))
    return mesh.combine()

def main():
    print("Launching 3D Einstein Aperiodic Monotiles Looping Animation...")
    p = pv.Plotter()
    p.set_background('#121212')
    
    hex_colors = ['#FF5733', '#33FF57', '#3357FF', '#F333FF', '#33FFF5', '#F5FF33',
                  '#FF8333', '#33FF83', '#8333FF', '#FF3383', '#3383FF', '#83FF33']
    
    count = 0
    meshes = []
    for row in range(3):
        for col in range(4):
            x, y, z = col * 5, row * 6, 0
            tile = create_base_einstein_monotile(offset=(x, y, z))
            p.add_mesh(tile, color=hex_colors[count], show_edges=True, edge_color='white', line_width=2, smooth_shading=False)
            meshes.append(tile)
            count += 1
            
    p.add_text("FTD CUDA: 3D Aperiodic Monotiles\n(Looping 3D Rotation - Drag to override)", 
               position='upper_left', font_size=14, color='white')
    p.camera_position = 'iso'
    
    # Use PyVista's asynchronous timer to create a background rotation loop 
    # that still allows user interaction!
    def rotate_camera(step):
        p.camera.azimuth += 0.5
        
    p.add_timer_event(max_steps=1000000, duration=20, callback=rotate_camera)
    
    p.show()

if __name__ == "__main__":
    main()
