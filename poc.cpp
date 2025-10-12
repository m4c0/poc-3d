#pragma leco app

import casein;
import hai;
import vinyl;
import voo;

struct app_stuff {
  voo::device_and_queue dq { "poc-3d", casein::native_ptr };
  vee::render_pass rp = voo::single_att_render_pass(dq);
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

