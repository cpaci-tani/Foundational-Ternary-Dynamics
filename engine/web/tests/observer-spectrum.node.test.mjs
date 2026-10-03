import test from 'node:test';
import assert from 'node:assert/strict';
import {REFERENCE_WAVELENGTHS_NM,referenceLineWeights,spectrumToLinearDisplay,displayLineSpectrum} from '../js/observer/spectrum.js';
import {dopplerFactor} from '../js/observer/math.js';

test('reference wavelengths and frequency ratios transform before approximate RGB display',()=>{
    const c=299792458;
    for(const D of [.5,1,2])for(const wavelength of REFERENCE_WAVELENGTHS_NM){
        const sourceFrequency=c/(wavelength*1e-9),observedFrequency=D*sourceFrequency;
        assert.ok(Math.abs(c/observedFrequency*1e9-wavelength/D)<1e-12);
    }
    assert.equal(dopplerFactor([0,0,1],[0,0,.6],[0,0,0]),2);
    assert.equal(dopplerFactor([0,0,1],[0,0,-.6],[0,0,0]),.5);
    assert.equal(dopplerFactor([0,0,1],[0,0,0],[0,0,.6]),.5);
});

test('D=1 on/off identity and beaming independence hold for black, white, lines and mixtures',()=>{
    for(const color of [[1,1,1],[1,0,0],[0,1,0],[0,0,1],[.2,.5,.8],[0,0,0]])for(const spectral of ['white','red-line','green-line','blue-line']){
        const weights=referenceLineWeights(color,spectral);
        for(const beaming of [false,true])assert.deepEqual(displayLineSpectrum(weights,1,{doppler:false,beaming}),displayLineSpectrum(weights,1,{doppler:true,beaming}));
        for(const D of [.5,2]){
            const baseline=displayLineSpectrum(weights,D,{doppler:false,beaming:false});
            assert.deepEqual(baseline,spectrumToLinearDisplay(weights,1));
            const beamed=displayLineSpectrum(weights,D,{doppler:false,beaming:true});
            beamed.forEach((x,i)=>assert.equal(x,baseline[i]*D**4));
        }
    }
    assert.deepEqual(spectrumToLinearDisplay([0,0,0],.5),[0,0,0]);
    for(const D of [0,-1,NaN,Infinity])assert.throws(()=>spectrumToLinearDisplay([1,1,1],D));
});
