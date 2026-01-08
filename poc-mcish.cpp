#pragma leco app
#pragma leco add_shader "poc-mcish.frag"
#pragma leco add_shader "poc-mcish.vert"

import clay;
import dotz;
import sitime;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  float aspect;
  float fov = 90;
};
struct vtx {
  dotz::vec4 pos;
};

struct app_stuff : vinyl::base_app_stuff {
  clay::buffer<vtx> vbuf { 36 };

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
    .bindings { vbuf.vertex_input_bind() },
    .attributes {
      clay::buffer<vtx>::vertex_attribute(&vtx::pos),
    },
  });

  app_stuff() : base_app_stuff { "poc-mcish" } {
    auto m = vbuf.map();

    // Front
    m += vtx { .pos { -0.9, -0.9, 1.0, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, 1.0, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, 1.0, 1.0 } };
    m += vtx { .pos {  0.9,  0.9, 1.0, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, 1.0, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, 1.0, 1.0 } };

    // Back
    m += vtx { .pos { -0.9, -0.9, -1.0, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, -1.0, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, -1.0, 1.0 } };
    m += vtx { .pos {  0.9,  0.9, -1.0, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, -1.0, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, -1.0, 1.0 } };

    // Bottom
    m += vtx { .pos { -0.9, -1.0, -0.9, 1.0 } };
    m += vtx { .pos {  0.9, -1.0, -0.9, 1.0 } };
    m += vtx { .pos { -0.9, -1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9, -1.0,  0.9, 1.0 } };
    m += vtx { .pos { -0.9, -1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9, -1.0, -0.9, 1.0 } };

    // Top
    m += vtx { .pos { -0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos { -0.9,  1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos {  0.9,  1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos { -0.9,  1.0,  0.9, 1.0 } };
 
    // Left
    m += vtx { .pos { -1.0, -0.9, -0.9, 1.0 } };
    m += vtx { .pos { -1.0, -0.9,  0.9, 1.0 } };
    m += vtx { .pos { -1.0,  0.9, -0.9, 1.0 } };
    m += vtx { .pos { -1.0,  0.9,  0.9, 1.0 } };
    m += vtx { .pos { -1.0,  0.9, -0.9, 1.0 } };
    m += vtx { .pos { -1.0, -0.9,  0.9, 1.0 } };
 
    // Right
    m += vtx { .pos {  1.0, -0.9, -0.9, 1.0 } };
    m += vtx { .pos {  1.0,  0.9, -0.9, 1.0 } };
    m += vtx { .pos {  1.0, -0.9,  0.9, 1.0 } };
    m += vtx { .pos {  1.0,  0.9,  0.9, 1.0 } };
    m += vtx { .pos {  1.0, -0.9,  0.9, 1.0 } };
    m += vtx { .pos {  1.0,  0.9, -0.9, 1.0 } };
  }
};
struct ext_stuff : vinyl::base_extent_stuff {
  ext_stuff() : base_extent_stuff { vv::as() } {}
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->frame([] {
      static sitime::stopwatch timer {};

      [[maybe_unused]] auto rp = vv::ss()->clear({ 0, 0, 0, 1 });

      upc pc {
        .aspect = vv::ss()->aspect(),
      };

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &pc);
      vv::as()->vbuf.bind(cb);
      vee::cmd_draw(cb, vv::as()->vbuf.count());
    });
  });
}
