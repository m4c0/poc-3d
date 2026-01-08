#pragma leco app
#pragma leco add_shader "poc-mcish.frag"
#pragma leco add_shader "poc-mcish.vert"

import clay;
import dotz;
import traits;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  float aspect;
  float fov = 90;
};

namespace cube {
  struct vtx {
    dotz::vec3 pos;
    float _pad;
    dotz::vec2 uv;
  };

  struct buffer : public clay::buffer<vtx> {
    buffer() : clay::buffer<vtx> { 36 } {
      auto m = map();

      // Front
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 1, 1 } };

      // Back
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };

      // Bottom
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 1 } };

      // Top
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };

      // Left
      m += vtx { .pos { -0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos { -0.5, -0.5,  0.5 }, .uv { 1, 1 } };

      // Right
      m += vtx { .pos {  0.5, -0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5, -0.5,  0.5 }, .uv { 1, 1 } };
      m += vtx { .pos {  0.5,  0.5, -0.5 }, .uv { 1, 1 } };
    }
  };
}

struct app_stuff : vinyl::base_app_stuff {
  cube::buffer cube {};

  vee::render_pass rp = voo::single_att_depth_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .depth = vee::depth::op_less(),
    .shaders {
      *clay::vert_shader("poc-mcish", [] {}),
      *clay::frag_shader("poc-mcish", [] {}),
    },
    .bindings { cube::buffer::vertex_input_bind() },
    .attributes { 
      vee::vertex_attribute_vec3(0, traits::offset_of(&cube::vtx::pos)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&cube::vtx::uv)),
    },
  });

  app_stuff() : base_app_stuff { "poc-mcish" } {}
};
struct ext_stuff : vinyl::base_extent_stuff {
  ext_stuff() : base_extent_stuff { vv::as() } {}
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->frame([] {
      [[maybe_unused]] auto rp = vv::ss()->clear({ 0, 0, 0, 1 });

      upc pc {
        .aspect = vv::ss()->aspect(),
      };

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &pc);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->cube, 0);
      vee::cmd_draw(cb, vv::as()->cube.count());
    });
  });
}
