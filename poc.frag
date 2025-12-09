#version 450

layout(set = 0, binding = 0) uniform sampler2D texcolour;
layout(set = 1, binding = 0) uniform sampler2D normal;

layout(std140, set = 2, binding = 0) uniform uni {
  mat4 view;
};

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_uv;
layout(location = 1) in vec4 f_colour;
layout(location = 2) in float f_diffuse;

void main() {
  vec4 c = f_colour * texture(texcolour, f_uv);
  colour = vec4(c.rgb * f_diffuse, c.a);
}
