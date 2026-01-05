#pragma leco app
#pragma leco add_shader "poc-axis.frag"
#pragma leco add_shader "poc-axis.vert"

import clay;
import dotz;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct vtx {
  dotz::vec4 pos;
};

struct app_stuff : vinyl::base_app_stuff {
  clay::buffer<vtx> vbuf { 6 };

  vee::render_pass rp = voo::single_att_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout();
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .back_face_cull = false,
    .shaders {
      *clay::vert_shader("poc-axis", [] {}),
      *clay::frag_shader("poc-axis", [] {}),
    },
    .bindings { vbuf.vertex_input_bind() },
    .attributes {
      clay::buffer<vtx>::vertex_attribute(&vtx::pos),
    },
  });

  app_stuff() : base_app_stuff { "poc-3d" } {
    auto m = vbuf.map();
    m += vtx { .pos { -0.5, -0.5, 0.5, 1.0 } };
    m += vtx { .pos {  0.5, -0.5, 0.5, 1.0 } };
    m += vtx { .pos { -0.5,  0.5, 0.5, 1.0 } };

    m += vtx { .pos {  0.5,  0.5, 0.5, 1.0 } };
    m += vtx { .pos {  0.5, -0.5, 0.5, 1.0 } };
    m += vtx { .pos { -0.5,  0.5, 0.5, 1.0 } };
  }
};
struct ext_stuff : vinyl::base_extent_stuff {
  ext_stuff() : base_extent_stuff { vv::as() } {}
};

extern "C" void casein_init() {
  vv::setup([] {
    vv::ss()->frame([] {
      auto rp = vv::ss()->clear({ 0, 0, 0, 1 });

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vv::as()->vbuf.bind(cb);
      vee::cmd_draw(cb, vv::as()->vbuf.count());
    });
  });
}
