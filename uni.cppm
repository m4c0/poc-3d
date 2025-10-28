export module uni;
import voo;

namespace uni {
  export struct gpu {
    float mat[16];
  };

  export class t {
    vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
      vee::dsl_vertex_uniform(),
    });
    vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
      vee::uniform_buffer(),
    });
    vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

    voo::bound_buffer m_buf;
   
  public:
    t(vee::physical_device pd) :
      m_buf { voo::bound_buffer::create_from_host(pd, sizeof(gpu), vee::buffer_usage::uniform_buffer) }
    {
      auto bi = vee::descriptor_buffer_info(*m_buf.buffer);
      vee::update_descriptor_set(vee::write_descriptor_set({
        .dstSet = m_dset,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bi,
      }));
    }

    [[nodiscard]] constexpr auto dsl() const { return *m_dsl; }
    [[nodiscard]] constexpr auto dset() const { return m_dset; }

    void load(gpu g) {
      voo::memiter<gpu> m { *m_buf.memory };
      m += g;
    }
  };
}
