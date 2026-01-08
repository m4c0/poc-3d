/******************************************************************************/
// This draws a rotating cube with open edges.
//
// Faces are coloured based on their vertex model positions. This visually
// represents each axis (X, Y, Z) as a gradient from black to a primary colour
// (R, G, B). Therefore we can get a visual feedback of the axis alignments.
//
// This project runs in both Vulkan and WASM, mostly to use WASM as a "control
// group". Vulkan has way too many options that impacts the output. Making
// Vulkan align with WebGL made it easier to reason and compare.
//
// The main "trick" is how to convert any input (3D vertices, in this case) to
// NDC space. Then from NDC to screen space.
//
// Both NDC and screen spaces are different between Vulkan and WebGL.
//
// Vulkan's NDC ranges from (-1,-1,0) to (1,1,1) whilst WebGL ranges from
// (-1,-1,-1) to (1,1,1). The Z range only affects clipping.
//
// The "Y" coordinate is flipped between Vulkan and WebGL's screen coordinates.
// Vulkan supports flipping that by using negative viewport size.
//
// Finally, the whole "perspective matrix" calculation is simple. Matrices like
// those used in gluPerspective apply these semi-independent rules:
// - Scaling (x, y) by the field-of-view (calculated with the tangent of the
//   half of the FOV angle). Which can be avoided if FOV is 90 degrees (since
//   tan(90/2) = 1)
// - Scaling X by screen aspect. Which would not be needed for a square screen
//   space.
// - Dividing (x, y) by z for the perspective effect
// - Mapping Z to NDC's Z (i.e. given Z in [near;far] range, transform it to
//   [0;1] or [-1;1] - the math is simpler than it looks)
//
// This example does some minimal math to keep a right-handed coordinate system
// with Y pointing up and Z outwards camera. Changing this is as simple as
// flipping individual axis.
/******************************************************************************/
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
