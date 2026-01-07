#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov_deg;
  float time;
} pc;

layout(location = 0) in  vec4 pos;
layout(location = 0) out vec4 f_pos;

const float near =  0.01;
const float far  = 10.0;

void main() {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  float a = pc.time;
  float b = sin(pc.time / 3.14);
  mat3 rot = mat3(
    cos(a), 0, sin(a),
    0, 1, 0,
    -sin(a), 0, cos(a)
  ) * mat3(
    1, 0, 0,
    0,  cos(b), sin(b),
    0, -sin(b), cos(b)
  );

  vec3 p = rot * pos.xyz + vec3(0, 0, -3);

  p.z *= -1; // Left-hand to right-hand
  gl_Position = vec4(
    p.x * f / pc.aspect,
    p.y * f,
    far * (p.z - near) / (far - near),
    p.z
  );
  f_pos = pos;
}
