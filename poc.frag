#version 450

layout(set = 0, binding = 0) uniform sampler2D texcolour;
layout(set = 1, binding = 0) uniform sampler2D normal;

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_uv;
layout(location = 1) in vec4 f_colour;
layout(location = 2) in vec3 f_normal;

void main() {
  colour = f_colour * texture(texcolour, f_uv);
}
