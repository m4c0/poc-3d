#version 450

layout(location = 0) in  vec2 f_uv;
layout(location = 0) out vec4 colour;

void main() {
  vec2 p = f_uv * vec2(2, 4);
  p.x += floor(p.y) * 0.25;
  p = fract(p);

  colour = vec4(p, 0, 1);
}
