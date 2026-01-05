#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import glub;
import hai;
import jojo;
import sitime;
import stubby;
import traits;
import uni;
import vinyl;
import voo;

import print;

struct upc {
  float aspect;
  float fov = dotz::radians(80);
  float far = 10.0;
  float near = 0.01;

  dotz::vec4 colour { 1, 1, 1, 1 };
  dotz::vec4 light { -1, 0, 0, 0 };

  dotz::vec4 cam_pos { 0, 0, -3, 0 };
  dotz::vec3 cam_rot { 0.0f, 3.14f, 0.0f };
  float time;
} g_pc;

static bool g_key_w = false;
static bool g_key_s = false;
static bool g_key_a = false;
static bool g_key_d = false;

struct vertex {
  dotz::vec4 position {};
  dotz::vec4 normal {};
  dotz::vec2 uv {};
};
struct batch {
  vee::draw_indexed_params xparams {};
  dotz::vec4 colour { 1, 1, 1, 1 };
  int normal = -1;
  int texcolour = -1;
};
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

  uni::t uni { dq.physical_device(), 1 };

  vee::sampler smp = vee::create_sampler(vee::linear_sampler);
  vee::descriptor_set_layout dsl_smp = vee::create_descriptor_set_layout({
    vee::dsl_fragment_samplers({ *smp }),
  });
  vee::descriptor_pool dpool = vee::create_descriptor_pool(8, {
    vee::combined_image_sampler(8),
  });
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::pipeline_layout_params {
    .descriptor_set_layouts {{ *dsl_smp, *dsl_smp, uni.dsl() }},
    .push_constant_ranges {{ vee::vertex_push_constant_range<upc>() }},
  });
  vee::gr_pipeline gp = vee::create_graphics_pipeline({
    .pipeline_layout = *pl,
    .render_pass = *rp,
    .depth = vee::depth::op_less(),
    .shaders {
      voo::vert_shader("poc.vert.spv").pipeline_stage(),
      voo::frag_shader("poc.frag.spv").pipeline_stage(),
    },
    .bindings {
      vee::vertex_input_bind(sizeof(vertex)),
    },
    .attributes {
      vee::vertex_attribute_vec3(0, traits::offset_of(&vertex::position)),
      vee::vertex_attribute_vec4(0, traits::offset_of(&vertex::normal)),
      vee::vertex_attribute_vec2(0, traits::offset_of(&vertex::uv)),
    },
  });
  voo::bound_buffer vb;
  voo::bound_buffer ib;
  hai::array<voo::bound_image> imgs;
  hai::array<vee::descriptor_set> dsets;

  glub::t model;
  hai::array<hai::array<batch>> xparams {};
};
static hai::uptr<app_stuff> gas {};

struct sized_stuff {
  voo::offscreen::depth_buffer depth { gas->dq.physical_device(), gas->dq.extent_of() };
  voo::swapchain_and_stuff sw { gas->dq, *gas->rp, depth.image_view() };
};
static hai::uptr<sized_stuff> gss {};

