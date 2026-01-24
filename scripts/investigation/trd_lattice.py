"""
TRD Lattice Generator: A Small Existence
=========================================
Creates a 3D grid of heptagonal antiprism cells representing
the Ternary Realization Dynamics discrete spacetime substrate.

Each cell encodes:
- 7 upper vertices (+1 states, positive manifestation)
- 7 lower vertices (-1 states, negative manifestation)
- Central origin (0 state, void substrate)
- Twisted edges (reflexive coupling J(t)·J(t-τ))

Author: Generated for TRD Framework Visualization
"""

import bpy
import bmesh
import math
from mathutils import Vector, Matrix

# =============================================================================
# CONFIGURATION - TRD Framework Constants
# =============================================================================

# Grid dimensions (number of cells in each direction)
GRID_X = 3
GRID_Y = 3
GRID_Z = 2

# Cell geometry
N_SIDES = 7  # b₃ = 7 (Gauss constraints)
CELL_RADIUS = 1.0  # Radius of heptagon
CELL_HEIGHT = 1.5  # Height between top/bottom heptagons
CELL_SPACING = 2.5  # Distance between cell centers

# Antiprism twist angle (radians)
# For antiprism: twist = π/N for proper triangulation
TWIST_ANGLE = math.pi / N_SIDES

# Visual settings
VERTEX_RADIUS = 0.08
EDGE_RADIUS = 0.025
SHOW_VOID_CENTER = True
VOID_RADIUS = 0.05

# Colors (RGB, 0-1 scale)
COLOR_POSITIVE = (1.0, 0.85, 0.8, 1.0)  # Warm white/pink for +1
COLOR_NEGATIVE = (0.8, 0.85, 1.0, 1.0)  # Cool white/blue for -1
COLOR_EDGE = (0.7, 0.95, 0.95, 1.0)     # Cyan for edges
COLOR_VOID = (0.3, 0.0, 0.3, 1.0)       # Deep purple for void
COLOR_FACE = (0.3, 0.2, 0.5, 0.3)       # Purple transparent for faces

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

def clear_scene():
    """Remove all mesh objects from the scene."""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)

    # Clear orphan data
    for mesh in bpy.data.meshes:
        bpy.data.meshes.remove(mesh)
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)

def create_material(name, color, emission_strength=0.0):
    """Create an emissive material."""
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links

    # Clear default nodes
    nodes.clear()

    # Create nodes
    output = nodes.new('ShaderNodeOutputMaterial')
    principled = nodes.new('ShaderNodeBsdfPrincipled')

    # Set color and emission
    principled.inputs['Base Color'].default_value = color
    if emission_strength > 0:
        principled.inputs['Emission Color'].default_value = color
        principled.inputs['Emission Strength'].default_value = emission_strength

    # Link
    links.new(principled.outputs['BSDF'], output.inputs['Surface'])

    output.location = (300, 0)
    principled.location = (0, 0)

    return mat

def create_sphere(location, radius, material):
    """Create a UV sphere at the given location."""
    bpy.ops.mesh.primitive_uv_sphere_add(
        radius=radius,
        location=location,
        segments=16,
        ring_count=8
    )
    obj = bpy.context.active_object
    obj.data.materials.append(material)
    bpy.ops.object.shade_smooth()
    return obj

def create_cylinder_between(p1, p2, radius, material):
    """Create a cylinder (edge) between two points."""
    # Calculate midpoint and direction
    mid = (p1 + p2) / 2
    direction = p2 - p1
    length = direction.length

    # Create cylinder
    bpy.ops.mesh.primitive_cylinder_add(
        radius=radius,
        depth=length,
        location=mid
    )
    obj = bpy.context.active_object

    # Rotate to align with direction
    up = Vector((0, 0, 1))
    if direction.normalized() != up and direction.normalized() != -up:
        rot_axis = up.cross(direction).normalized()
        rot_angle = math.acos(up.dot(direction.normalized()))
        obj.rotation_mode = 'AXIS_ANGLE'
        obj.rotation_axis_angle = (rot_angle, rot_axis.x, rot_axis.y, rot_axis.z)
    elif direction.normalized() == -up:
        obj.rotation_euler = (math.pi, 0, 0)

    obj.data.materials.append(material)
    bpy.ops.object.shade_smooth()
    return obj

# =============================================================================
# ANTIPRISM CELL CONSTRUCTION
# =============================================================================

