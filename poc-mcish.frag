#version 450

layout(location = 0) in  vec2 f_uv;
layout(location = 0) out vec4 colour;

void main() {
  colour = vec4(f_uv, 0, 1);
}
