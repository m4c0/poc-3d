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
import hai;
import sitime;
import traits;
import vinyl;
import voo;

using namespace traits::ints;
using namespace wagen;

struct app_stuff;
struct ext_stuff;
using vv = vinyl::v<app_stuff, ext_stuff>;

struct upc {
  dotz::vec4 colour { 1, 1, 1, 0.3 };
  float aspect;
  float fov = 90;
  float time;
};
struct vtx {
  dotz::vec4 pos;
};

struct app_stuff : vinyl::base_app_stuff {
  voo::bound_buffer vbuf = voo::bound_buffer::create_from_host(1024 * sizeof(vtx), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  voo::bound_buffer xbuf = voo::bound_buffer::create_from_host(1024 * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  voo::bound_buffer shd_xbuf = voo::bound_buffer::create_from_host(1024 * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  voo::vert_shader vshd { "poc-shadowvolume.vert.spv" };
  voo::frag_shader fshd { "poc-shadowvolume.frag.spv" };

  vee::render_pass rp = vee::create_render_pass({
    .attachments {{
      vee::create_colour_attachment(dq.physical_device(), dq.surface()),
      vee::create_depth_stencil_attachment(),
    }},
    .subpasses {{
      vee::create_subpass({
        .colours {{ vee::create_attachment_ref(0, vee::image_layout_color_attachment_optimal) }},
        .depth_stencil = vee::create_attachment_ref(1, vee::image_layout_depth_stencil_attachment_optimal),
      }),
    }},
    .dependencies {{
      vee::create_colour_dependency(),
      vee::create_depth_dependency(),
    }},
  });
  vee::pipeline_layout pl = vee::create_pipeline_layout(vee::vert_frag_push_constant_range<upc>());

  inline vee::gr_pipeline create_pipeline(vee::gr_pipeline_params p) {
    return vee::create_graphics_pipeline({
      .pipeline_layout = *pl,
      .render_pass = *rp,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .back_face_cull = p.back_face_cull,
      .depth = p.depth,
      .blends = p.blends,
      .shaders { *vshd, *fshd },
      .bindings {
        vee::vertex_input_bind(sizeof(vtx)),
      },
      .attributes {
        vee::vertex_attribute_vec4(0, traits::offset_of(&vtx::pos)),
      },
    });
  }

  vee::gr_pipeline ppl = create_pipeline({
    .back_face_cull = false,
    .depth = vee::depth::of({
      .depthTestEnable = vk_true,
      .depthWriteEnable = vk_true,
      .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
    }),
    .blends { VkPipelineColorBlendAttachmentState { .colorWriteMask = VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT } },
  });
  vee::gr_pipeline shd_ppl = create_pipeline({
    .back_face_cull = false,
    .depth = vee::depth::of({
      .depthTestEnable = vk_true,
      .depthWriteEnable = vk_false,
      .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
      .stencilTestEnable = vk_true,
      .front = {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = ~0U,
        .writeMask = ~0U,
      },
      .back = {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = ~0U,
        .writeMask = ~0U,
      },
    }),
    .blends { VkPipelineColorBlendAttachmentState { .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT } },
  });
  vee::gr_pipeline light_ppl = create_pipeline({
    .back_face_cull = false,
    .depth = vee::depth::of({
      .depthTestEnable = vk_true,
      .depthWriteEnable = vk_false,
      .depthCompareOp = VK_COMPARE_OP_EQUAL,
      .stencilTestEnable = vk_true,
      .front = {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_EQUAL,
        .compareMask = ~0U,
        .writeMask = ~0U,
      },
      .back = {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_EQUAL,
        .compareMask = ~0U,
        .writeMask = ~0U,
      },
    }),
    .blends { VkPipelineColorBlendAttachmentState { .colorWriteMask = VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT } },
  });

  app_stuff() : base_app_stuff { "poc-3d" } {
    auto m = voo::memiter<vtx> { *vbuf.memory };

    // Light (w = 0, signaling shader to use light vector)
    m += vtx { .pos {} };

    // Bottom
    m += vtx { .pos { -0.9, -1.0, -0.9, 1.0 } };
    m += vtx { .pos {  0.9, -1.0, -0.9, 1.0 } };
    m += vtx { .pos { -0.9, -1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9, -1.0,  0.9, 1.0 } };

    // Top
    m += vtx { .pos { -0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos { -0.9,  1.0,  0.9, 1.0 } };
    m += vtx { .pos {  0.9,  1.0, -0.9, 1.0 } };
    m += vtx { .pos {  0.9,  1.0,  0.9, 1.0 } };

    struct tri { uint16_t x[3]; };
    auto mx = voo::memiter<tri> { *xbuf.memory };
    // Bottom
    mx += {{ 1, 2, 3 }}; mx += {{ 4, 3, 2 }};
    // Top
    mx += {{ 5, 6, 7 }}; mx += {{ 8, 7, 6 }};
    
    auto msx = voo::memiter<tri> { *shd_xbuf.memory };
    // Bottom Shadows
    msx += {{ 1, 2, 0 }};
    msx += {{ 4, 3, 0 }};
    msx += {{ 3, 1, 0 }};
    msx += {{ 2, 4, 0 }};
    // Top Shadows
    msx += {{ 6, 5, 0 }};
    msx += {{ 7, 8, 0 }};
    msx += {{ 5, 7, 0 }};
    msx += {{ 8, 6, 0 }};
  }
};
static auto depth_image_info() {
  auto ext = vv::as()->dq.extent_of();
  return vee::image_create_info(ext, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
struct ext_stuff {
  voo::single_cb cb {};
  voo::bound_image depth = voo::bound_image::create(depth_image_info(), VK_IMAGE_ASPECT_DEPTH_BIT);
  voo::swapchain swc { vv::as()->dq };
  hai::array<vee::framebuffer> fbs = swc.create_framebuffers(*vv::as()->rp, *depth.iv);
};

extern "C" void casein_init() {
  vv::setup([] {
    auto cb = vv::ss()->cb.cb();

    vv::ss()->swc.acquire_next_image();
    vv::ss()->swc.queue_one_time_submit(cb, [] {
      static sitime::stopwatch timer {};

      upc pc {
        .aspect = vv::ss()->swc.aspect(),
        .time = timer.secs(),
      };

      auto cb = vv::ss()->cb.cb();
      auto ext = vv::ss()->swc.extent();

      voo::cmd_render_pass rp { vee::render_pass_begin {
        .command_buffer = cb,
        .render_pass = *vv::as()->rp,
        .framebuffer = *vv::ss()->fbs[vv::ss()->swc.index()],
        .extent = ext,
        .clear_colours { 
          vee::clear_colour({ 0, 0, 0, 1 }), 
          vee::clear_depth(1.0, 0),
        },
      }, true };
      vee::cmd_set_viewport_flipped(cb, ext);
      vee::cmd_set_scissor(cb, ext);
      vee::cmd_bind_vertex_buffers(cb, 0, *vv::as()->vbuf.buffer);
      vee::cmd_bind_index_buffer_u16(cb, *vv::as()->xbuf.buffer);
      vee::cmd_push_vert_frag_constants(cb, *vv::as()->pl, &pc);

      // bottom
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->ppl);
      vee::cmd_draw_indexed(cb, { .xcount = 12, .first_x = 0 });

      // shadow edge
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->shd_ppl);
      vee::cmd_bind_index_buffer_u16(cb, *vv::as()->shd_xbuf.buffer);
      vee::cmd_draw_indexed(cb, { .xcount = 24 });

      // lights
      vee::cmd_bind_gr_pipeline(cb, *vv::as()->light_ppl);
      vee::cmd_bind_index_buffer_u16(cb, *vv::as()->xbuf.buffer);
      vee::cmd_draw_indexed(cb, { .xcount = 12, .first_x = 0 });
    });
    vv::ss()->swc.queue_present();
  });
}
