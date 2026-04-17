export const INSPECTOR_MODE_COPY = Object.freeze({
    lattice: {
        label: 'Lattice',
        prompt: 'Single-click a visible object in the viewport to inspect it. Camera drags will not trigger a selection.',
    },
    particles: {
        label: 'Particles',
        prompt: 'Single-click a particle in the viewport to inspect its identity and nearest interactions.',
    },
    atoms: {
        label: 'Atoms & Molecules',
        prompt: 'Single-click an atom or orbital cloud sample to inspect its local chemistry.',
    },
    planetary: {
        label: 'Planetary',
        prompt: 'Single-click a world or star to inspect its live telemetry.',
    },
    cosmic: {
        label: 'Cosmic',
        prompt: 'Single-click a body to inspect its mass, motion, and evolutionary state.',
    },
});

export function resetInspectorSelection(target) {
    target.selectedIndex = -1;
    target._selectedPos = null;
    target._selectedPEParticleId = -1;
    target._selectedAEAtomId = -1;
    target._selectedPlanetaryId = -1;
    target._selectedCosmicId = -1;
}

export function hasInspectorSelection(target) {
    return !!(
        target._selectedPos ||
        target._selectedPEParticleId >= 0 ||
        target._selectedAEAtomId >= 0 ||
        target._selectedPlanetaryId >= 0 ||
        target._selectedCosmicId >= 0
    );
}

export function getInspectorModeCopy(mode) {
    return INSPECTOR_MODE_COPY[mode] || INSPECTOR_MODE_COPY.lattice;
}

export function getInspectorSelectionSummary(target) {
    if (target._selectedPos) {
        const { x, y, z } = target._selectedPos;
        return `Selected voxel at (${x}, ${y}, ${z}).`;
    }
    if (target._selectedPEParticleId >= 0) return `Selected particle #${target._selectedPEParticleId}.`;
    if (target._selectedAEAtomId >= 0) return `Selected atom #${target._selectedAEAtomId}.`;
    if (target._selectedPlanetaryId >= 0) return `Selected planetary body #${target._selectedPlanetaryId}.`;
    if (target._selectedCosmicId >= 0) return `Selected cosmic body #${target._selectedCosmicId}.`;
    return getInspectorModeCopy(target._engineMode).prompt;
}

export function updateInspectorChrome(target) {
    const modeCopy = getInspectorModeCopy(target._engineMode);
    if (target.modeLabelEl) target.modeLabelEl.textContent = modeCopy.label;
    if (target.selectionSummaryEl) {
        target.selectionSummaryEl.textContent = getInspectorSelectionSummary(target);
    }
    if (target.clearSelectionBtn) {
        target.clearSelectionBtn.disabled = !hasInspectorSelection(target);
    }
    if (target.focusSelectionBtn) {
        target.focusSelectionBtn.disabled = !(target._engineMode === 'lattice' && target._selectedPos);
    }
}

export function setInspectorSectionVisibility(emptyEl, contentEl, isVisible) {
    if (emptyEl) emptyEl.style.display = isVisible ? 'none' : 'block';
    if (contentEl) contentEl.style.display = isVisible ? 'block' : 'none';
}
