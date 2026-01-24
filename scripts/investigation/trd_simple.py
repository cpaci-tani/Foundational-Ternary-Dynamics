"""
TRD SIMPLE - Clean, Fast, Beautiful
====================================
A streamlined version that just works.
"""

import bpy
import bmesh
import math
from mathutils import Vector

# =============================================================================
# CLEAR EVERYTHING
# =============================================================================

# Delete all objects
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

# Clear orphan data
for mesh in bpy.data.meshes:
    bpy.data.meshes.remove(mesh)
for mat in bpy.data.materials:
    bpy.data.materials.remove(mat)

print("\n" + "="*50)
print("  TRD SIMPLE GENERATOR")
print("="*50)

# =============================================================================
# SETTINGS
# =============================================================================

GRID = (2, 2, 1)  # Smaller grid for performance
SPACING = 3.0
N = 7  # Heptagon (b₃ = 7)
RADIUS = 1.0
HEIGHT = 1.5

# =============================================================================
# MATERIALS
# =============================================================================

def make_mat(name, color, emit=1.0):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Emission Color"].default_value = color
    bsdf.inputs["Emission Strength"].default_value = emit
    return mat

mat_pos = make_mat("Positive", (1.0, 0.9, 0.8, 1), 2.0)
mat_neg = make_mat("Negative", (0.8, 0.9, 1.0, 1), 2.0)
mat_void = make_mat("Void", (0.5, 0.2, 0.8, 1), 1.0)
mat_edge = make_mat("Edge", (0.5, 0.9, 0.95, 1), 1.0)

# =============================================================================
# BUILD CELLS
# =============================================================================

cell_count = 0

for ix in range(GRID[0]):
    for iy in range(GRID[1]):
        for iz in range(GRID[2]):
            # Cell center
            cx = ix * SPACING
            cy = iy * SPACING
            cz = iz * HEIGHT * 1.2

            # Generate vertices
            top_verts = []
            bot_verts = []
            twist = math.pi / N

            for i in range(N):
                angle_top = 2 * math.pi * i / N
                angle_bot = angle_top + twist

                top_verts.append(Vector((
                    cx + RADIUS * math.cos(angle_top),
                    cy + RADIUS * math.sin(angle_top),
                    cz + HEIGHT/2
                )))
                bot_verts.append(Vector((
                    cx + RADIUS * math.cos(angle_bot),
                    cy + RADIUS * math.sin(angle_bot),
                    cz - HEIGHT/2
                )))

            # Create vertex spheres - TOP (positive)
            for i, v in enumerate(top_verts):
                bpy.ops.mesh.primitive_uv_sphere_add(radius=0.12, location=v, segments=12, ring_count=8)
                obj = bpy.context.active_object
                obj.name = f"Cell{cell_count}_Top{i}"
                obj.data.materials.append(mat_pos)

            # Create vertex spheres - BOTTOM (negative)
            for i, v in enumerate(bot_verts):
                bpy.ops.mesh.primitive_uv_sphere_add(radius=0.10, location=v, segments=12, ring_count=8)
                obj = bpy.context.active_object
                obj.name = f"Cell{cell_count}_Bot{i}"
                obj.data.materials.append(mat_neg)

            # Create void center
            bpy.ops.mesh.primitive_uv_sphere_add(radius=0.06, location=(cx, cy, cz), segments=8, ring_count=6)
            obj = bpy.context.active_object
            obj.name = f"Cell{cell_count}_Void"
            obj.data.materials.append(mat_void)

            # Create edges using curves (lighter than cylinders)
            def make_edge(p1, p2, name):
                curve = bpy.data.curves.new(name, 'CURVE')
                curve.dimensions = '3D'
                curve.bevel_depth = 0.02

                spline = curve.splines.new('POLY')
                spline.points.add(1)
                spline.points[0].co = (*p1, 1)
                spline.points[1].co = (*p2, 1)

                obj = bpy.data.objects.new(name, curve)
                bpy.context.collection.objects.link(obj)
                obj.data.materials.append(mat_edge)
                return obj

            # Top edges
            for i in range(N):
                make_edge(top_verts[i], top_verts[(i+1) % N], f"Edge_T{cell_count}_{i}")

            # Bottom edges
            for i in range(N):
                make_edge(bot_verts[i], bot_verts[(i+1) % N], f"Edge_B{cell_count}_{i}")

            # Lateral edges
            for i in range(N):
                make_edge(top_verts[i], bot_verts[i], f"Edge_L{cell_count}_{i}a")
                make_edge(top_verts[i], bot_verts[(i-1) % N], f"Edge_L{cell_count}_{i}b")

            cell_count += 1
            print(f"  Cell {cell_count} created at ({cx:.1f}, {cy:.1f}, {cz:.1f})")

# =============================================================================
# CAMERA
# =============================================================================

cam_loc = (GRID[0] * SPACING * 1.5, -GRID[1] * SPACING * 1.2, GRID[2] * HEIGHT * 3)
bpy.ops.object.camera_add(location=cam_loc)
camera = bpy.context.active_object
camera.name = "Camera"

# Point at center
center = Vector((
    (GRID[0] - 1) * SPACING / 2,
    (GRID[1] - 1) * SPACING / 2,
    0
))
direction = center - Vector(cam_loc)
camera.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
bpy.context.scene.camera = camera

# =============================================================================
# LIGHTING
# =============================================================================

bpy.ops.object.light_add(type='SUN', location=(5, -5, 10))
sun = bpy.context.active_object
sun.data.energy = 2.0

bpy.ops.object.light_add(type='AREA', location=(0, 5, 5))
fill = bpy.context.active_object
fill.data.energy = 100
fill.data.size = 5

# =============================================================================
# WORLD
# =============================================================================

world = bpy.context.scene.world
if not world:
    world = bpy.data.worlds.new("World")
    bpy.context.scene.world = world
world.use_nodes = True
world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.01, 0.01, 0.02, 1)

# =============================================================================
# RENDER SETTINGS - USE EEVEE
# =============================================================================

bpy.context.scene.render.engine = 'BLENDER_EEVEE'
bpy.context.scene.eevee.use_bloom = True
bpy.context.scene.eevee.bloom_intensity = 0.05
bpy.context.scene.eevee.bloom_threshold = 0.8

# =============================================================================
# DONE
# =============================================================================

print("\n" + "="*50)
print("  COMPLETE!")
print("="*50)
print(f"""
  Cells: {cell_count}
  Vertices per cell: {N * 2 + 1} = {N*2+1}

  Framework encoded:
    N = {N} (b₃ = 7 Gauss constraints)
    14 boundary vertices = duality (+1/-1)
    1 void center = state 0

  Controls:
    Numpad 0 = Camera view
    Z = Shading pie menu (choose 'Rendered')
    Middle mouse = Orbit
    Scroll = Zoom

  The cells show:
    Pink/warm = +1 states (top heptagon)
    Blue/cool = -1 states (bottom heptagon)
    Purple = void substrate (center)
    Cyan = flux pathways (edges)
""")
print("="*50)
