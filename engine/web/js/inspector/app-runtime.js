import * as THREE from 'three';
import { Inspector } from '../inspector.js';

export function createInspectorAppRuntime({ viewport, bridge, setZooMode }) {
    const inspector = new Inspector(viewport, bridge);
    let symmetryPanelEl = null;
    let symmetryVector = null;

    function syncMode(mode) {
        inspector.setEngineMode(mode);
        viewport?.setEngineMode?.(mode);
        setZooMode?.(mode);
    }

    function updateFloatingPanels() {
        if (!symmetryPanelEl) {
            symmetryPanelEl = document.getElementById('floating-symmetry-panel');
        }
        const pos = inspector.getSelectedLatticePosition?.();
        if (!symmetryPanelEl || symmetryPanelEl.style.display !== 'block' || !pos || !viewport?.camera) {
            return;
        }
        if (!symmetryVector) symmetryVector = new THREE.Vector3();

        symmetryVector.set(pos.x, pos.y, pos.z);
        symmetryVector.project(viewport.camera);

        const halfW = window.innerWidth / 2;
        const halfH = window.innerHeight / 2;
        const xOffset = (symmetryVector.x * halfW) + halfW;
        const yOffset = -(symmetryVector.y * halfH) + halfH;

        if (symmetryVector.z < 1) {
            symmetryPanelEl.style.left = `${xOffset + 20}px`;
            symmetryPanelEl.style.top = `${yOffset - 20}px`;
        } else {
            symmetryPanelEl.style.left = '-9999px';
        }
    }

    function setBridge(nextBridge) {
        inspector.setBridge(nextBridge);
    }

    return {
        inspector,
        setBridge,
        syncMode,
        updateFloatingPanels,
    };
}
