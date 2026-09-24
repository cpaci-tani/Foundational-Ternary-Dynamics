/** WebGL2 first-hit ray tracing. Optical rays and picking execute this same program. */
import { FRACTAL_GLSL } from './fractal-shaders.js';
export const observerVertexShader = `
precision highp float;
in vec3 position;
out vec2 vUv;
void main(){ vUv=position.xy*.5+.5; gl_Position=vec4(position.xy,0.,1.); }
`;

export const observerFragmentShader = `
precision highp float;
precision highp int;
in vec2 vUv;
out vec4 fragColor;
uniform sampler2D uSegments,uNodes,uOrder,uMeshNodes,uTriangles,uMeshes,uPanorama,uLines,uFeedback;
uniform int uNodeCount,uSegmentCount,uLineCount,uPreset,uLayers,uDebugMode,uSelected;
uniform int uFractalStyle;
uniform float uFractalDetail;
uniform bool uSR,uOptical,uDoppler,uBeaming,uShading,uHasPanorama,uCameraShell,uMirrorWorld,uHasFeedback;
uniform int uFeedbackLayers;
uniform float uFeedbackStrength,uFeedbackFade,uSeed;
uniform vec2 uResolution,uPickNdc;
uniform vec3 uCamera,uVelocity,uForward,uRight,uUp,uEnvironmentColor;
uniform float uFov,uAspect,uTime,uHistoryStart,uRadius,uDensity,uSpacing,uOrientation,uOpacity,uAnimationRate;
const float INF=1e25;
const float EPS=.00001;
vec4 tex(sampler2D s,int i){ ivec2 size=textureSize(s,0); return texelFetch(s,ivec2(i%size.x,i/size.x),0); }
vec4 record(int i,int field){return tex(uSegments,i*8+field);}
float gm(vec3 v){return inversesqrt(max(1.-dot(v,v),.000001));}
vec3 boostSpace(vec3 x,float t,vec3 v){float b=dot(v,v); return x+(b>1e-12?(gm(v)-1.)*dot(v,x)/b+gm(v)*t:0.)*v;}
bool aabb(vec3 o,vec3 d,vec3 mn,vec3 mx,float limit){
    float lo=0.,hi=limit;
    for(int j=0;j<3;j++){
        if(abs(d[j])<1e-14){if(o[j]<mn[j]||o[j]>mx[j])return false;}
        else{float a=(mn[j]-o[j])/d[j],b=(mx[j]-o[j])/d[j];lo=max(lo,min(a,b));hi=min(hi,max(a,b));if(lo>hi)return false;}
    }return hi>=0.;
}
bool triangle(vec3 o,vec3 d,vec3 a,vec3 b,vec3 c,inout float best,out vec3 normal){
    vec3 e=b-a,f=c-a,p=cross(d,f);float det=dot(e,p);if(abs(det)<1e-12)return false;
    vec3 s=o-a;float u=dot(s,p)/det;if(u<0.||u>1.)return false;
    vec3 q=cross(s,e);float v=dot(d,q)/det;if(v<0.||u+v>1.)return false;
    float t=dot(f,q)/det;if(t<=EPS||t>=best)return false;
    best=t;normal=normalize(cross(e,f));return true;
}
bool shapeHit(int shape,vec3 o,vec3 d,inout float best,out vec3 normal){
    if(shape==0||shape==12||shape==18||shape==19){
        float a=dot(d,d),b=dot(o,d),c=dot(o,o)-.25,disc=b*b-a*c;if(disc<0.)return false;
        float near=(-b-sqrt(disc))/a,far=(-b+sqrt(disc))/a,t=near>EPS?near:far;
        if(t<=EPS||t>=best)return false;best=t;normal=normalize(o+t*d);return true;
    }
    if(shape==1||shape==16||shape==17){
        float near=-INF,far=INF;vec3 nn=vec3(0),fn=vec3(0);
        for(int j=0;j<3;j++){
            if(abs(d[j])<1e-14){if(abs(o[j])>.5)return false;continue;}
            float a=(-.5-o[j])/d[j],b=(.5-o[j])/d[j];
            float lo=min(a,b),hi=max(a,b);
            if(lo>near){near=lo;nn=vec3(0);nn[j]=-sign(d[j]);}if(hi<far){far=hi;fn=vec3(0);fn[j]=sign(d[j]);}
        }
        float t=near>EPS?near:far;if(near>far||t<=EPS||t>=best)return false;
        best=t;normal=near>EPS?nn:fn;return true;
    }
    if(shape==2||shape==3){
        if(abs(d.y)<1e-14)return false;float t=-o.y/d.y;vec3 p=o+t*d;
        if(t<=EPS||t>=best||(shape==3?dot(p.xz,p.xz)>.25:max(abs(p.x),abs(p.z))>.5))return false;
        best=t;normal=vec3(0,-sign(d.y),0);return true;
    }
    vec4 info=tex(uMeshes,shape);int cursor=int(info.x),end=cursor+int(info.y);bool found=false;
    while(cursor<end){
        vec4 mn=tex(uMeshNodes,cursor*3),mx=tex(uMeshNodes,cursor*3+1),meta=tex(uMeshNodes,cursor*3+2);
        if(!aabb(o,d,mn.xyz,mx.xyz,best)){cursor=int(meta.z);continue;}
        for(int k=0;k<int(meta.y);k++){
            int tri=(int(meta.x)+k)*3;vec3 n;
            if(triangle(o,d,tex(uTriangles,tri).xyz,tex(uTriangles,tri+1).xyz,tex(uTriangles,tri+2).xyz,best,n)){normal=n;found=true;}
        }cursor++;
    }return found;
}
bool segmentHit(int index,vec3 rayOrigin,vec3 direction,float timeSlope,inout float best,out vec3 normal,out vec3 local,out float proper){
    vec4 a=record(index,0),b=record(index,1),c=record(index,2),r0=record(index,3),r1=record(index,4),r2=record(index,5),extra=record(index,7);
    vec3 velocity=uSR?b.xyz:vec3(0),relative=rayOrigin-a.xyz;
    float b2=dot(velocity,velocity),g=uSR?b.w:1.;
    vec3 origin=relative+(b2>1e-12?(g-1.)*dot(velocity,relative)/b2:0.)*velocity;
    vec3 ray=direction+(b2>1e-12?(g-1.)*dot(velocity,direction)/b2-g*timeSlope:0.)*velocity;
    mat3 invRotation=mat3(r0.xyz,r1.xyz,r2.xyz);
    vec3 o=(invRotation*origin)/c.xyz,d=(invRotation*ray)/c.xyz;
    float minimum=uOptical?max(0.,-r0.w):0.,maximum=uOptical?min(best,-max(c.w,uHistoryStart-uTime)+.000001):best;
    if(minimum>=maximum)return false;
    float distance=maximum-minimum;vec3 n;
    if(!shapeHit(int(a.w),o+minimum*d,d,distance,n))return false;
    distance+=minimum;
    float offset=distance*timeSlope;
    if(uOptical&&(offset<c.w||offset>=r0.w))return false;
    best=distance;normal=normalize(transpose(invRotation)*(n/c.xyz));local=o+distance*d;
    // The center proper clock is anchored at originTime. Surface synchronization is Einstein synchronization in the rest frame.
    proper=extra.x+(-extra.y)/g-g*dot(velocity,relative)+g*(timeSlope-dot(velocity,direction))*distance;
    return true;
}
float hash(vec3 p){p=fract(p*.1031);p+=dot(p,p.yzx+33.33);return fract((p.x+p.y)*p.z);}
vec3 spectrum(float wavelength){return vec3(exp(-.5*pow((wavelength-610.)/35.,2.)),exp(-.5*pow((wavelength-545.)/30.,2.)),exp(-.5*pow((wavelength-455.)/25.,2.)));}
vec3 shifted(vec3 color,float factor){if(!uDoppler)return color;return color.r*spectrum(610./factor)+color.g*spectrum(545./factor)+color.b*spectrum(455./factor);}
float line(float coordinate,float width){return 1.-smoothstep(width,width*2.,abs(coordinate));}
${FRACTAL_GLSL}
vec3 sky(vec3 d){
    float ca=cos(uOrientation),sa=sin(uOrientation);d.xz=mat2(ca,-sa,sa,ca)*d.xz;
    vec3 color=mix(vec3(.008,.014,.031),vec3(.025,.044,.074),max(0.,d.y));
    if(uFractalStyle>=0)return fractalEnvironment(d,uFractalStyle,uTime,uAnimationRate,uSeed,uEnvironmentColor,uDensity,uFractalDetail)*uOpacity;
    if(uHasPanorama){vec2 uv=vec2(atan(d.z,d.x)/6.2831853+.5,asin(clamp(d.y,-1.,1.))/3.14159265+.5);return texture(uPanorama,uv).rgb*.8;}
    if(uPreset>=1&&uPreset<=5){
        vec3 cell=floor(d*700.);float stars=step(.9985,hash(cell))*pow(max(0.,1.-length(fract(d*700.)-.5)),8.)*6.;
        color+=stars*vec3(.65,.83,1.);
        float wave=sin(d.x*10.+sin(d.z*8.)+uTime*uAnimationRate*.04)*sin(d.y*13.-d.z*3.);
        if(uPreset==2)color+=pow(max(0.,wave),3.)*vec3(.11,.025,.2);
        if(uPreset==3)color+=pow(max(0.,1.-abs(wave)*16.),3.)*uEnvironmentColor*.28;
        if(uPreset==4)color+=uEnvironmentColor*pow(max(0.,1.-abs(d.y+sin(d.x*4.)*.15)*8.),3.)*.4;
        if(uPreset==5)color+=uEnvironmentColor*pow(max(0.,wave),16.)*2.;
    }
    if(uCameraShell){
        vec3 p=d*uRadius;float spacing=max(.1,uSpacing);vec3 q=abs(fract(p/spacing+.5)-.5);
        float grid=1.-smoothstep(.01,.03,min(q.x,min(q.y,q.z)));
        if(uPreset==14)grid=pow(abs(sin(atan(d.x,d.z)*12.)*sin(acos(d.y)*12.)),30.);
        else if(uPreset==20)grid=pow(max(0.,cos(atan(d.x,d.y)*24.)),50.);
        else if(uPreset==21)grid=pow(max(0.,cos(atan(d.x,d.y)*6.)),50.);
        else if(uPreset==23)grid=1.-smoothstep(.01,.035,q.y);
        color+=uEnvironmentColor*grid*.32*uDensity;
    }
    return color*uOpacity;
}
vec3 ground(vec3 p,float distance){
    float spacing=max(.1,uSpacing),width=max(.006,distance/uResolution.y*.8);
    vec2 g=abs(fract(p.xz/spacing+.5)-.5)*spacing;
    float fine=1.-smoothstep(width,width*2.,min(g.x,g.y));
    vec2 big=abs(fract(p.xz/(spacing*5.)+.5)-.5)*spacing*5.;
    float major=1.-smoothstep(width,width*2.,min(big.x,big.y));
    vec3 color=vec3(.014,.024,.037);
    if((uLayers&1)!=0)color+=vec3(.055,.19,.25)*fine+vec3(.1,.29,.37)*major*.35;
    if((uLayers&2)!=0){float ring=abs(fract(length(p.xz)/spacing+.5)-.5)*spacing;float spokes=abs(sin(atan(p.z,p.x)*12.))*max(1.,length(p.xz));color+=uEnvironmentColor*(line(ring,width)+line(spokes,width))*.25;}
    if((uLayers&8192)!=0){float distanceFromObserver=length(p.xz-uCamera.xz);float ring=abs(fract(distanceFromObserver/spacing+.5)-.5)*spacing;color+=vec3(.65,.44,.95)*line(ring,width)*.4;}
    if((uLayers&4)!=0){color+=vec3(.8,.13,.15)*line(p.z,width*2.);color+=vec3(.1,.5,1.)*line(p.x,width*2.);}
    if((uLayers&8)!=0)color+=uEnvironmentColor*step(length(g),width*4.)*.8;
    return color;
}
// Bounded current-frame camera images, confined to distant presentation pixels.
// Tone-mapped texture samples are approximately inverted before final scene mapping.
vec3 cameraBillboards(vec3 color){
    if(!uHasFeedback)return color;
    for(int layer=0;layer<3;layer++){
        if(layer>=uFeedbackLayers)break;
        float f=float(layer),angle=(f-1.)*.045,c=cos(angle),s=sin(angle);
        vec2 p=mat2(c,-s,s,c)*(vUv-.5)*(1.05+f*.20)+vec2((f-1.)*.035,0.)+.5;
        float edge=smoothstep(0.,.08,min(min(p.x,1.-p.x),min(p.y,1.-p.y)));
        vec3 sampleColor=texture(uFeedback,clamp(p,vec2(0),vec2(1))).rgb;
        vec3 linearColor=-log(max(vec3(.001),vec3(1)-pow(sampleColor,vec3(2.2))));
        float weight=uFeedbackStrength*uFeedbackFade*edge/(1.+f*.8);
        color=mix(color,linearColor,clamp(weight,0.,.8));
    }
    return color;
}
void main(){
    vec2 ndc=uDebugMode>0?uPickNdc:vUv*2.-1.;float tangent=tan(uFov*.00872664626);
    vec3 restDirection=normalize(uForward+ndc.x*tangent*uAspect*uRight+ndc.y*tangent*uUp),direction=restDirection;float timeSlope=0.;
    if(uSR){float t=uOptical?-1.:0.,transformedTime=gm(uVelocity)*(t+dot(uVelocity,restDirection));direction=boostSpace(restDirection,t,uVelocity);if(uOptical){direction/=-transformedTime;timeSlope=-1.;}else timeSlope=transformedTime;}
    float best=INF,proper=0.;int picked=-1;vec3 normal=vec3(0),local=vec3(0);bool pickedMirror=false;
    for(int pass=0;pass<2;pass++){
      if(pass==1&&!uMirrorWorld)break;
      vec3 rayOrigin=pass==1?vec3(0.,-2.*uCamera.y,0.):vec3(0),tracingDirection=direction;
      if(pass==1)tracingDirection.y=-tracingDirection.y;
      int cursor=0;
      while(cursor<uNodeCount){
        vec4 mn=tex(uNodes,cursor*3),mx=tex(uNodes,cursor*3+1),meta=tex(uNodes,cursor*3+2);
        if(uOptical&&!aabb(rayOrigin,tracingDirection,mn.xyz,mx.xyz,best)){cursor=int(meta.z);continue;}
        for(int k=0;k<int(meta.y);k++){
            int index=int(tex(uOrder,int(meta.x)+k).x);vec3 n,l;float clock;
            if(segmentHit(index,rayOrigin,tracingDirection,timeSlope,best,n,l,clock)){picked=index;normal=n;local=l;proper=clock;pickedMirror=pass==1;if(pickedMirror)normal.y=-normal.y;}
        }cursor++;
      }
    }
    float groundDistance=abs(direction.y)>.000001?-uCamera.y/direction.y:INF;
    bool groundBlocks=!uMirrorWorld&&direction.y<0.&&groundDistance>0.&&groundDistance<best;
    if(uDebugMode>0){fragColor=picked<0||groundBlocks?vec4(0):vec4(float(picked+1)*(pickedMirror?-1.:1.),best,uTime+best*timeSlope,proper);return;}
    vec3 skyDirection=normalize(direction);if(uMirrorWorld)skyDirection.y=abs(skyDirection.y);
    vec3 color=picked<0?cameraBillboards(sky(skyDirection)):vec3(0);
    if(groundBlocks){color=ground(uCamera+direction*groundDistance,groundDistance);picked=-1;best=groundDistance;}
    if(picked<0&&uOptical){float environmentFactor=gm(uVelocity)*(1.+dot(uVelocity,normalize(direction)));color=shifted(color,environmentFactor)*(uBeaming?pow(environmentFactor,4.):1.);}
    if(picked>=0){
        vec4 material=record(picked,6);vec3 velocity=record(picked,1).xyz;
        if(pickedMirror)velocity.y=-velocity.y;
        float factor=uOptical?gm(uVelocity)*(1.+dot(uVelocity,normalize(direction)))/(gm(velocity)*(1.+dot(velocity,normalize(direction)))):1.;
        color=shifted(material.rgb,factor)*material.a*(uBeaming?pow(factor,4.):1.);
        if(record(picked,7).z==2.&&record(picked,7).w!=0.)color*=.8+.2*sin((uTime+best*timeSlope)*record(picked,7).w+float(picked)*.13);
        if(uShading){float face=.35+.65*abs(dot(normal,normalize(vec3(.4,.8,.6))));color*=face;}
        int shape=int(record(picked,0).w);
        if((uLayers&32)!=0&&(shape==1||shape==16||shape==17)){vec3 q=abs(abs(local)-.5);float edge=1.-smoothstep(.008,.025,min(max(q.x,q.y),min(max(q.x,q.z),max(q.y,q.z))));color+=vec3(.2,.5,.6)*edge;}
        if(picked==uSelected)color+=vec3(.07,.13,.13);
        if(int(record(picked,0).w)==16){float angle=proper*6.2831853;float face=step(.4,abs(local.z));vec2 dial=local.xy;float rim=line(length(dial)-.32,.012);vec2 hand=vec2(sin(angle),cos(angle));float projection=clamp(dot(dial,hand),0.,.27);float tick=line(length(dial-hand*projection),.012);color+=vec3(1)*face*(rim+tick);}
    }
    // With the mirror active the plane becomes a passable reference grid, not an opaque occluder.
    if(uMirrorWorld&&groundDistance>0.&&groundDistance<best){vec3 grid=ground(uCamera+direction*groundDistance,groundDistance)-vec3(.014,.024,.037);color+=max(grid,vec3(0))*.65;}
    // Explicit presentation overlays: already projected into the same observer's image, never geometry occluders.
    for(int i=0;i<uLineCount;i++){
        vec4 ends=tex(uLines,i*2),style=tex(uLines,i*2+1);vec2 a=ends.xy*uResolution,b=ends.zw*uResolution,p=vUv*uResolution;
        vec2 delta=b-a;float fraction=clamp(dot(p-a,delta)/max(dot(delta,delta),.001),0.,1.);float distance=length(p-a-fraction*delta);
        color+=style.rgb*(1.-smoothstep(style.a,style.a+1.,distance));
    }
    color=vec3(1)-exp(-max(color,vec3(0)));color=pow(color,vec3(1./2.2));
    float vignette=1.-.2*pow(length(vUv-.5),2.);fragColor=vec4(color*vignette,1.);
}
`;
