/**
 * Scale 0 Controls Component
 * Mounts all Scale 0 control cards into the controls panel
 */

import { createPhysicsTogglesCard } from './physics-toggles.js?v=3';
import { createSubstrateControlsCard } from './substrate-controls.js?v=2';
import { createFluxVolumeCard, createParticleDisplayCard, createSelectionCard } from './flux-volume.js?v=2';
import { createFlowLinesCard } from './flow-lines.js?v=1';

export class Scale0ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Find or create the panel grid that will hold Scale 0 cards.
    // Use the explicit #panel-controls-grid id — a plain `.panel-grid.panel-grid-3`
    // selector also matches the Scale 5 cosmic block inserted by
    // ensurePanelResources(), which is hidden in Scale 0 mode.
    let gridContainer = this.panel.querySelector('#panel-controls-grid');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.id = 'panel-controls-grid';
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    // Reconcile the six Scale-0 cards by stable ownership key. bindUI can run
    // again after an engine-mode round trip; replacing or appending cards on
    // re-entry duplicated IDs/listeners and discarded retained control state.
    const cards = [
      ['physics', createPhysicsTogglesCard],
      ['substrate', createSubstrateControlsCard],
      ['flux-volume', createFluxVolumeCard],
      ['flow-lines', createFlowLinesCard],
      ['particle-display', createParticleDisplayCard],
      ['selection', createSelectionCard],
    ];
    for (const [key, createCard] of cards) {
      if (gridContainer.querySelector(`[data-scale0-control-card="${key}"]`)) continue;
      const card = createCard();
      card.dataset.scale0ControlCard = key;
      gridContainer.appendChild(card);
    }

    return this;
  }
}
