#version 450

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_uv;
layout(location = 1) in vec4 f_colour;

void main() {
  colour = f_colour;
}
