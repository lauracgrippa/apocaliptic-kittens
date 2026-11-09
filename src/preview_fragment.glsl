#version 330 core
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;
out vec4 FragColor;
uniform vec3 uColor;
uniform vec3 uSunDirection;
uniform vec3 uCameraPosition;
uniform bool uTextured;
uniform sampler2D uTexture;
uniform float uTile;
void main() {
  vec3 n = normalize(vWorldNormal);
  vec3 l = normalize(uSunDirection);
  vec3 v = normalize(uCameraPosition - vWorldPos);
  float diff = max(dot(n, l), 0.0);
  vec3 h = normalize(l + v);
  float spec = pow(max(dot(n, h), 0.0), 30.0) * 0.10;
  vec3 tex = uTextured ? texture(uTexture, vUV * uTile).rgb : vec3(1.0);
  vec3 light = vec3(0.38,0.34,0.39) + vec3(1.0,0.85,0.69) * diff * 0.77 + vec3(spec);
  vec3 result = uColor * tex * light;
  result = pow(result, vec3(1.0/2.2));
  FragColor = vec4(result, 1.0);
}
