#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import glub;
import hai;
import jojo;
import sitime;
import traits;
import vinyl;
import voo;

import print;

struct upc {
  float aspect;
  float fov = dotz::radians(80);
  float far = 10.0;
  float near = 0.01;

  dotz::vec4 cam_pos { 0, 0, 3, 0 };
  dotz::vec4 cam_rot { 0, 0, 0, 0 };
  float time;
} g_pc;

static bool g_key_w = false;
static bool g_key_s = false;
static bool g_key_a = false;
static bool g_key_d = false;

struct app_stuff {
  voo::device_and_queue dq { "poc-3d", casein::native_ptr };
  vee::render_pass rp = vee::create_render_pass({
    .attachments {{
      vee::create_colour_attachment(dq.physical_device(), dq.surface()),
      vee::create_depth_attachment(),
    }},
    .subpasses {{
      vee::create_subpass({
        .colours {{ vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal) }},
        .depth_stencil { vee::create_attachment_ref(1, vee::image_layout_depth_stencil_attachment_optimal) },
      }),
    }},
    .dependencies {{
      vee::create_colour_dependency(),
      vee::create_depth_dependency(),
    }},
  });
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vertex_push_constant_range<upc>());
  vee::gr_pipeline gp = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .depth = vee::depth::op_less(),
    .shaders {
      voo::shader("poc.vert.spv").pipeline_vert_stage(),
      voo::shader("poc.frag.spv").pipeline_frag_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(dotz::vec3)),
      vee::vertex_input_bind(sizeof(dotz::vec3)),
      vee::vertex_input_bind(sizeof(dotz::vec2)),
    },
    .attributes {
      vee::vertex_attribute_vec3(0, 0),
      vee::vertex_attribute_vec3(1, 0),
      vee::vertex_attribute_vec2(2, 0),
    },
  });
  voo::bound_buffer vb;
  voo::bound_buffer nb;
  voo::bound_buffer ib;
  voo::bound_buffer ub;
  hai::varray<vee::draw_indexed_params> xparams {};
};
static hai::uptr<app_stuff> gas {};

struct sized_stuff {
  voo::offscreen::depth_buffer depth { gas->dq.physical_device(), gas->dq.extent_of() };
  voo::swapchain_and_stuff sw { gas->dq, *gas->rp, depth.image_view() };
};
static hai::uptr<sized_stuff> gss {};

static void init() {
  gas.reset(new app_stuff {});

  auto src = jojo::read("DamagedHelmet.glb");
  const auto t = glub::parse(src.begin(), src.size());
  auto [v_count, i_count] = glub::mesh_counts::for_all_meshes(t);

  gas->vb = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(dotz::vec3) * v_count,
      vee::buffer_usage::vertex_buffer);
  gas->nb = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(dotz::vec3) * v_count,
      vee::buffer_usage::vertex_buffer);
  gas->ub = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(dotz::vec2) * v_count,
      vee::buffer_usage::vertex_buffer);
  gas->ib = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(short) * i_count,
      vee::buffer_usage::index_buffer);

  unsigned i_acc = 0;
  int v_acc = 0;
  glub::mesh_counts::for_each(t, [&](auto mc) {
    auto [v_count, i_count] = mc;
    gas->xparams.push_back_doubling(vee::draw_indexed_params {
      .xcount = i_count,
      .first_x = i_acc,
      .voffs = v_acc,
    });
    i_acc += i_count;
    v_acc += v_count;
  });

  casein::cursor_visible = false;
  casein::interrupt(casein::IRQ_CURSOR);

  glub::load_all_indices(t, static_cast<unsigned short *>(*voo::mapmem { *gas->ib.memory }));
  glub::load_all_vertices(t, static_cast<dotz::vec3 *>(*voo::mapmem { *gas->vb.memory }));
  glub::load_all_normals(t, static_cast<dotz::vec3 *>(*voo::mapmem { *gas->nb.memory }));
  glub::load_all_uvs(t, static_cast<dotz::vec2 *>(*voo::mapmem { *gas->ub.memory }));
}

static void frame() {
  if (!gss) gss.reset(new sized_stuff {});
  
  static sitime::stopwatch ftime {};
  dotz::vec2 cam_delta {
    ((g_key_a ? -1 : 0) + (g_key_d ? 1 : 0)),
    ((g_key_w ? -1 : 0) + (g_key_s ? 1 : 0))
  };
  if (dotz::sq_length(cam_delta) > 0) {
    float c = dotz::cos(g_pc.cam_rot.y);
    float s = dotz::sin(g_pc.cam_rot.y);
    auto d = dotz::normalise(cam_delta) * 2.0f * ftime.millis() / 1000.0f;
    g_pc.cam_pos.x += c * d.x + s * d.y;
    g_pc.cam_pos.z += -s * d.x + c * d.y;
  }
  ftime = {};

  gss->sw.acquire_next_image();
  gss->sw.queue_one_time_submit(gas->dq.queue(), [&] {
    static sitime::stopwatch time {};
    g_pc.aspect = gss->sw.aspect();
    g_pc.time = time.millis() / 1000.0f;

    auto cb = gss->sw.command_buffer();
    auto rp = gss->sw.cmd_render_pass({
      .command_buffer = gss->sw.command_buffer(),
      .clear_colours {
        vee::clear_colour(0.1, 0.2, 0.3, 1.0),
        vee::clear_depth(1),
      },
    });
    vee::cmd_set_viewport(cb, gss->sw.extent());
    vee::cmd_set_scissor(cb, gss->sw.extent());
    vee::cmd_bind_gr_pipeline(cb, *gas->gp);
    vee::cmd_bind_vertex_buffers(cb, 0, *gas->vb.buffer);
    vee::cmd_bind_vertex_buffers(cb, 1, *gas->nb.buffer);
    vee::cmd_bind_vertex_buffers(cb, 2, *gas->ub.buffer);
    vee::cmd_bind_index_buffer_u16(cb, *gas->ib.buffer);
    vee::cmd_push_vertex_constants(cb, *gas->pl, &g_pc);
    for (auto & p: gas->xparams) vee::cmd_draw_indexed(cb, p);
  });
  gss->sw.queue_present(gas->dq.queue());
}

const auto i = [] {
  using namespace vinyl;
  on(START,  &init);
  on(RESIZE, [] { gss.reset(nullptr); });
  on(FRAME,  &frame);
  on(STOP,   [] { 
    gss.reset(nullptr);
    gas.reset(nullptr);
  });

  using namespace casein;
  handle(MOUSE_MOVE, [] {
    mouse_pos = window_size / 2.0;
    interrupt(IRQ_MOUSE_POS);
  });
  handle(MOUSE_MOVE_REL, [] {
    g_pc.cam_rot.x -= mouse_rel.y * 0.01;
    g_pc.cam_rot.x = dotz::clamp(g_pc.cam_rot.x, -0.5f, 0.5f);
    g_pc.cam_rot.y -= mouse_rel.x * 0.01;
  });

  handle(KEY_DOWN, K_W, [] { g_key_w = true;  });
  handle(KEY_UP,   K_W, [] { g_key_w = false; });
  handle(KEY_DOWN, K_S, [] { g_key_s = true;  });
  handle(KEY_UP,   K_S, [] { g_key_s = false; });
  handle(KEY_DOWN, K_A, [] { g_key_a = true;  });
  handle(KEY_UP,   K_A, [] { g_key_a = false; });
  handle(KEY_DOWN, K_D, [] { g_key_d = true;  });
  handle(KEY_UP,   K_D, [] { g_key_d = false; });

  return 0;
}();
