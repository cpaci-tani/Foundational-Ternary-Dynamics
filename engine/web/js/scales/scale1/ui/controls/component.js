/**
 * Scale 1 Controls Component
 * Mounts the Scale 1 (Particle Engine) control card into the controls panel.
 */

import { createPeControlsCard } from './pe-controls.js';

export class Scale1ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Find or create the panel grid that holds Scale 1 cards
    let gridContainer = this.panel.querySelector('.panel-grid.panel-grid-3');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    // Mount the Particle Engine controls card
    gridContainer.appendChild(createPeControlsCard());

    return this;
  }
}
