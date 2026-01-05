#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov;
};

layout(location = 0) in  vec4 pos;
layout(location = 0) out vec4 f_pos;

const float near =  0.01;
const float far  = 10.0;

void main() {
  mat4 proj = mat4(
    1 / (aspect * tan(fov / 2)), 0, 0, 0,
    0, 1 / tan(fov / 2), 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -far * near / (far - near), 0
  );

  gl_Position = proj * pos;
  f_pos = pos;
}
