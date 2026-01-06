#pragma leco app
#pragma leco add_shader "poc-axis.frag"
#pragma leco add_shader "poc-axis.vert"

import clay;
import dotz;
import vinyl;

#ifdef LECO_TARGET_WASM
import gelo;
import silog;
#else
import voo;
#endif

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
  clay::buffer<vtx> vbuf { 6 };

#ifdef LECO_TARGET_WASM
  clay::program prog { "poc-axis" };
#else
  vee::render_pass rp = voo::single_att_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .shaders {
      *clay::vert_shader("poc-axis", [] {}),
      *clay::frag_shader("poc-axis", [] {}),
    },
    .bindings { vbuf.vertex_input_bind() },
    .attributes {
      clay::buffer<vtx>::vertex_attribute(&vtx::pos),
    },
  });
#endif

  app_stuff() : base_app_stuff { "poc-3d" } {
    auto m = vbuf.map();

    m += vtx { .pos { -0.9, -0.9, 0.9, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, 0.9, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, 0.9, 1.0 } };

    m += vtx { .pos {  0.9,  0.9, 0.9, 1.0 } };
    m += vtx { .pos { -0.9,  0.9, 0.9, 1.0 } };
    m += vtx { .pos {  0.9, -0.9, 0.9, 1.0 } };
  }
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

#ifdef LECO_TARGET_WASM
      if (!vv::as()->prog) return;

      using namespace gelo;
      draw_arrays(TRIANGLES, 0, vv::as()->vbuf.count());
#else
      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_push_vertex_constants(cb, *vv::as()->pl, &pc);
      vv::as()->vbuf.bind(cb);
      vee::cmd_draw(cb, vv::as()->vbuf.count());
#endif
    });
  });
}
