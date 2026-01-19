#version 450

layout(push_constant) uniform upc {
  vec4 colour;
  float aspect;
  float fov_deg;
  float time;
} pc;

layout(location = 0) in  vec4 pos;

const float near =  0.01;
const float far  = 10.0;

void main() {
  float f = 1.0 / tan(radians(pc.fov_deg) / 2.0);

  float a = pc.time;
  float b = sin(pc.time / 3.14);
  mat4 rot = mat4(
    cos(a), 0, sin(a), 0,
    0, 1, 0, 0,
    -sin(a), 0, cos(a), 0,
    0, 0, 0, 1
  ) * mat4(
    1, 0, 0, 0,
    0,  cos(b), sin(b), 0,
    0, -sin(b), cos(b), 0,
    0, 0, 0, 1
  );

  vec4 p = pos.w == 0
    ? vec4(0, -1, 0, 0) // Point at infinity, oriented to the light
    : rot * pos + vec4(0, 0, 3, 0);
  p.x *= -1; // Left-hand to right-hand
  gl_Position = mat4(
    f / pc.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -(far * near) / (far - near), 0
  ) * p;
}
