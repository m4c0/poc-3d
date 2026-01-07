#pragma leco app
#pragma leco add_shader "poc-axis.frag"
#pragma leco add_shader "poc-axis.vert"

import clay;
import dotz;
import sitime;
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
  float time;
};
struct vtx {
  dotz::vec4 pos;
};

struct app_stuff : vinyl::base_app_stuff {
  clay::buffer<vtx> vbuf { 36 };

#ifdef LECO_TARGET_WASM
  clay::program prog { "poc-axis" };
#else
  struct sconst {
    float ndc_mult = 1;
    float ndc_add  = 0;
  };
  vee::render_pass rp = voo::single_att_depth_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .depth = vee::depth::op_less(),
    .shaders {
      clay::vert_shader("poc-axis", [] {}).pipeline_stage("main", vee::specialisation_info<sconst>({
        vee::specialisation_map_entry(0, &sconst::ndc_mult),
        vee::specialisation_map_entry(1, &sconst::ndc_add),
      })),
      *clay::frag_shader("poc-axis", [] {}),
    },
    .bindings { vbuf.vertex_input_bind() },
    .attributes {
      clay::buffer<vtx>::vertex_attribute(&vtx::pos),
    },
  });
#endif

  app_stuff() : base_app_stuff { "poc-3d" } {
#ifdef LECO_TARGET_WASM
    vbuf.bind();
    clay::buffer<vtx>::vertex_attribute(&vtx::pos, false)(0);

    using namespace gelo;
    enable(DEPTH_TEST);
    enable(CULL_FACE);
#endif

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
        .time = timer.secs(),
      };

#ifdef LECO_TARGET_WASM
      using namespace gelo;

      if (!vv::as()->prog) return;

      static unsigned u_aspect = get_uniform_location(vv::as()->prog.id(), "pc.aspect");
      static unsigned u_fov    = get_uniform_location(vv::as()->prog.id(), "pc.fov_deg");
      static unsigned u_time   = get_uniform_location(vv::as()->prog.id(), "pc.time");
      silog::assert(u_aspect && u_fov, "missing uniforms");

      uniform1f(u_aspect, pc.aspect);
      uniform1f(u_fov,    pc.fov);
      uniform1f(u_time,   pc.time);
      clear_depth(1);
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
