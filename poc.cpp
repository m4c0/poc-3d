#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import hai;
import vinyl;
import voo;

struct vtx {
  dotz::vec3 p;
};

struct app_stuff {
  voo::device_and_queue dq { "poc-3d", casein::native_ptr };
  vee::render_pass rp = voo::single_att_render_pass(dq);
  vee::pipeline_layout pl = vee::create_pipeline_layout();
  vee::gr_pipeline gp = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .shaders {
      voo::shader("poc.vert.spv").pipeline_vert_stage(),
      voo::shader("poc.frag.spv").pipeline_frag_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(vtx)),
    },
    .attributes {
      vee::vertex_attribute_vec3(0, 0),
    },
  });

};
static hai::uptr<app_stuff> gas {};

struct sized_stuff {
  voo::swapchain_and_stuff sw { gas->dq, *gas->rp };
};
static hai::uptr<sized_stuff> gss {};

static void frame() {
  if (!gss) gss.reset(new sized_stuff {});

  gss->sw.acquire_next_image();
  gss->sw.queue_one_time_submit(gas->dq.queue(), [&] {
    gss->sw.cmd_render_pass({
      .command_buffer = gss->sw.command_buffer(),
      .clear_colours { vee::clear_colour(0.1, 0.2, 0.3, 1.0) },
    });

    auto cb = gss->sw.command_buffer();
    vee::cmd_set_viewport(cb, gss->sw.extent());
    vee::cmd_set_scissor(cb, gss->sw.extent());
    vee::cmd_bind_gr_pipeline(cb, *gas->gp);
    // vee::cmd_bind_vertex_buffers(cb, 1, u.data().local_buffer());
    // vee::cmd_draw(cb, quad::v_count, inst, first_inst);
  });
  gss->sw.queue_present(gas->dq.queue());
}

struct app_init {
  app_init() {
    using namespace vinyl;
    on(START,  [] { gas.reset(new app_stuff {}); });
    on(RESIZE, [] { gss.reset(nullptr); });
    on(FRAME,  &frame);
    on(STOP,   [] { 
      gss.reset(nullptr);
      gas.reset(nullptr);
    });
  }
} i;

//

