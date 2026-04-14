import * as THREE from 'three';

export class BaseRenderer {
    constructor(scene, camera, renderer) {
        this.scene = scene;
        this.camera = camera;
        this.renderer = renderer;
        this._group = new THREE.Group();
        this.scene.add(this._group);

        this._meshes = [];
        this._lights = [];
        this._materials = [];
        
        this.clock = new THREE.Clock();
    }

    dispose() {
        if (this._group && this.scene) {
            this.scene.remove(this._group);
        }
        
        if (this._materials) {
            this._materials.forEach(m => {
                if (m) m.dispose();
            });
        }
        
        // Dispose mesh geometries
        if (this._meshes) {
            this._meshes.forEach(m => {
                if (m && m.geometry) m.geometry.dispose();
            });
        }

        // Let subclasses do additional geometry cleanup
        if (this._cleanGeometries) {
            this._cleanGeometries();
        }

        this._materials = [];
        this._meshes = [];
        this._lights = [];
        this._group = null;
    }
}
