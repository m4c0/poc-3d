#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 f_uv;

void main() {
  float aspect = 720.0 / 1280.0; 
  float fov = radians(120);
  float far = 1.00;
  float near = 0.01;

  vec3 cam_pos = vec3(0, 0, 0);
  vec3 cam_rot = vec3(radians(30), 0, 0);

  mat4 proj = mat4(
    aspect / tan(fov / 2), 0, 0, 0,
    0, 1 / tan(fov / 2), 0, 0,
    0, 0, far / (far - near), 1,
    0, 0, -far * near / (far - near), 0
  );
  mat4 cam = mat4(
    1, 0, 0, 0,
    0, cos(cam_rot.x), -sin(cam_rot.x), 0,
    0, sin(cam_rot.x), cos(cam_rot.x), 0,
    0, 0, 0, 1
  );

  gl_Position = proj * cam * vec4(pos.x, -pos.yz, 1);
  f_uv = uv;
}