template<typename T>
static auto cast(auto & acc, auto & t) {
  auto & bv = t.buffer_views[acc.buffer_view];
  auto ptr = reinterpret_cast<const T *>(t.data.begin() + acc.byte_offset + bv.byte_offset);
  if ((void *)(ptr + acc.count) > t.data.end()) die("buffer overrun");
  return ptr;
}
static void init() {
  gas.reset(new app_stuff {});

  auto src = jojo::slurp("DamagedHelmet.glb");
  const auto & t = gas->model = glub::parse(src.begin(), src.size());

  unsigned i_acc = 0;
  int v_acc = 0;
  gas->xparams.set_capacity(t.meshes.size());
  auto mm = gas->xparams.begin();
  for (auto & m : t.meshes) {
    *mm = hai::array<batch> { m.primitives.size() };
    auto pp = mm->begin();
    mm++;

    for (auto & p : m.primitives) {
      unsigned v_count = t.accessors[p.accessors.position].count;
      unsigned x_count = t.accessors[p.indices].count;
      auto & c = t.materials[p.material].base_colour_factor;
      auto nor = t.materials[p.material].normal_texture.index;
      auto tc0 = t.materials[p.material].base_colour_texture.index;

      *pp++ = batch {
        .xparams = vee::draw_indexed_params {
          .xcount = x_count,
          .first_x = i_acc,
          .voffs = v_acc,
        },
        .colour { c[0], c[1], c[2], c[3] },
        .normal = nor,
        .texcolour = tc0,
      };
      v_acc += v_count;
      i_acc += x_count;
    }
  }

  gas->vb = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(vertex) * v_acc,
      vee::buffer_usage::vertex_buffer);
  gas->ib = voo::bound_buffer::create_from_host(
      gas->dq.physical_device(),
      sizeof(short) * i_acc,
      vee::buffer_usage::index_buffer);

  casein::cursor_visible = false;
  casein::interrupt(casein::IRQ_CURSOR);

  voo::mapmem im { *gas->ib.memory }; auto ip = static_cast<unsigned short *>(*im);
  voo::mapmem vm { *gas->vb.memory }; auto vp = static_cast<vertex *>(*vm);
  for (auto & m : t.meshes) {
    for (auto & p : m.primitives) {
      auto & acc = t.accessors[p.indices];
      auto ptr = cast<unsigned short>(acc, t);
      for (auto i = 0; i < acc.count; i++) ip[i] = ptr[i];

      if (p.accessors.position >= 0) {
        auto & acc = t.accessors[p.accessors.position];
        auto ptr = cast<dotz::vec3>(acc, t);
        for (auto i = 0; i < acc.count; i++) vp[i].position = { ptr[i], 1 };
      }
      if (p.accessors.normal >= 0) {
        auto & acc = t.accessors[p.accessors.normal];
        auto ptr = cast<dotz::vec3>(acc, t);
        for (auto i = 0; i < acc.count; i++) vp[i].normal = { ptr[i], 1 };
      }
      if (p.accessors.texcoord_0 >= 0) {
        auto & acc = t.accessors[p.accessors.texcoord_0];
        auto ptr = cast<dotz::vec2>(acc, t);
        for (auto i = 0; i < acc.count; i++) vp[i].uv = ptr[i];
      }

      ip += acc.count;
      vp += t.accessors[p.accessors.position].count;
    }
  }

  gas->dsets.set_capacity(t.textures.size());
  gas->imgs.set_capacity(t.textures.size());
  auto imgptr = gas->imgs.begin();
  for (auto xi = 0; xi < t.textures.size(); xi++, imgptr++) {
    auto & x = t.textures[xi];
    auto & i = t.images[x.source];
    auto & bv = t.buffer_views[i.buffer_view];
    auto ptr = t.data.begin() + bv.byte_offset;

    auto img = stbi::load(ptr, bv.byte_length);
    unsigned w = img.width;
    unsigned h = img.height;
    unsigned sz = w * h * 4;
    auto host = voo::bound_buffer::create_from_host(gas->dq.physical_device(), sz);
    {
      voo::memiter<unsigned char> c { *host.memory };
      for (auto i = 0; i < sz; i++) c[i] = (*img.data)[i];
    }
  
    constexpr const auto fmt = VK_FORMAT_R8G8B8A8_SRGB;
    vee::extent ext { w, h };
    imgptr->img = vee::create_image(ext, fmt);
    imgptr->mem = vee::create_local_image_memory(gas->dq.physical_device(), *imgptr->img);
    vee::bind_image_memory(*imgptr->img, *imgptr->mem);
    imgptr->iv = vee::create_image_view(*imgptr->img, fmt);
  
    voo::fence f { false };
    voo::command_pool cpool {};
    auto cb = cpool.allocate_primary_command_buffer();
  
    {
      voo::cmd_buf_one_time_submit ots { cb };
      vee::cmd_pipeline_barrier(cb, *imgptr->img, vee::from_host_to_transfer);
      vee::cmd_copy_buffer_to_image(cb, ext, *host.buffer, *imgptr->img);
      vee::cmd_pipeline_barrier(cb, *imgptr->img, vee::from_transfer_to_fragment);
    }
    gas->dq.queue()->queue_submit({
      .fence = f,
      .command_buffer = cb,
    });
  
    f.wait();

    gas->dsets[xi] = vee::allocate_descriptor_set(*gas->dpool, *gas->dsl_smp);
    vee::update_descriptor_set(gas->dsets[xi], 0, *imgptr->iv);
  }

  gas->uni.load(0, {
    .mat {
      0.7f, 0.0f, 0.7f, 0.0f,
      0, 1, 0, 0,
      -0.7f, 0.0f, 0.7f, 0.0f,
      0, 0, 0, 1,
    }
  });
}

static void enqueue_nodes(vee::command_buffer cb, const hai::array<int> & nodes) {
  for (auto nidx : nodes) {
    auto & n = gas->model.nodes[nidx];
    enqueue_nodes(cb, n.children);

    if (n.mesh >= 0) {
      for (auto & p: gas->xparams[n.mesh]) {
        if (p.texcolour >= 0) vee::cmd_bind_descriptor_set(cb, *gas->pl, 0, gas->dsets[p.texcolour]);
        if (p.normal >= 0) vee::cmd_bind_descriptor_set(cb, *gas->pl, 1, gas->dsets[p.normal]);
        vee::cmd_bind_descriptor_set(cb, *gas->pl, 2, gas->uni.dset(0));
        g_pc.colour = p.colour;
        vee::cmd_push_vertex_constants(cb, *gas->pl, &g_pc);
        vee::cmd_draw_indexed(cb, p.xparams);
      }
    }
  }
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
  gss->sw.queue_one_time_submit([&] {
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
    vee::cmd_bind_index_buffer_u16(cb, *gas->ib.buffer);
    
    enqueue_nodes(cb, gas->model.scenes[gas->model.scene].nodes);
  });
  gss->sw.queue_present();
}

const auto i = [] {
  using namespace vinyl;
  on(START,  &::init);
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
