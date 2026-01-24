"""
═══════════════════════════════════════════════════════════════════════════════
    TRD UNIFIED LAUNCHER
    ═══════════════════════════════════════════════════════════════════════════

    One script to rule them all.

    Usage:
        1. Open in Blender Scripting workspace
        2. Run script
        3. Use the TRD panel in the sidebar (N key → TRD tab)

    Features:
        - One-click existence generation
        - Parameter controls in sidebar
        - Animation toggle
        - Quick render presets
        - Framework constant display

═══════════════════════════════════════════════════════════════════════════════
"""

import bpy
import sys
import os
from pathlib import Path

# Add the script directory to path for imports
script_dir = Path(bpy.data.filepath).parent if bpy.data.filepath else Path.cwd()
if str(script_dir) not in sys.path:
    sys.path.append(str(script_dir))

# =============================================================================
# BLENDER ADDON REGISTRATION
# =============================================================================

bl_info = {
    "name": "TRD Existence Generator",
    "author": "TRD Visualization System",
    "version": (1, 0, 0),
    "blender": (2, 80, 0),
    "location": "View3D > Sidebar > TRD",
    "description": "Generate Ternary Realization Dynamics visualizations",
    "category": "Object",
}


# =============================================================================
# PROPERTY GROUP
# =============================================================================

class TRDProperties(bpy.types.PropertyGroup):
    """Properties for the TRD panel."""

    grid_x: bpy.props.IntProperty(
        name="Grid X",
        description="Number of cells in X direction",
        default=3, min=1, max=10
    )
    grid_y: bpy.props.IntProperty(
        name="Grid Y",
        description="Number of cells in Y direction",
        default=3, min=1, max=10
    )
    grid_z: bpy.props.IntProperty(
        name="Grid Z",
        description="Number of cells in Z direction",
        default=2, min=1, max=5
    )
    cell_radius: bpy.props.FloatProperty(
        name="Cell Radius",
        description="Radius of heptagonal cell",
        default=1.0, min=0.5, max=3.0
    )
    show_flux: bpy.props.BoolProperty(
        name="Show Flux Vectors",
        description="Display flux field arrows",
        default=True
    )
    show_triads: bpy.props.BoolProperty(
        name="Show Triads",
        description="Display proto-nucleon structures",
        default=True
    )
    show_shells: bpy.props.BoolProperty(
        name="Show Shells",
        description="Display electron orbital shells",
        default=True
    )
    show_sloop: bpy.props.BoolProperty(
        name="Show sLoop",
        description="Display self-referential loop",
        default=True
    )
    animate: bpy.props.BoolProperty(
        name="Add Animation",
        description="Generate animated existence",
        default=False
    )
    animation_frames: bpy.props.IntProperty(
        name="Frames",
        description="Total animation frames",
        default=300, min=60, max=1000
    )


# =============================================================================
# OPERATORS
# =============================================================================

class TRD_OT_GenerateExistence(bpy.types.Operator):
    """Generate a TRD existence visualization."""

    bl_idname = "trd.generate_existence"
    bl_label = "Generate Existence"
    bl_description = "Create the TRD lattice visualization"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        props = context.scene.trd_props

        # Import and configure the existence generator
        try:
            # Try to import the full module
            import importlib

            # Check if trd_existence is already imported
            if 'trd_existence' in sys.modules:
                trd_existence = importlib.reload(sys.modules['trd_existence'])
            else:
                import trd_existence

            # Update configuration from properties
            trd_existence.VisualConfig.GRID_SIZE = (props.grid_x, props.grid_y, props.grid_z)
            trd_existence.VisualConfig.CELL_RADIUS = props.cell_radius

            # Generate
            existence = trd_existence.TRDExistence()
            existence.clear_scene()
            existence.setup_collections()
            existence.build_lattice()

            if props.show_flux:
                existence.build_flux_field()

            if props.show_triads or props.show_shells:
                existence.build_manifestations()

            if props.show_sloop:
                existence.build_sloop()

            existence.setup_environment()

            # Add animation if requested
            if props.animate:
                if 'trd_animation' in sys.modules:
                    trd_animation = importlib.reload(sys.modules['trd_animation'])
                else:
                    import trd_animation

                trd_animation.AnimConfig.FRAME_END = props.animation_frames
                animator = trd_animation.TRDAnimator()
                animator.animate_all()

            self.report({'INFO'}, f"Created existence with {props.grid_x}×{props.grid_y}×{props.grid_z} cells")

        except ImportError as e:
            self.report({'ERROR'}, f"Could not import modules: {e}")
            self.report({'INFO'}, "Running inline generation...")
            self.generate_inline(context)

        return {'FINISHED'}

    def generate_inline(self, context):
        """Fallback inline generation if modules aren't found."""
        import math
        import bmesh
        from mathutils import Vector

        props = context.scene.trd_props

        # Clear scene
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete()

        # Create a simple heptagonal antiprism
        n = 7
        radius = props.cell_radius
        height = 1.8

        for ix in range(props.grid_x):
            for iy in range(props.grid_y):
                for iz in range(props.grid_z):
                    cx = ix * 2.8
                    cy = iy * 2.8 * 0.866
                    cz = iz * 2.0

                    # Create vertices
                    for i in range(n):
                        angle = 2 * math.pi * i / n
                        x = cx + radius * math.cos(angle)
                        y = cy + radius * math.sin(angle)

                        bpy.ops.mesh.primitive_uv_sphere_add(radius=0.1, location=(x, y, cz + height/2))
                        bpy.ops.mesh.primitive_uv_sphere_add(radius=0.1, location=(x, y, cz - height/2))

        self.report({'INFO'}, "Generated simplified existence (inline mode)")


