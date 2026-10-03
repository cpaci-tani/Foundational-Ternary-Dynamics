/** Actual WebGL2 Schwarzschild null-geodesic integration. Paired Float64 oracle
 * lives in compact-star.js; GPU parity is measured by float-target readback.
 */
export const COMPACT_STAR_GLSL = `
uniform vec3 uStarCenter;
uniform float uStarRs,uStarRadius,uStarEscape;
float starA(vec3 p){float q=uStarRs/(4.*length(p-uStarCenter));return (1.-q)/(1.+q);}
float starN(vec3 p){float q=uStarRs/(4.*length(p-uStarCenter));return pow(1.+q,3.)/(1.-q);}
vec3 starGradient(vec3 p){vec3 r=p-uStarCenter;float rho=length(r),q=uStarRs/(4.*rho);return -r/(rho*rho)*q*(3./(1.+q)+1./(1.-q));}
vec3 starBend(vec3 p,vec3 d){vec3 g=starGradient(p);return g-d*dot(d,g);}
void starStep(inout vec3 p,inout vec3 d,inout float delay,float h){
    vec3 k1p=d,k1d=starBend(p,d);float k1t=starN(p);
    vec3 p2=p+k1p*h*.5,d2=d+k1d*h*.5,k2d=starBend(p2,d2);float k2t=starN(p2);
    vec3 p3=p+d2*h*.5,d3=d+k2d*h*.5,k3d=starBend(p3,d3);float k3t=starN(p3);
    vec3 p4=p+d3*h,d4=d+k3d*h,k4d=starBend(p4,d4);float k4t=starN(p4);
    p+=h*(k1p+2.*d2+2.*d3+d4)/6.;d=normalize(d+h*(k1d+2.*k2d+2.*k3d+k4d)/6.);
    delay+=h*(k1t+2.*k2t+2.*k3t+k4t)/6.;
}
float starDistance(vec3 p){return length(p-uStarCenter)-uStarRadius;}
vec4 renderCompactStar(vec3 restDirection){
    float transformedTime=gm(uVelocity)*(-1.+dot(uVelocity,restDirection));
    vec3 initial=normalize(boostSpace(restDirection,-1.,uVelocity)/(-transformedTime));
    vec3 p=vec3(0),d=initial;float delay=0.,arc=0.;int picked=-1;bool escaped=false;
    for(int step=0;step<768;step++){
        float nearest=starDistance(p);
        if(nearest<=.00001){picked=0;break;}
        float rho=length(p-uStarCenter);if(rho>=uStarEscape){escaped=true;break;}
        float h=min(min(.04*rho,.5),max(.000001,.8*nearest));starStep(p,d,delay,h);arc+=h;
    }
    float Aemit=picked==0?starA(uStarCenter+vec3(uStarRadius,0,0)):starA(p);
    float D=Aemit/starA(vec3(0))*gm(uVelocity)*(1.+dot(uVelocity,initial));
    if(picked>=0&&(-delay<max(record(picked,2).w,uHistoryStart)||-delay>=record(picked,3).w))picked=-1;
    if(uDebugMode==1){return picked<0?vec4(0):vec4(float(picked+1),arc,-delay,-Aemit*delay);}
    if(uDebugMode==2){return vec4(D,Aemit,starA(vec3(0)),escaped?1.:0.);}
    if(uDebugMode==4){return picked<0?vec4(0):vec4(p,1.);}
    vec3 color;
    if(picked>=0){
        vec4 material=record(picked,6);color=shifted(material.rgb,D)*material.a*(uBeaming?pow(D,4.):1.);
        if(picked==0){
            // Surface markings reveal the extra visible hemisphere. These are
            // prescribed emissivity, with no temperature or equation-of-state claim.
            vec3 normal=normalize(p-uStarCenter);
            float check=sin(atan(normal.z,normal.x)*12.)*sin(acos(clamp(normal.y,-1.,1.))*12.);
            color*=.7+.3*smoothstep(-.08,.08,check);
        }
    }else if(escaped){
        float latitude=asin(clamp(d.y,-1.,1.)),longitude=atan(d.z,d.x);
        float grid=pow(abs(cos(latitude*24.)*cos(longitude*24.)),32.);
        vec3 background=vec3(.015,.027,.045)+grid*vec3(.12,.3,.5);
        color=shifted(background,D)*(uBeaming?pow(D,4.):1.);
    }else color=vec3(.13,.045,.008); // Missing history or exhausted integration: no invented hit.
    if(uDebugMode==3){return vec4(color,1.);}
    color=pow(vec3(1)-exp(-max(color,vec3(0))),vec3(1./2.2));return vec4(color*(1.-.2*pow(length(vUv-.5),2.)),1.);
}
`;

/** Separate material keeps the Minkowski shader and its numerical paths intact. */
export const compactStarFragmentShader=`
precision highp float;
precision highp int;
in vec2 vUv;
out vec4 fragColor;
uniform sampler2D uSegments;
uniform int uDebugMode;
uniform bool uOptical,uDoppler,uBeaming;
uniform vec2 uPickNdc;
uniform vec3 uVelocity,uForward,uRight,uUp;
uniform float uFov,uAspect,uHistoryStart;
const float INF=1e25;
vec4 tex(sampler2D s,int i){ivec2 size=textureSize(s,0);return texelFetch(s,ivec2(i%size.x,i/size.x),0);}
vec4 record(int i,int field){return tex(uSegments,i*8+field);}
float gm(vec3 v){return inversesqrt(max(1.-dot(v,v),.000001));}
vec3 boostSpace(vec3 x,float t,vec3 v){float g=gm(v);return x+(g*g/(g+1.)*dot(v,x)+g*t)*v;}
vec3 spectrum(float wavelength){return vec3(exp(-.5*pow((wavelength-610.)/35.,2.)),exp(-.5*pow((wavelength-545.)/30.,2.)),exp(-.5*pow((wavelength-455.)/25.,2.)));}
vec3 shifted(vec3 color,float factor){float D=uOptical&&uDoppler?factor:1.;return color.r*spectrum(610./D)+color.g*spectrum(545./D)+color.b*spectrum(455./D);}
${COMPACT_STAR_GLSL}
void main(){
    vec2 ndc=uDebugMode>0?uPickNdc:vUv*2.-1.;float tangent=tan(uFov*.00872664626);
    vec3 restDirection=normalize(uForward+ndc.x*tangent*uAspect*uRight+ndc.y*tangent*uUp);
    fragColor=renderCompactStar(restDirection);
}
`;
