#version 450

layout(push_constant) uniform upc {
  float aspect;
};

layout(location = 0) in  vec4 pos;
layout(location = 0) out vec4 f_pos;

void main() {
  gl_Position = pos * vec4(1/aspect, 1, 1, 1);
  f_pos = pos;
}
