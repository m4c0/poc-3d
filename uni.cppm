export module uni;
import hai;
import voo;

namespace uni {
  export struct gpu {
    float mat[16];
  };

  export class t {
    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_vertex_uniform(),
    });
    vee::descriptor_pool m_dpool;
    hai::array<vee::descriptor_set> m_dsets;

    voo::bound_buffer m_buf;
   
  public:
    explicit t(unsigned n) :
      m_dpool { vee::create_descriptor_pool(n, { vee::uniform_buffer(n) }) }
    , m_dsets { n }
    , m_buf { voo::bound_buffer::create_from_host(n * sizeof(gpu), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) }
    {
      auto bi = vee::descriptor_buffer_info(*m_buf.buffer);

      for (auto i = 0; i < n; i++) {
        m_dsets[i] = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

        bi.offset = i * sizeof(gpu);
        bi.range = sizeof(gpu);

        vee::update_descriptor_set(vee::write_descriptor_set({
          .dstSet = m_dsets[i],
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &bi,
        }));
      }
    }

    [[nodiscard]] constexpr auto dsl() const { return *m_dsl; }
    [[nodiscard]] constexpr auto dset(unsigned i) const { return m_dsets[i]; }

    void load(unsigned i, gpu g) {
      auto * ptr = vee::map_memory(*m_buf.memory, i * sizeof(gpu), sizeof(gpu));
      *static_cast<gpu *>(ptr) = g;
      vee::unmap_memory(*m_buf.memory);
    }
  };
}
