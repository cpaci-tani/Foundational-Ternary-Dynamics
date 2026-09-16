/** Direct nonnegative scalar observations, without an effective energy proxy. */
export function copyScalarActivation(density, activation) {
    let instantMax = 0;
    for (let i = 0; i < density.length; i++) {
        const value = Number(density[i]);
        activation[i] = Number.isFinite(value) && value >= 0 ? value : 0;
        instantMax = Math.max(instantMax, activation[i]);
    }
    return {instantMax};
}