def create_heptagonal_antiprism(center, materials):
    """
    Create a single heptagonal antiprism cell at the given center.

    Returns lists of created objects for potential grouping.
    """
    objects = []
    cx, cy, cz = center

    mat_pos, mat_neg, mat_edge, mat_void = materials

    # Generate vertex positions
    top_verts = []  # +1 states
    bot_verts = []  # -1 states

    for i in range(N_SIDES):
        # Top heptagon (no twist)
        angle_top = 2 * math.pi * i / N_SIDES
        x_top = cx + CELL_RADIUS * math.cos(angle_top)
        y_top = cy + CELL_RADIUS * math.sin(angle_top)
        z_top = cz + CELL_HEIGHT / 2
        top_verts.append(Vector((x_top, y_top, z_top)))

        # Bottom heptagon (twisted by π/7 for antiprism)
        angle_bot = 2 * math.pi * i / N_SIDES + TWIST_ANGLE
        x_bot = cx + CELL_RADIUS * math.cos(angle_bot)
        y_bot = cy + CELL_RADIUS * math.sin(angle_bot)
        z_bot = cz - CELL_HEIGHT / 2
        bot_verts.append(Vector((x_bot, y_bot, z_bot)))

    # Create vertex spheres
    for v in top_verts:
        obj = create_sphere(v, VERTEX_RADIUS, mat_pos)
        obj.name = "Vertex_Positive"
        objects.append(obj)

    for v in bot_verts:
        obj = create_sphere(v, VERTEX_RADIUS, mat_neg)
        obj.name = "Vertex_Negative"
        objects.append(obj)

    # Create void center
    if SHOW_VOID_CENTER:
        void_pos = Vector((cx, cy, cz))
        obj = create_sphere(void_pos, VOID_RADIUS, mat_void)
        obj.name = "Void_Center"
        objects.append(obj)

    # Create edges
    # Top heptagon edges
    for i in range(N_SIDES):
        p1 = top_verts[i]
        p2 = top_verts[(i + 1) % N_SIDES]
        obj = create_cylinder_between(p1, p2, EDGE_RADIUS, mat_edge)
        obj.name = "Edge_Top"
        objects.append(obj)

    # Bottom heptagon edges
    for i in range(N_SIDES):
        p1 = bot_verts[i]
        p2 = bot_verts[(i + 1) % N_SIDES]
        obj = create_cylinder_between(p1, p2, EDGE_RADIUS, mat_edge)
        obj.name = "Edge_Bottom"
        objects.append(obj)

    # Antiprism lateral edges (the twisted connections)
    for i in range(N_SIDES):
        # Each top vertex connects to two bottom vertices
        # Top[i] -> Bot[i] and Top[i] -> Bot[i-1]
        p1 = top_verts[i]
        p2 = bot_verts[i]
        obj = create_cylinder_between(p1, p2, EDGE_RADIUS, mat_edge)
        obj.name = "Edge_Lateral"
        objects.append(obj)

        p3 = bot_verts[(i - 1) % N_SIDES]
        obj = create_cylinder_between(p1, p3, EDGE_RADIUS, mat_edge)
        obj.name = "Edge_Lateral"
        objects.append(obj)

    return objects

# =============================================================================
# LATTICE CONSTRUCTION
# =============================================================================

