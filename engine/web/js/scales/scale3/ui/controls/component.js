/**
 * Scale 3 Controls Component
 * Scale 3 (Molecules) shares the Atom Engine controls card with Scale 2.
 * The AE controls card uses class="card scale-ae" which is visible for
 * both scale2 and scale3. No additional scale3-specific cards are needed.
 */

export class Scale3ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    // Scale 3 inherits Scale 2's AE controls via scale-ae visibility.
    // No additional cards to mount here.
    return this;
  }
}
