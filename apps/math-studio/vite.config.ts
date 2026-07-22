import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  server: { port: 4173 },
  build: {
    sourcemap: true,
    rollupOptions: {
      output: {
        manualChunks(id) {
          if (id.includes("node_modules/three")) return "three";
          if (id.includes("node_modules/mathjs") || id.includes("node_modules/@babel/runtime")) return "math";
          if (id.includes("node_modules/react") || id.includes("node_modules/zustand") || id.includes("node_modules/use-sync-external-store")) return "react";
          if (id.includes("node_modules/lucide-react")) return "icons";
          return undefined;
        }
      }
    }
  },
  worker: { format: "es" }
});
