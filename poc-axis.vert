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
  mat4 proj = mat4(
    f / pc.aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -far * near / (far - near), 0
  );

  mat4 model = mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 3, 1
  );

  gl_Position = proj * model * pos;
  f_pos = pos;
}
