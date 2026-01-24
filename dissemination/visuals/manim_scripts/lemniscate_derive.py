from manim import *
import numpy as np

class LemniscateDerivation(Scene):
    def construct(self):
        self.intro()
        self.harmonic_summation()
        self.alpha_lock()

    def intro(self):
        title = Text("The Geometry of Alpha", font_size=48)
        subtitle = Text("Deriving the Fine Structure Constant", font_size=24, color=GRAY)
        subtitle.next_to(title, DOWN)
        
        self.play(Write(title), Write(subtitle))
        self.wait(2)
        self.play(FadeOut(title), FadeOut(subtitle))

    def harmonic_summation(self):
        # Coefficients from FTD v5.8
        # x(t) terms: 1, 1/2, 1/2, 2/5, 1/16
        # y(t) terms: 1, -1/2, 1/2, -7/20, 1/16
        # freqs: 1, 2, 4, 8, 16
        
        coeffs = [
            (1.0, 1.0, 1),       # Mode 1
            (0.5, -0.5, 2),      # Mode 2
            (0.5, 0.5, 4),       # Mode 4
            (0.4, -0.35, 8),     # Mode 8
            (1/16, 1/16, 16)     # Mode 16
        ]
        
        plane = NumberPlane(x_range=[-3, 3], y_range=[-3, 3]).set_opacity(0.3)
        self.play(Create(plane))
        
        # Trackers for time
        t = ValueTracker(0)
        
        # Definition of the curve point trace
        def get_point(time_val):
            x, y = 0, 0
            for ax, ay, f in coeffs:
                x += ax * np.cos(f * time_val)
                y += ay * np.sin(f * time_val)
            return np.array([x, y, 0])

        # Vectors for each harmonic
        vectors = VGroup()
        for i in range(len(coeffs)):
            vectors.add(Arrow(ORIGIN, ORIGIN, buff=0))
            
        def update_vectors(v_group):
            time_val = t.get_value()
            current_origin = np.array([0.0, 0.0, 0.0])
            
            for i, (ax, ay, f) in enumerate(coeffs):
                dx = ax * np.cos(f * time_val)
                dy = ay * np.sin(f * time_val)
                vec_end = current_origin + np.array([dx, dy, 0])
                
                v_group[i].put_start_and_end_on(current_origin, vec_end)
                v_group[i].set_color(interpolate_color(BLUE, RED, i/len(coeffs)))
                
                current_origin = vec_end

        vectors.add_updater(update_vectors)
        self.add(vectors)
        
        # Trace the path
        path = TracedPath(lambda: vectors[-1].get_end(), stroke_color=YELLOW, stroke_width=3)
        self.add(path)
        
        # Run the animation
        self.play(t.animate.set_value(2 * np.pi), run_time=10, rate_func=linear)
        self.wait()
        
        # Final Curve Object
        final_curve = ParametricFunction(
            lambda u: get_point(u),
            t_range=[0, 2*np.pi],
            color=YELLOW
        )
        self.add(final_curve)
        self.remove(vectors, path)

    def alpha_lock(self):
        # Highlight: This simple harmonic sum yields G*
        
        # Calculate Arc Length L 
        # (Approximate visualization text)
        L_text = MathTex("L", "=", "23.7996...")
        L_text.to_corner(UL)
        
        G_text = MathTex("G^*", "=", "L", "\\times", "\\frac{91}{732}", "=", "2.9587...")
        G_text.next_to(L_text, DOWN)
        
        self.play(Write(L_text))
        self.wait()
        self.play(Write(G_text))
        self.wait()
        
        # Show Equation
        quad_text = MathTex("x^2", "-", "16(G^*)^2", "x", "+", "16(G^*)^3", "=", "0")
        quad_text.to_edge(DOWN)
        self.play(Write(quad_text))
        
        # Roots
        root_text = MathTex("x_+", "=", "137.036...", "=", "1/\\alpha")
        root_text.next_to(quad_text, UP)
        self.play(Write(root_text))
        
        self.wait(3)
