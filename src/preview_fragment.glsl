#version 330 core
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;
in vec3 vObjectPos;
in vec3 vObjectNormal;
out vec4 FragColor;
uniform vec3 uColor;
uniform vec3 uSunDirection;
uniform vec3 uCameraPosition;
uniform bool uTextured;
uniform sampler2D uTexture;
uniform float uTile;
// Caixas texturizadas: uTile e repeticoes por metro e o UV vem do tamanho da face.
uniform bool uBoxUV;
uniform vec3 uObjectScale;

vec2 boxUV() {
    // Projecao planar pela face: coordenadas do objeto escaladas pelo tamanho real
    // da caixa, para a imagem manter a proporcao em qualquer face.
    vec3 p = vObjectPos * uObjectScale;
    vec3 a = abs(vObjectNormal);
    if (a.y > .6) return p.xz;
    if (a.z > .6) return p.xy;
    return p.zy;
}

void main(){
    vec3 n=normalize(vWorldNormal);
    vec3 l=normalize(uSunDirection);
    vec3 v=normalize(uCameraPosition-vWorldPos);
    float blockyNoise=fract(sin(dot(floor(vWorldPos.xz*7.0+vWorldPos.yy*2.0),vec2(12.9898,78.233)))*43758.5453);
    // Procedural texture for non-image materials: chunky tonal mottling.
    vec3 procedural=vec3(0.87+0.13*blockyNoise);
    vec2 uv=(uBoxUV?boxUV():vUV)*uTile;
    vec3 tex=uTextured?texture(uTexture,uv).rgb:procedural;

    // Blinn-Phong: ambiente + difusa (Lambert) + especular com o half-vector.
    vec3 ambient=vec3(.23,.26,.32);
    float diffuse=max(dot(n,l),0.0);
    vec3 h=normalize(l+v);
    float specular=diffuse>0.0?pow(max(dot(n,h),0.0),24.0):0.0;
    vec3 light=ambient+vec3(.58,.58,.50)*diffuse;
    // Local emergency lamp pools, giving the concrete/brick passage a sci-fi look.
    float redLamp=(.5+.5*cos(vWorldPos.z*1.22))*exp(-abs(abs(vWorldPos.x)-8.6)*.40);
    light+=vec3(.29,.025,.018)*redLamp;
    vec3 result=uColor*tex*light+vec3(.30,.29,.25)*specular;

    float d=length(vWorldPos-uCameraPosition);
    float fog=clamp((d-10.0)/40.0,0.0,.83);
    result=mix(result,vec3(.080,.098,.123),fog);
    result=floor(result*26.0)/26.0;
    FragColor=vec4(pow(max(result,vec3(0.0)),vec3(1.0/2.2)),1.0);
}
