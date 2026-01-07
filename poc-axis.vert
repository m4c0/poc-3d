#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov_deg;
} pc;

layout(location = 0) in  vec4 pos;
layout(location = 0) out vec4 f_pos;

const float near =  0.01;
const float far  = 10.0;

void main() {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  vec3 p = pos.xyz + vec3(0, 0, 3);

  gl_Position = vec4(
    p.x * f / pc.aspect,
    p.y * f,
    far * (p.z - near) / (far - near),
    p.z
  );
  f_pos = pos;
}
