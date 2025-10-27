#version 450

layout(push_constant) uniform upc {
  float aspect;
  float fov;
  float far;
  float near;

  vec4 colour;
  vec3 cam_pos;
  vec3 cam_rot;

  float time;
};

layout(std140, set = 2, binding = 0) uniform uni {
  mat4 view;
};

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 1, component = 3) in float has_normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 f_uv;
layout(location = 1) out vec4 f_colour;
layout(location = 2) out vec3 f_normal;

void main() {
  float asp = 1.0 / aspect;
  mat4 proj = mat4(
    asp / tan(fov / 2), 0, 0, 0,
    0, 1 / tan(fov / 2), 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -far * near / (far - near), 0
  );
  mat4 cam = mat4(
    1, 0, 0, 0,
    0, cos(cam_rot.x), -sin(cam_rot.x), 0,
    0, sin(cam_rot.x), cos(cam_rot.x), 0,
    0, 0, 0, 1
  ) * mat4(
    cos(cam_rot.y), 0, -sin(cam_rot.y), 0,
    0, 1, 0, 0,
    sin(cam_rot.y), 0, cos(cam_rot.y), 0,
    0, 0, 0, 1
  ) * mat4(
    cos(cam_rot.z), -sin(cam_rot.z), 0, 0,
    sin(cam_rot.z), cos(cam_rot.z), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  ) * mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    -cam_pos.x, cam_pos.yz, 1
  );

  gl_Position = proj * cam * view * vec4(pos.x, -pos.yz, 1);
  f_uv = uv;
  f_colour = colour;
  f_normal = normal;
}
