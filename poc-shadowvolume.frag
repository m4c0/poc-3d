#version 450

layout(push_constant) uniform upc {
  vec4 colour;
  float aspect;
  float fov_deg;
  float time;
} pc;

layout(location = 0) out vec4 colour;

void main() {
  colour = pc.colour;
}