def create_trd_lattice():
    """
    Create the full TRD lattice grid - a 'small existence'.
    """
    print("=" * 60)
    print("TRD LATTICE GENERATOR: Creating a Small Existence")
    print("=" * 60)
    print(f"Grid: {GRID_X} × {GRID_Y} × {GRID_Z} cells")
    print(f"Cell symmetry: {N_SIDES}-fold (b₃ = 7)")
    print(f"Vertices per cell: {2 * N_SIDES} = 14")
    print(f"Total cells: {GRID_X * GRID_Y * GRID_Z}")
    print(f"Total vertices: {GRID_X * GRID_Y * GRID_Z * 2 * N_SIDES}")
    print("=" * 60)

    # Clear existing scene
    clear_scene()

    # Create materials
    mat_positive = create_material("TRD_Positive", COLOR_POSITIVE, emission_strength=2.0)
    mat_negative = create_material("TRD_Negative", COLOR_NEGATIVE, emission_strength=2.0)
    mat_edge = create_material("TRD_Edge", COLOR_EDGE, emission_strength=1.0)
    mat_void = create_material("TRD_Void", COLOR_VOID, emission_strength=0.5)

    materials = (mat_positive, mat_negative, mat_edge, mat_void)

    all_objects = []

    # Create grid of cells
    for ix in range(GRID_X):
        for iy in range(GRID_Y):
            for iz in range(GRID_Z):
                # Calculate cell center
                # Offset every other layer for hexagonal-like packing
                x_offset = (iy % 2) * CELL_SPACING * 0.5

                cx = ix * CELL_SPACING + x_offset
                cy = iy * CELL_SPACING * math.sqrt(3) / 2
                cz = iz * CELL_HEIGHT * 1.2

                center = (cx, cy, cz)

                print(f"Creating cell at ({ix}, {iy}, {iz}) -> {center}")

                cell_objects = create_heptagonal_antiprism(center, materials)
                all_objects.extend(cell_objects)

    # Create a collection for organization
    collection = bpy.data.collections.new("TRD_Lattice")
    bpy.context.scene.collection.children.link(collection)

    for obj in all_objects:
        # Unlink from default collection
        for coll in obj.users_collection:
            coll.objects.unlink(obj)
        # Link to TRD collection
        collection.objects.link(obj)

    # Set up camera and lighting
    setup_scene()

    print("=" * 60)
    print("CREATION COMPLETE")
    print(f"Total objects created: {len(all_objects)}")
    print("=" * 60)

    return all_objects

def setup_scene():
    """Set up camera, lighting, and render settings."""

    # Add camera
    bpy.ops.object.camera_add(
        location=(GRID_X * CELL_SPACING, -GRID_Y * CELL_SPACING * 1.5, GRID_Z * CELL_HEIGHT * 2)
    )
    camera = bpy.context.active_object
    camera.name = "TRD_Camera"

    # Point camera at center of lattice
    center_x = (GRID_X - 1) * CELL_SPACING / 2
    center_y = (GRID_Y - 1) * CELL_SPACING * math.sqrt(3) / 4
    center_z = (GRID_Z - 1) * CELL_HEIGHT * 0.6

    direction = Vector((center_x, center_y, center_z)) - camera.location
    rot_quat = direction.to_track_quat('-Z', 'Y')
    camera.rotation_euler = rot_quat.to_euler()

    bpy.context.scene.camera = camera

    # Add sun light
    bpy.ops.object.light_add(type='SUN', location=(10, -10, 20))
    sun = bpy.context.active_object
    sun.name = "TRD_Sun"
    sun.data.energy = 2.0

    # Add ambient light
    bpy.ops.object.light_add(type='AREA', location=(0, 0, 10))
    ambient = bpy.context.active_object
    ambient.name = "TRD_Ambient"
    ambient.data.energy = 100
    ambient.data.size = 20

    # Set world background to black
    world = bpy.context.scene.world
    if world is None:
        world = bpy.data.worlds.new("TRD_World")
        bpy.context.scene.world = world
    world.use_nodes = True
    bg_node = world.node_tree.nodes.get('Background')
    if bg_node:
        bg_node.inputs['Color'].default_value = (0, 0, 0, 1)

    # Render settings
    bpy.context.scene.render.engine = 'CYCLES'
    bpy.context.scene.cycles.samples = 128
    bpy.context.scene.render.resolution_x = 1920
    bpy.context.scene.render.resolution_y = 1080

# =============================================================================
# ALTERNATIVE: SINGLE CELL FOR INSPECTION
# =============================================================================

def create_single_cell():
    """Create just one cell for detailed inspection."""
    clear_scene()

    mat_positive = create_material("TRD_Positive", COLOR_POSITIVE, emission_strength=2.0)
    mat_negative = create_material("TRD_Negative", COLOR_NEGATIVE, emission_strength=2.0)
    mat_edge = create_material("TRD_Edge", COLOR_EDGE, emission_strength=1.0)
    mat_void = create_material("TRD_Void", COLOR_VOID, emission_strength=0.5)

    materials = (mat_positive, mat_negative, mat_edge, mat_void)

    create_heptagonal_antiprism((0, 0, 0), materials)
    setup_scene()

# =============================================================================
# MAIN EXECUTION
# =============================================================================

if __name__ == "__main__":
    # Choose one:
    create_trd_lattice()      # Full grid
    # create_single_cell()    # Just one cell for inspection
