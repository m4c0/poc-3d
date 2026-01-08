#version 450

layout(location = 0) in  vec4 f_pos;
layout(location = 0) out vec4 colour;

void main() {
  colour = vec4(f_pos.rgb * 0.5 + 0.5, 1);
}
