/** Shared approximate spectral display response for all Observer materials. */
export const LINE_SPECTRUM_GLSL = `
vec3 spectrum(float wavelength){
    vec3 z=(vec3(wavelength)-vec3(610.,545.,455.))/vec3(35.,30.,25.);
    // Signed offsets must be multiplied: GLSL pow(x,y) is undefined for x<0.
    return exp(-.5*(z*z));
}
`;