class TRD_OT_QuickRender(bpy.types.Operator):
    """Set up quick render settings."""

    bl_idname = "trd.quick_render"
    bl_label = "Quick Render Setup"
    bl_description = "Configure render settings for TRD visualization"

    preset: bpy.props.EnumProperty(
        name="Preset",
        items=[
            ('PREVIEW', "Preview", "Fast preview render"),
            ('QUALITY', "Quality", "High quality final render"),
            ('ANIMATION', "Animation", "Optimized for animation"),
        ],
        default='PREVIEW'
    )

    def execute(self, context):
        scene = context.scene
        scene.render.engine = 'CYCLES'

        if self.preset == 'PREVIEW':
            scene.cycles.samples = 64
            scene.render.resolution_x = 1280
            scene.render.resolution_y = 720
        elif self.preset == 'QUALITY':
            scene.cycles.samples = 512
            scene.render.resolution_x = 1920
            scene.render.resolution_y = 1080
        elif self.preset == 'ANIMATION':
            scene.cycles.samples = 128
            scene.render.resolution_x = 1920
            scene.render.resolution_y = 1080

        self.report({'INFO'}, f"Applied {self.preset} render preset")
        return {'FINISHED'}


class TRD_OT_ClearScene(bpy.types.Operator):
    """Clear the scene."""

    bl_idname = "trd.clear_scene"
    bl_label = "Clear Scene"
    bl_description = "Remove all TRD objects"

    def execute(self, context):
        bpy.ops.object.select_all(action='SELECT')
        bpy.ops.object.delete()
        self.report({'INFO'}, "Scene cleared")
        return {'FINISHED'}


# =============================================================================
# PANEL
# =============================================================================

class TRD_PT_MainPanel(bpy.types.Panel):
    """Main TRD control panel."""

    bl_label = "TRD Existence"
    bl_idname = "TRD_PT_main"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'TRD'

    def draw(self, context):
        layout = self.layout
        props = context.scene.trd_props

        # Header
        box = layout.box()
        box.label(text="Ternary Realization Dynamics", icon='OUTLINER_OB_MESH')
        box.label(text="Discrete Spacetime Visualizer")

        # Framework Constants
        box = layout.box()
        box.label(text="Framework Constants:", icon='INFO')
        col = box.column(align=True)
        col.label(text="b₃ = 7 (Gauss constraints)")
        col.label(text="N_c = 3 (Color charges)")
        col.label(text="n_eff = 13 (F₇)")
        col.label(text="N_base = 4 (Base structure)")

        layout.separator()

        # Grid Settings
        box = layout.box()
        box.label(text="Grid Configuration:", icon='MESH_GRID')
        row = box.row(align=True)
        row.prop(props, "grid_x", text="X")
        row.prop(props, "grid_y", text="Y")
        row.prop(props, "grid_z", text="Z")
        box.prop(props, "cell_radius")

        layout.separator()

        # Components
        box = layout.box()
        box.label(text="Components:", icon='PARTICLES')
        col = box.column(align=True)
        col.prop(props, "show_flux")
        col.prop(props, "show_triads")
        col.prop(props, "show_shells")
        col.prop(props, "show_sloop")

        layout.separator()

        # Animation
        box = layout.box()
        box.label(text="Animation:", icon='RENDER_ANIMATION')
        box.prop(props, "animate")
        if props.animate:
            box.prop(props, "animation_frames")

        layout.separator()

        # Generate Button
        row = layout.row(align=True)
        row.scale_y = 2.0
        row.operator("trd.generate_existence", text="Generate Existence", icon='WORLD')

        layout.separator()

        # Utility buttons
        row = layout.row(align=True)
        op = row.operator("trd.quick_render", text="Preview")
        op.preset = 'PREVIEW'
        op = row.operator("trd.quick_render", text="Quality")
        op.preset = 'QUALITY'

        layout.operator("trd.clear_scene", text="Clear Scene", icon='TRASH')

        # Stats
        box = layout.box()
        box.label(text="Statistics:", icon='LINENUMBERS_ON')
        total_cells = props.grid_x * props.grid_y * props.grid_z
        total_verts = total_cells * 15
        box.label(text=f"Cells: {total_cells}")
        box.label(text=f"Vertices: {total_verts}")


# =============================================================================
# REGISTRATION
# =============================================================================

classes = (
    TRDProperties,
    TRD_OT_GenerateExistence,
    TRD_OT_QuickRender,
    TRD_OT_ClearScene,
    TRD_PT_MainPanel,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.trd_props = bpy.props.PointerProperty(type=TRDProperties)
    print("TRD Visualization Suite registered")


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.trd_props
    print("TRD Visualization Suite unregistered")


if __name__ == "__main__":
    # Unregister if already registered
    try:
        unregister()
    except:
        pass

    register()

    print("""
═══════════════════════════════════════════════════════════════════════════════
  TRD VISUALIZATION SUITE LOADED
═══════════════════════════════════════════════════════════════════════════════

  Press N in the 3D Viewport to open the sidebar
  Navigate to the "TRD" tab
  Click "Generate Existence" to create the visualization

  Framework Constants Encoded:
    b₃ = 7      Heptagonal symmetry (Gauss constraints)
    N_c = 3     Triad structure (color charges)
    n_eff = 13  Effective dimension (F₇)
    N_base = 4  Base structure

  "Events are ontic. Constraints are real. Meaning is emergent."

═══════════════════════════════════════════════════════════════════════════════
""")
