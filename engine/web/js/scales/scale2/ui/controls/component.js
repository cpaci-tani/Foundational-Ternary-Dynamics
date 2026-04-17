/**
 * Scale 2 Controls Component
 * Mounts the Scale 2 (Atom Engine) control card into the controls panel.
 */

import { createAeControlsCard } from './ae-controls.js';

export class Scale2ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Find or create the panel grid that holds Scale 2 cards
    let gridContainer = this.panel.querySelector('.panel-grid.panel-grid-3');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    // Mount the Atom Engine controls card
    gridContainer.appendChild(createAeControlsCard());

    return this;
  }
}
