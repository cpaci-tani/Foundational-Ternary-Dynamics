/** Production Schwarzschild black-hole null rays. A captured ray has no
 * emitting horizon. Unresolved rays have their own diagnostic color/status. */
import {SCHWARZSCHILD_GLSL} from './compact-star-shaders.js';
export const blackHoleFragmentShader=`
precision highp float;
precision highp int;
in vec2 vUv;
out vec4 fragColor;
uniform int uDebugMode;
uniform bool uOptical,uDoppler,uBeaming;
uniform vec2 uPickNdc;
uniform vec3 uVelocity,uForward,uRight,uUp;
uniform float uFov,uAspect;
float gm(vec3 v){return inversesqrt(max(1.-dot(v,v),.000001));}
vec3 boostSpace(vec3 x,float t,vec3 v){float g=gm(v);return x+(g*g/(g+1.)*dot(v,x)+g*t)*v;}
vec3 spectrum(float wavelength){return vec3(exp(-.5*pow((wavelength-610.)/35.,2.)),exp(-.5*pow((wavelength-545.)/30.,2.)),exp(-.5*pow((wavelength-455.)/25.,2.)));}
vec3 shifted(vec3 color,float factor){float D=uOptical&&uDoppler?factor:1.;return color.r*spectrum(610./D)+color.g*spectrum(545./D)+color.b*spectrum(455./D);}
${SCHWARZSCHILD_GLSL}
vec4 renderBlackHole(vec3 restDirection){
    float transformedTime=gm(uVelocity)*(-1.+dot(uVelocity,restDirection));
    vec3 initial=normalize(boostSpace(restDirection,-1.,uVelocity)/(-transformedTime));
    vec3 p=vec3(0),d=initial;float delay=0.,arc=0.;int status=3;
    float startRho=length(uStarCenter),photonRho=uStarRs*(1.+sqrt(.75))*.5;
    float impact=starN(vec3(0))*startRho*length(cross(normalize(-uStarCenter),initial)),critical=1.5*sqrt(3.)*uStarRs;
    float initialRadial=dot(-uStarCenter,initial);
    bool criticalApproach=startRho>=photonRho?initialRadial<0.:initialRadial>0.;
    bool criticalUncertain=criticalApproach&&abs(impact/critical-1.)<.00001;
    for(int step=0;step<1536;step++){
        float rho=length(p-uStarCenter);
        if(rho<photonRho*(1.-.00001)&&dot(p-uStarCenter,d)<0.){
            // The exact radial potential has no inner turning point here.
            // Near-critical Float32 rays remain explicitly unresolved.
            if(!criticalUncertain&&(startRho<photonRho||impact<critical*(1.-.00001)))status=1;
            break;
        }
        float h=min(min(.015*rho,.25),.5*(rho-uStarRadius));
        vec3 oldP=p,oldD=d;float oldDelay=delay;starStep(p,d,delay,h);
        if(length(p-uStarCenter)>=uStarEscape){
            float lo=0.,hi=h;
            for(int i=0;i<22;i++){
                float mid=(lo+hi)*.5;vec3 candidateP=oldP,candidateD=oldD;float candidateT=oldDelay;
                starStep(candidateP,candidateD,candidateT,mid);
                if(length(candidateP-uStarCenter)>=uStarEscape)hi=mid;else lo=mid;
            }
            float used=(lo+hi)*.5;p=oldP;d=oldD;delay=oldDelay;starStep(p,d,delay,used);arc+=used;status=criticalUncertain?3:2;break;
        }
        arc+=h;
    }
    float receiverA=starA(vec3(0)),emissionA=status==2?starA(p):0.;
    float D=status==2?emissionA/receiverA*gm(uVelocity)*(1.+dot(uVelocity,initial)):0.;
    if(uDebugMode==1)return vec4(0); // No surface hit or horizon clock.
    if(uDebugMode==2)return vec4(D,emissionA,receiverA,float(status));
    if(uDebugMode==4)return vec4(p,float(status));
    if(uDebugMode==5)return vec4(float(status),arc,delay,impact);
    if(uDebugMode==6)return vec4(d,starN(p)*length(p-uStarCenter)*length(cross(normalize(p-uStarCenter),d)));
    vec3 color=vec3(0);
    if(status==2){
        // Stationary patterned sky shell: geometry reveals lensing without an
        // atmosphere, disk, thermal source or magnification multiplier.
        vec3 sky=normalize(p-uStarCenter);
        float latitude=asin(clamp(sky.y,-1.,1.)),longitude=atan(sky.z,sky.x);
        float grid=pow(abs(cos(latitude*24.)*cos(longitude*24.)),32.);
        vec3 background=vec3(.015,.027,.045)+grid*vec3(.12,.3,.5);
        color=shifted(background,D)*(uBeaming?pow(D,4.):1.);
    }else if(status==3)color=vec3(.13,.045,.008); // Distinct from a captured shadow.
    if(uDebugMode==3)return vec4(color,1.);
    color=pow(vec3(1)-exp(-max(color,vec3(0))),vec3(1./2.2));return vec4(color*(1.-.2*pow(length(vUv-.5),2.)),1.);
}
void main(){
    vec2 ndc=uDebugMode>0?uPickNdc:vUv*2.-1.;float tangent=tan(uFov*.00872664626);
    vec3 restDirection=normalize(uForward+ndc.x*tangent*uAspect*uRight+ndc.y*tangent*uUp);
    fragColor=renderBlackHole(restDirection);
}
`;
