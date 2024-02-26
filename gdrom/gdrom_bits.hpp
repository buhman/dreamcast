#pragma once

#include <cstdint>

#include "../float_uint32.hpp"

namespace gdrom {
  namespace status {
    constexpr uint32_t bsy(uint32_t reg) { return (reg >> 7) & 0x1; }
    constexpr uint32_t drdy(uint32_t reg) { return (reg >> 6) & 0x1; }
    constexpr uint32_t df(uint32_t reg) { return (reg >> 5) & 0x1; }
    constexpr uint32_t dsc(uint32_t reg) { return (reg >> 4) & 0x1; }
    constexpr uint32_t drq(uint32_t reg) { return (reg >> 3) & 0x1; }
    constexpr uint32_t corr(uint32_t reg) { return (reg >> 2) & 0x1; }
    constexpr uint32_t check(uint32_t reg) { return (reg >> 0) & 0x1; }
  }

  namespace alternate_status {
    constexpr uint32_t bsy(uint32_t reg) { return (reg >> 7) & 0x1; }
    constexpr uint32_t drdy(uint32_t reg) { return (reg >> 6) & 0x1; }
    constexpr uint32_t df(uint32_t reg) { return (reg >> 5) & 0x1; }
    constexpr uint32_t dsc(uint32_t reg) { return (reg >> 4) & 0x1; }
    constexpr uint32_t drq(uint32_t reg) { return (reg >> 3) & 0x1; }
    constexpr uint32_t corr(uint32_t reg) { return (reg >> 2) & 0x1; }
    constexpr uint32_t check(uint32_t reg) { return (reg >> 0) & 0x1; }
  }

  namespace command {
    namespace code {
      constexpr uint32_t soft_reset = 0x08 << 0;
      constexpr uint32_t execute_device_diagnostic = 0x90 << 0;
      constexpr uint32_t nop = 0x00 << 0;
      constexpr uint32_t packet_command = 0xa0 << 0;
      constexpr uint32_t identify_device = 0xa1 << 0;
      constexpr uint32_t set_features = 0xef << 0;

      constexpr uint32_t bit_mask = 0xff << 0;
    }
  }

  namespace device_control {
    constexpr uint32_t device_control = 0b1000 << 0;
    constexpr uint32_t srst = 1 << 2;
    constexpr uint32_t nien = 1 << 1;
  }

  namespace drive_select {
    constexpr uint32_t drive_select = 0b1010 << 4;
    constexpr uint32_t lun(uint32_t num) { return (num & 0xf) << 0; }
  }

  namespace error {
    constexpr uint32_t sense_key(uint32_t reg) { return (reg >> 4) & 0xf; }
    constexpr uint32_t mcr(uint32_t reg) { return (reg >> 3) & 0x1; }
    constexpr uint32_t abrt(uint32_t reg) { return (reg >> 2) & 0x1; }
    constexpr uint32_t eomf(uint32_t reg) { return (reg >> 1) & 0x1; }
    constexpr uint32_t ili(uint32_t reg) { return (reg >> 0) & 0x1; }
  }

  namespace features {
    namespace dma {
      constexpr uint32_t disable = 0 << 0;
      constexpr uint32_t enable = 1 << 0;

      constexpr uint32_t bit_mask = 0x1 << 0;
    }
  }

  namespace features_ata {
    namespace set_clear {
      constexpr uint32_t clear = 0 << 7;
      constexpr uint32_t set = 1 << 7;

      constexpr uint32_t bit_mask = 0x1 << 7;
    }

    namespace command {
      constexpr uint32_t set_transfer_mode = 3 << 0;

      constexpr uint32_t bit_mask = 0x7f << 0;
    }
  }

  namespace interrupt_reason {
    constexpr uint32_t io(uint32_t reg) { return (reg >> 1) & 0x1; }
    constexpr uint32_t cod(uint32_t reg) { return (reg >> 0) & 0x1; }
  }

  namespace sector_count {
    namespace transfer_mode {
      constexpr uint32_t pio_default_transfer_mode = 0b00000000 << 0;
      constexpr uint32_t pio_flow_control_transfer_mode = 0b00001000 << 0;
      constexpr uint32_t single_word_dma_mode = 0b00010000 << 0;
      constexpr uint32_t multi_word_dma_mode = 0b00100000 << 0;

      constexpr uint32_t bit_mask = 0xff << 0;
    }
  }

  namespace sector_number {
    constexpr uint32_t disc_format(uint32_t reg) { return (reg >> 4) & 0xf; }
    constexpr uint32_t status(uint32_t reg) { return (reg >> 0) & 0xf; }
  }

  namespace error_ata {
    namespace sense_key {
      constexpr uint32_t no_sense = 0x0 << 0;
      constexpr uint32_t recovered_error = 0x1 << 0;
      constexpr uint32_t not_ready = 0x2 << 0;
      constexpr uint32_t medium_error = 0x3 << 0;
      constexpr uint32_t hardware_error = 0x4 << 0;
      constexpr uint32_t illegal_request = 0x5 << 0;
      constexpr uint32_t unit_attention = 0x6 << 0;
      constexpr uint32_t data_protect = 0x7 << 0;
      constexpr uint32_t aborted_command = 0xb << 0;

      constexpr uint32_t bit_mask = 0xf << 0;
    }
  }

}
