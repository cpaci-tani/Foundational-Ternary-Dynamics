/** Ordinary least-squares of y on x. Pure display arithmetic: every guard, unit and sign stays with the caller. */
export function leastSquares(xs, ys) {
    const n = xs.length;
    const mx = xs.reduce((sum, x) => sum + x, 0) / n;
    const my = ys.reduce((sum, y) => sum + y, 0) / n;
    const sxx = xs.reduce((sum, x) => sum + (x - mx) ** 2, 0);
    const sxy = xs.reduce((sum, x, i) => sum + (x - mx) * (ys[i] - my), 0);
    const slope = sxy / sxx;
    const residual = xs.reduce((sum, x, i) => sum + (ys[i] - my - slope * (x - mx)) ** 2, 0);
    const syy = ys.reduce((sum, y) => sum + (y - my) ** 2, 0);
    return { n, mx, my, sxx, sxy, slope, residual, syy };
}
