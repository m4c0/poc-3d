#version 450

layout(constant_id = 99) const uint txt_count = 8;
layout(set = 0, binding = 0) uniform sampler2D textures[txt_count];

layout(location = 0) out vec4 colour;

layout(location = 0) in vec2 f_uv;
layout(location = 1) in vec4 f_colour;

void main() {
  colour = f_colour * texture(textures[0], f_uv);
}
