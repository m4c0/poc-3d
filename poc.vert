#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 f_uv;

void main() {
  gl_Position = vec4(pos, 1);
  f_uv = uv;
}
