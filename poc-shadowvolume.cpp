//---------------------------------------------------------------------
// Trying to implement shadow volumes
//
// Refs:
// * https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-9-efficient-shadow-volume-rendering
//
// --------------------------------------------------------------------
#pragma leco app
#pragma leco add_shader "poc-shadowvolume.frag"
#pragma leco add_shader "poc-shadowvolume.vert"

import dotz;
import sitime;
import traits;
import vinyl;
import voo;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  dotz::vec4 colour;
  float aspect;
  float fov = 90;
  float time;
};
struct vtx {
  dotz::vec4 pos;
};

struct app_stuff : vinyl::base_app_stuff {
  voo::bound_buffer vbuf = voo::bound_buffer::create_from_host(1024 * sizeof(vtx), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

  vee::render_pass rp = voo::single_att_depth_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vert_frag_push_constant_range<upc>());
  vee::gr_pipeline ppl = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .back_face_cull = false,
    .depth = vee::depth::op_less(),
    .shaders {
      *voo::vert_shader("poc-shadowvolume.vert.spv"),
      *voo::frag_shader("poc-shadowvolume.frag.spv"),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(vtx)),
    },
    .attributes {
      vee::vertex_attribute_vec4(0, traits::offset_of(&vtx::pos)),
    },
  });

  app_stuff() : base_app_stuff { "poc-3d" } {
    auto m = voo::memiter<vtx> { *vbuf.memory };

    dotz::vec4 light {};

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

    // Top
    m += vtx { .pos { -0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos = light };
    m += vtx { .pos {  0.9,  1.0, -0.9, 1.0 } };

    m += vtx { .pos { -0.9, -1.0, -0.9, 1.0 } };
    m += vtx { .pos = light };
    m += vtx { .pos {  0.9, -1.0, -0.9, 1.0 } };
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

      auto cb = vv::ss()->sw.command_buffer();
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->vbuf.buffer);

      // bottom
      pc.colour = { 0, 1, 0, 1 };
      vee::cmd_push_vert_frag_constants(cb, *vv::as()->pl, &pc);
      vee::cmd_draw(cb, { .vcount = 6, .first_v = 0 });

      // top
      pc.colour = { 0, 0, 1, 1 };
      vee::cmd_push_vert_frag_constants(cb, *vv::as()->pl, &pc);
      vee::cmd_draw(cb, { .vcount = 6, .first_v = 6 });

      // shadow edge
      pc.colour = { 1, 0, 0, 0.3 };
      vee::cmd_push_vert_frag_constants(cb, *vv::as()->pl, &pc);
      vee::cmd_draw(cb, { .vcount = 6, .first_v = 12 });
    });
  });
}
