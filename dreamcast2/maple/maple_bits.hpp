#pragma once

#include <cstdint>

namespace maple {
  namespace mdstar {
    constexpr inline uint32_t table_address(uint32_t num) { return (num & 0xfffffe0) << 0; }
  }

  namespace mdtsel {
    namespace trigger_select {
      constexpr uint32_t software_initiation = 0 << 0;
      constexpr uint32_t v_blank_initiation = 1 << 0;

      constexpr uint32_t bit_mask = 0x1 << 0;
    }
  }

  namespace mden {
    namespace dma_enable {
      constexpr uint32_t abort = 0 << 0;
      constexpr uint32_t enable = 1 << 0;

      constexpr uint32_t bit_mask = 0x1 << 0;
    }
  }

  namespace mdst {
    namespace start_status {
      constexpr inline uint32_t status(uint32_t reg) { return (reg >> 0) & 0x1; }
      constexpr uint32_t start = 1 << 0;

      constexpr uint32_t bit_mask = 0x1 << 0;
    }
  }

  namespace msys {
    constexpr inline uint32_t time_out_counter(uint32_t num) { return (num & 0xffff) << 16; }
    constexpr uint32_t single_hard_trigger = 1 << 12;

    namespace sending_rate {
      constexpr uint32_t _2M = 0 << 8;
      constexpr uint32_t _1M = 1 << 8;

      constexpr uint32_t bit_mask = 0x3 << 8;
    }

    constexpr inline uint32_t delay_time(uint32_t num) { return (num & 0xf) << 0; }
  }

  namespace mst {
    constexpr inline uint32_t move_status(uint32_t reg) { return (reg >> 31) & 0x1; }
    constexpr inline uint32_t internal_frame_monitor(uint32_t reg) { return (reg >> 24) & 0x7; }
    constexpr inline uint32_t internal_state_monitor(uint32_t reg) { return (reg >> 16) & 0x3f; }
    constexpr inline uint32_t line_monitor(uint32_t reg) { return (reg >> 0) & 0xff; }
  }

  namespace mshtcl {
    constexpr uint32_t hard_dma_clear = 1 << 0;
  }

  namespace mdapro {
    constexpr uint32_t security_code = 0x6155 << 16;
    constexpr inline uint32_t top_address(uint32_t num) { return (num & 0x7f) << 8; }
    constexpr inline uint32_t bottom_address(uint32_t num) { return (num & 0x7f) << 0; }
  }

  namespace mmsel {
    namespace msb_selection {
      constexpr uint32_t bit7 = 0 << 0;
      constexpr uint32_t bit31 = 1 << 0;

      constexpr uint32_t bit_mask = 0x1 << 0;
    }
  }

  namespace mtxdad {
    constexpr inline uint32_t txd_address_counter(uint32_t reg) { return (reg >> 0) & 0x1fffffff; }
  }

  namespace mrxdad {
    constexpr inline uint32_t rxd_address_counter(uint32_t reg) { return (reg >> 0) & 0x1fffffff; }
  }

  namespace mrxdbd {
    constexpr inline uint32_t rxd_base_address(uint32_t reg) { return (reg >> 0) & 0x1fffffff; }
  }

}
