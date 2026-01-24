from manim import *
import numpy as np

class MinimalCell(ThreeDScene):
    def construct(self):
        self.set_camera_orientation(phi=75 * DEGREES, theta=30 * DEGREES)
        
        # Title
        title = Text("The Minimal Cell (2x2x2)", font_size=40)
        self.add_fixed_in_frame_mobjects(title)
        title.to_corner(UL)
        
        # 1. Draw the Vertices (8 Points)
        vertices = []
        # 2x2x2 grid centered at origin
        # Coordinates: -1 and 1
        for x in [-1, 1]:
            for y in [-1, 1]:
                for z in [-1, 1]:
                    v = list([x, y, z])
                    vertices.append(v)
        
        dots = VGroup(*[Dot3D(point=v, color=WHITE) for v in vertices])
        
        self.play(Write(title))
        self.play(Create(dots))
        self.wait()
        
        # 2. Draw the Edges (Lattice Structure)
        lines = VGroup()
        for i in range(len(vertices)):
            p1 = vertices[i]
            for j in range(i+1, len(vertices)):
                p2 = vertices[j]
                # If distance is 2 (adjacent in one dim)
                dist = np.linalg.norm(np.array(p1)-np.array(p2))
                if np.isclose(dist, 2.0):
                    lines.add(Line3D(start=p1, end=p2, color=GRAY, fill_opacity=0.2))
        
        self.play(Create(lines))
        self.begin_ambient_camera_rotation(rate=0.2)
        
        # 3. Flux Vectors (Red Arrows)
        # 3 components per vertex * 8 vertices = 24 DOFs
        vectors = VGroup()
        for v in vertices:
            # Small arrows pointing x, y, z
            start = np.array(v)
            scale = 0.5
            arrow_x = Arrow3D(start, start + np.array([scale, 0, 0]), color=RED)
            arrow_y = Arrow3D(start, start + np.array([0, scale, 0]), color=GREEN)
            arrow_z = Arrow3D(start, start + np.array([0, 0, scale]), color=BLUE)
            vectors.add(arrow_x, arrow_y, arrow_z)
            
        self.play(Create(vectors))
        
        count_text = Text("Total Flux Components: 24", font_size=24, color=RED)
        self.add_fixed_in_frame_mobjects(count_text)
        count_text.next_to(title, DOWN)
        self.play(Write(count_text))
        self.wait(2)
        
        # 4. Constraints
        # -7 Gauss Constraints (Div J = 0)
        # -1 Global Gauge
        constraint_text = Text("Constraints: -8 (Gauss + Gauge)", font_size=24, color=YELLOW)
        self.add_fixed_in_frame_mobjects(constraint_text)
        constraint_text.next_to(count_text, DOWN)
        
        self.play(Write(constraint_text))
        
        # Visualize constraint (Flash vertices)
        self.play(dots.animate.set_color(YELLOW), run_time=0.5)
        self.play(dots.animate.set_color(WHITE), run_time=0.5)
        
        # 5. Result
        result_text = Text("Physical Degrees of Freedom: 16", font_size=32, color=BLUE)
        self.add_fixed_in_frame_mobjects(result_text)
        result_text.to_edge(DOWN)
        
        self.play(Transform(count_text, result_text), FadeOut(constraint_text))
        self.wait(3)
