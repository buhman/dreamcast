#pragma once

#include <cstdint>

namespace sh7091 {
  namespace ccn {
    namespace pteh {
      constexpr inline uint32_t VPN(uint32_t reg) { return (reg >> 10) & 0x3fffff; }
      constexpr inline uint32_t ASID(uint32_t reg) { return (reg >> 0) & 0xff; }
    }

    namespace ptel {
      constexpr inline uint32_t PPN(uint32_t reg) { return (reg >> 10) & 0x7ffff; }

      namespace v {
        constexpr uint32_t invalid = 0 << 8;
        constexpr uint32_t valid = 1 << 8;

        constexpr uint32_t bit_mask = 0x1 << 8;
      }

      namespace sz {
        constexpr uint32_t _1_kbyte_page = 0b0000 << 4;
        constexpr uint32_t _4_kbyte_page = 0b0001 << 4;
        constexpr uint32_t _64_kbyte_page = 0b1000 << 4;
        constexpr uint32_t _1_mbyte_page = 0b1001 << 4;

        constexpr uint32_t bit_mask = 0x9 << 4;
      }

      namespace pr {
        constexpr uint32_t read_only_in_privileged_mode = 0b00 << 5;
        constexpr uint32_t read_write_in_privileged_mode = 0b01 << 5;
        constexpr uint32_t read_only_in_privileged_and_user_mode = 0b10 << 5;
        constexpr uint32_t read_write_in_privileged_and_user_mode = 0b11 << 5;

        constexpr uint32_t bit_mask = 0x3 << 5;
      }

      namespace c {
        constexpr uint32_t not_cacheable = 0 << 3;
        constexpr uint32_t cacheable = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace d {
        constexpr uint32_t write_has_not_been_performed = 0 << 2;
        constexpr uint32_t write_has_been_performed = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace sh {
        constexpr uint32_t pages_are_shared_by_processes = 0 << 1;
        constexpr uint32_t pages_are_not_shared_by_processes = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace wt {
        constexpr uint32_t copy_back_mode = 0 << 0;
        constexpr uint32_t write_through_mode = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace mmucr {
      constexpr inline uint32_t LRUI(uint32_t reg) { return (reg >> 26) & 0x3f; }
      constexpr inline uint32_t URB(uint32_t reg) { return (reg >> 18) & 0x3f; }
      constexpr inline uint32_t URC(uint32_t reg) { return (reg >> 10) & 0x3f; }

      namespace sqmd {
        constexpr uint32_t user_privileged_access_possible = 0 << 9;
        constexpr uint32_t privileged_access_possible = 1 << 9;

        constexpr uint32_t bit_mask = 0x1 << 9;
      }

      namespace sv {
        constexpr uint32_t multiple_virtual_memory_mode = 0 << 8;
        constexpr uint32_t single_virtual_memory_mode = 1 << 8;

        constexpr uint32_t bit_mask = 0x1 << 8;
      }

      namespace ti {
        constexpr uint32_t invalidate_all_utlb_itlb_bits = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace at {
        constexpr uint32_t mmu_disabled = 0 << 0;
        constexpr uint32_t mmu_enabled = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace basra {
      constexpr inline uint32_t basa(uint32_t num) { return (num & 0xff) << 0; }
    }

    namespace basrb {
      constexpr inline uint32_t basa(uint32_t num) { return (num & 0xff) << 0; }
    }

    namespace ccr {
      namespace iix {
        constexpr uint32_t address_bits_12_5_used_for_ic_entry_selection = 0 << 15;
        constexpr uint32_t address_bits_25_and_11_5_used_for_ic_entry_selection = 1 << 15;

        constexpr uint32_t bit_mask = 0x1 << 15;
      }

      namespace ici {
        constexpr uint32_t clear_v_bits_of_all_ic_entries = 1 << 11;

        constexpr uint32_t bit_mask = 0x1 << 11;
      }

      namespace ice {
        constexpr uint32_t ic_not_used = 0 << 8;
        constexpr uint32_t ic_used = 1 << 8;

        constexpr uint32_t bit_mask = 0x1 << 8;
      }

      namespace oix {
        constexpr uint32_t address_bits_13_5_used_for_oc_entry_selection = 0 << 7;
        constexpr uint32_t address_bits_25_and_12_5_used_for_oc_entry_selection = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace ora {
        constexpr uint32_t _16_kbytes_used_as_cache = 0 << 5;
        constexpr uint32_t _8_kbytes_used_as_cache_8_kbytes_used_as_ram = 1 << 5;

        constexpr uint32_t bit_mask = 0x1 << 5;
      }

      namespace oci {
        constexpr uint32_t clear_v_and_u_bits_of_all_oc_entries = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace cb {
        constexpr uint32_t write_through_mode = 0 << 2;
        constexpr uint32_t copy_back_mode = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace wt {
        constexpr uint32_t copy_back_mode = 0 << 1;
        constexpr uint32_t write_through_mode = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace oce {
        constexpr uint32_t oc_not_used = 0 << 0;
        constexpr uint32_t oc_used = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace tra {
      constexpr inline uint32_t imm(uint32_t reg) { return (reg >> 2) & 0xff; }
    }

    namespace expevt {
      constexpr inline uint32_t exception_code(uint32_t reg) { return (reg >> 0) & 0xfff; }
    }

    namespace intevt {
      constexpr inline uint32_t exception_code(uint32_t reg) { return (reg >> 0) & 0xfff; }
    }

    namespace ptea {
      namespace tc {
        constexpr uint32_t area_5_is_used = 0 << 3;
        constexpr uint32_t area_6_is_used = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace sa {
        constexpr uint32_t undefined = 0b000 << 0;
        constexpr uint32_t variable_size_io_space = 0b001 << 0;
        constexpr uint32_t _8_bit_io_space = 0b010 << 0;
        constexpr uint32_t _16_bit_io_space = 0b011 << 0;
        constexpr uint32_t _8_bit_common_memory_space = 0b100 << 0;
        constexpr uint32_t _16_bit_common_memory_space = 0b101 << 0;
        constexpr uint32_t _8_bit_attribute_memory_space = 0b110 << 0;
        constexpr uint32_t _16_bit_attribute_memory_space = 0b111 << 0;

        constexpr uint32_t bit_mask = 0x7 << 0;
      }
    }

    namespace qacr0 {
      constexpr inline uint32_t area(uint32_t num) { return (num & 0x7) << 2; }
    }

    namespace qacr1 {
      constexpr inline uint32_t area(uint32_t num) { return (num & 0x7) << 2; }
    }

  }

  namespace dmac {
    namespace dmatcr {
      constexpr inline uint32_t transfer_count(uint32_t num) { return (num & 0xffffff) << 0; }
    }

    namespace chcr {
      namespace ssa {
        constexpr uint32_t reserved_in_pcmcia_access = 0b000 << 29;
        constexpr uint32_t dynamic_bus_sizing_io_space = 0b001 << 29;
        constexpr uint32_t _8_bit_io_space = 0b010 << 29;
        constexpr uint32_t _16_bit_io_space = 0b011 << 29;
        constexpr uint32_t _8_bit_common_memory_space = 0b100 << 29;
        constexpr uint32_t _16_bit_common_memory_space = 0b101 << 29;
        constexpr uint32_t _8_bit_attribute_memory_space = 0b110 << 29;
        constexpr uint32_t _16_bit_attribute_memory_space = 0b111 << 29;

        constexpr uint32_t bit_mask = 0x7 << 29;
      }

      namespace stc {
        constexpr uint32_t c5_space_wait_cycle_selection = 0 << 28;
        constexpr uint32_t c6_space_wait_cycle_selection = 1 << 28;

        constexpr uint32_t bit_mask = 0x1 << 28;
      }

      namespace dsa {
        constexpr uint32_t reserved_in_pcmcia_access = 0b000 << 25;
        constexpr uint32_t dynamic_bus_sizing_io_space = 0b001 << 25;
        constexpr uint32_t _8_bit_io_space = 0b010 << 25;
        constexpr uint32_t _16_bit_io_space = 0b011 << 25;
        constexpr uint32_t _8_bit_common_memory_space = 0b100 << 25;
        constexpr uint32_t _16_bit_common_memory_space = 0b101 << 25;
        constexpr uint32_t _8_bit_attribute_memory_space = 0b110 << 25;
        constexpr uint32_t _16_bit_attribute_memory_space = 0b111 << 25;

        constexpr uint32_t bit_mask = 0x7 << 25;
      }

      namespace dtc {
        constexpr uint32_t c5_space_wait_cycle_selection = 0 << 24;
        constexpr uint32_t c6_space_wait_cycle_selection = 1 << 24;

        constexpr uint32_t bit_mask = 0x1 << 24;
      }

      namespace ds {
        constexpr uint32_t low_level_detection = 0 << 19;
        constexpr uint32_t falling_edge_detection = 1 << 19;

        constexpr uint32_t bit_mask = 0x1 << 19;
      }

      namespace rl {
        constexpr uint32_t drak_is_an_active_high = 0 << 18;
        constexpr uint32_t drak_is_an_active_low = 1 << 18;

        constexpr uint32_t bit_mask = 0x1 << 18;
      }

      namespace am {
        constexpr uint32_t dack_is_output_in_read_cycle = 0 << 17;
        constexpr uint32_t dack_is_output_in_write_cycle = 1 << 17;

        constexpr uint32_t bit_mask = 0x1 << 17;
      }

      namespace al {
        constexpr uint32_t active_high_output = 0 << 16;
        constexpr uint32_t active_low_output = 1 << 16;

        constexpr uint32_t bit_mask = 0x1 << 16;
      }

      namespace dm {
        constexpr uint32_t destination_address_fixed = 0b00 << 14;
        constexpr uint32_t destination_address_incremented = 0b01 << 14;
        constexpr uint32_t destination_address_decremented = 0b10 << 14;

        constexpr uint32_t bit_mask = 0x3 << 14;
      }

      namespace sm {
        constexpr uint32_t source_address_fixed = 0b00 << 12;
        constexpr uint32_t source_address_incremented = 0b01 << 12;
        constexpr uint32_t source_address_decremented = 0b10 << 12;

        constexpr uint32_t bit_mask = 0x3 << 12;
      }

      namespace rs {
        constexpr inline uint32_t resource_select(uint32_t num) { return (num & 0xf) << 8; }

        constexpr uint32_t bit_mask = 0xf << 8;
      }

      namespace tm {
        constexpr uint32_t cycle_steal_mode = 0 << 7;
        constexpr uint32_t cycle_burst_mode = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace ts {
        constexpr uint32_t _64_bit = 0b000 << 4;
        constexpr uint32_t _8_bit = 0b001 << 4;
        constexpr uint32_t _16_bit = 0b010 << 4;
        constexpr uint32_t _32_bit = 0b011 << 4;
        constexpr uint32_t _32_byte = 0b100 << 4;

        constexpr uint32_t bit_mask = 0x7 << 4;
      }

      namespace ie {
        constexpr uint32_t interrupt_request_not_generated = 0 << 2;
        constexpr uint32_t interrupt_request_generated = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace te {
        constexpr uint32_t transfers_not_completed = 0 << 1;
        constexpr uint32_t transfers_completed = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace de {
        constexpr uint32_t channel_operation_disabled = 0 << 0;
        constexpr uint32_t channel_operation_enabled = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace dmaor {
      namespace ddt {
        constexpr uint32_t normal_dma_mode = 0 << 15;
        constexpr uint32_t on_demand_data_transfer_mode = 1 << 15;

        constexpr uint32_t bit_mask = 0x1 << 15;
      }

      namespace pr {
        constexpr uint32_t ch0_ch1_ch2_ch3 = 0b00 << 8;
        constexpr uint32_t ch0_ch2_ch3_ch1 = 0b01 << 8;
        constexpr uint32_t ch2_ch0_ch1_ch3 = 0b10 << 8;
        constexpr uint32_t round_robin = 0b11 << 8;

        constexpr uint32_t bit_mask = 0x3 << 8;
      }

      namespace ae {
        constexpr uint32_t no_address_error__dma_transfer_enabled = 0 << 2;
        constexpr uint32_t address_error__dma_transfer_disabled = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace nmif {
        constexpr uint32_t no_nmi__dma_transfer_enabled = 0 << 1;
        constexpr uint32_t nmi__dma_transfer_disabled = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace dme {
        constexpr uint32_t operation_disabled_on_all_channels = 0 << 0;
        constexpr uint32_t operation_enabled_on_all_channels = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

  }

  namespace intc {
    namespace icr {
      namespace nmil {
        constexpr uint32_t pin_input_level_is_low = 0 << 15;
        constexpr uint32_t pin_input_level_is_high = 1 << 15;

        constexpr uint32_t bit_mask = 0x1 << 15;
      }

      namespace mai {
        constexpr uint32_t interrupts_enabled_while_nmi_pin_is_low = 0 << 14;
        constexpr uint32_t interrupts_disabled_while_nmi_pin_is_low = 1 << 14;

        constexpr uint32_t bit_mask = 0x1 << 14;
      }

      namespace nmib {
        constexpr uint32_t interrupt_requests_witheld = 0 << 9;
        constexpr uint32_t interrupt_requests_detected = 1 << 9;

        constexpr uint32_t bit_mask = 0x1 << 9;
      }

      namespace nmie {
        constexpr uint32_t interrupt_on_falling_edge_of_nmi = 0 << 8;
        constexpr uint32_t interrupt_on_rising_edge_of_nmi = 1 << 8;

        constexpr uint32_t bit_mask = 0x1 << 8;
      }

      namespace irlm {
        constexpr uint32_t level_encoded_interrupt_requests = 0 << 7;
        constexpr uint32_t independent_interrupt_request = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }
    }

    namespace ipra {
      constexpr inline uint32_t TMU0(uint32_t num) { return (num & 0xf) << 12; }
      constexpr inline uint32_t TMU1(uint32_t num) { return (num & 0xf) << 8; }
      constexpr inline uint32_t TMU2(uint32_t num) { return (num & 0xf) << 4; }
      constexpr inline uint32_t RTC(uint32_t num) { return (num & 0xf) << 0; }
    }

    namespace iprb {
      constexpr inline uint32_t WDT(uint32_t num) { return (num & 0xf) << 12; }
      constexpr inline uint32_t REF(uint32_t num) { return (num & 0xf) << 8; }
      constexpr inline uint32_t SCI1(uint32_t num) { return (num & 0xf) << 4; }
    }

    namespace iprc {
      constexpr inline uint32_t GPIO(uint32_t num) { return (num & 0xf) << 12; }
      constexpr inline uint32_t DMAC(uint32_t num) { return (num & 0xf) << 8; }
      constexpr inline uint32_t SCIF(uint32_t num) { return (num & 0xf) << 4; }
      constexpr inline uint32_t UDI(uint32_t num) { return (num & 0xf) << 0; }
    }

  }

  namespace tmu {
    namespace tocr {
      namespace tcoe {
        constexpr uint32_t tclk_is_external_clock_or_input_capture = 0 << 0;
        constexpr uint32_t tclk_is_on_chip_rtc = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace tstr {
      namespace str2 {
        constexpr uint32_t counter_start = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace str1 {
        constexpr uint32_t counter_start = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace str0 {
        constexpr uint32_t counter_start = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace tcr0 {
      constexpr uint32_t UNF = 1 << 8;
      constexpr uint32_t UNIE = 1 << 5;

      namespace ckeg {
        constexpr uint32_t rising = 0b00 << 3;
        constexpr uint32_t falling = 0b01 << 3;
        constexpr uint32_t rising_falling = 0b10 << 3;

        constexpr uint32_t bit_mask = 0x3 << 3;
      }

      namespace tpsc {
        constexpr uint32_t p_phi_4 = 0b000 << 0;
        constexpr uint32_t p_phi_16 = 0b001 << 0;
        constexpr uint32_t p_phi_64 = 0b010 << 0;
        constexpr uint32_t p_phi_256 = 0b011 << 0;
        constexpr uint32_t p_phi_1024 = 0b100 << 0;
        constexpr uint32_t rtc_output = 0b110 << 0;
        constexpr uint32_t external = 0b111 << 0;

        constexpr uint32_t bit_mask = 0x7 << 0;
      }
    }

    namespace tcr1 {
      constexpr uint32_t UNF = 1 << 8;
      constexpr uint32_t UNIE = 1 << 5;

      namespace ckeg {
        constexpr uint32_t rising = 0b00 << 3;
        constexpr uint32_t falling = 0b01 << 3;
        constexpr uint32_t rising_falling = 0b10 << 3;

        constexpr uint32_t bit_mask = 0x3 << 3;
      }

      namespace tpsc {
        constexpr uint32_t p_phi_4 = 0b000 << 0;
        constexpr uint32_t p_phi_16 = 0b001 << 0;
        constexpr uint32_t p_phi_64 = 0b010 << 0;
        constexpr uint32_t p_phi_256 = 0b011 << 0;
        constexpr uint32_t p_phi_1024 = 0b100 << 0;
        constexpr uint32_t rtc_output = 0b110 << 0;
        constexpr uint32_t external = 0b111 << 0;

        constexpr uint32_t bit_mask = 0x7 << 0;
      }
    }

    namespace tcr2 {
      constexpr uint32_t ICPF = 1 << 9;
      constexpr uint32_t UNF = 1 << 8;

      namespace icpe {
        constexpr uint32_t disabled = 0b00 << 6;
        constexpr uint32_t enabled = 0b10 << 6;
        constexpr uint32_t enabled_with_interrupts = 0b11 << 6;

        constexpr uint32_t bit_mask = 0x3 << 6;
      }

      constexpr uint32_t UNIE = 1 << 5;

      namespace ckeg {
        constexpr uint32_t rising = 0b00 << 3;
        constexpr uint32_t falling = 0b01 << 3;
        constexpr uint32_t rising_falling = 0b10 << 3;

        constexpr uint32_t bit_mask = 0x3 << 3;
      }

      namespace tpsc {
        constexpr uint32_t p_phi_4 = 0b000 << 0;
        constexpr uint32_t p_phi_16 = 0b001 << 0;
        constexpr uint32_t p_phi_64 = 0b010 << 0;
        constexpr uint32_t p_phi_256 = 0b011 << 0;
        constexpr uint32_t p_phi_1024 = 0b100 << 0;
        constexpr uint32_t rtc_output = 0b110 << 0;
        constexpr uint32_t external = 0b111 << 0;

        constexpr uint32_t bit_mask = 0x7 << 0;
      }
    }

  }

  namespace scif {
    namespace scsmr2 {
      namespace chr {
        constexpr uint32_t _8_bit_data = 0 << 6;
        constexpr uint32_t _7_bit_data = 1 << 6;

        constexpr uint32_t bit_mask = 0x1 << 6;
      }

      namespace pe {
        constexpr uint32_t parity_disabled = 0 << 5;
        constexpr uint32_t parity_enabled = 1 << 5;

        constexpr uint32_t bit_mask = 0x1 << 5;
      }

      namespace oe {
        constexpr uint32_t even_parity = 0 << 4;
        constexpr uint32_t odd_parity = 1 << 4;

        constexpr uint32_t bit_mask = 0x1 << 4;
      }

      namespace stop {
        constexpr uint32_t _1_stop_bit = 0 << 3;
        constexpr uint32_t _2_stop_bits = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace cks {
        constexpr uint32_t p_phi_clock = 0b00 << 0;
        constexpr uint32_t p_phi_4_clock = 0b01 << 0;
        constexpr uint32_t p_phi_16_clock = 0b10 << 0;
        constexpr uint32_t p_phi_64_clock = 0b11 << 0;

        constexpr uint32_t bit_mask = 0x3 << 0;
      }
    }

    namespace scscr2 {
      namespace tie {
        constexpr uint32_t transmit_fifo_data_empty_interrupt_disabled = 0 << 7;
        constexpr uint32_t transmit_fifo_data_empty_interrupt_enabled = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace rie {
        constexpr uint32_t request_disabled = 0 << 6;
        constexpr uint32_t request_enabled = 1 << 6;

        constexpr uint32_t bit_mask = 0x1 << 6;
      }

      namespace te {
        constexpr uint32_t transmission_disabled = 0 << 5;
        constexpr uint32_t transmission_enabled = 1 << 5;

        constexpr uint32_t bit_mask = 0x1 << 5;
      }

      namespace re {
        constexpr uint32_t reception_disabled = 0 << 4;
        constexpr uint32_t reception_enabled = 1 << 4;

        constexpr uint32_t bit_mask = 0x1 << 4;
      }

      namespace reie {
        constexpr uint32_t requests_disabled = 0 << 3;
        constexpr uint32_t requests_enabled = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace cke1 {
        constexpr uint32_t sck2_pin_functions_as_input_pin = 0 << 1;
        constexpr uint32_t sck2_pin_functions_as_clock_input = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }
    }

    namespace scfsr2 {
      namespace per3_0 {
        constexpr inline uint32_t number_of_parity_errors(uint32_t reg) { return (reg >> 12) & 0xf; }

        constexpr uint32_t bit_mask = 0xf << 12;
      }

      namespace fer3_0 {
        constexpr inline uint32_t number_of_framing_errors(uint32_t reg) { return (reg >> 8) & 0xf; }

        constexpr uint32_t bit_mask = 0xf << 8;
      }

      namespace er {
        constexpr uint32_t no_framing_error_or_parity_error = 0 << 7;
        constexpr uint32_t framing_error_or_parity_error = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace tend {
        constexpr uint32_t transmission_in_progress = 0 << 6;
        constexpr uint32_t transmission_has_ended = 1 << 6;

        constexpr uint32_t bit_mask = 0x1 << 6;
      }

      namespace tdfe {
        constexpr uint32_t transmit_data_bytes_does_exceed_trigger = 0 << 5;
        constexpr uint32_t transmit_data_bytes_does_not_exceed_trigger = 1 << 5;

        constexpr uint32_t bit_mask = 0x1 << 5;
      }

      namespace brk {
        constexpr uint32_t break_not_received = 0 << 4;
        constexpr uint32_t break_received = 1 << 4;

        constexpr uint32_t bit_mask = 0x1 << 4;
      }

      namespace fer {
        constexpr uint32_t no_framing_error = 0 << 3;
        constexpr uint32_t framing_error = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace per {
        constexpr uint32_t parity_error = 0 << 2;
        constexpr uint32_t no_parity_error = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace rdf {
        constexpr uint32_t receive_data_bytes_less_than_receive_trigger = 0 << 1;
        constexpr uint32_t receive_data_bytes_greater_than_or_equal_receive_trigger = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace dr {
        constexpr uint32_t reception_is_in_progress = 0 << 0;
        constexpr uint32_t no_further_data_has_arrived = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace scfcr2 {
      namespace rtrg {
        constexpr uint32_t trigger_on_1_byte = 0b00 << 6;
        constexpr uint32_t trigger_on_4_bytes = 0b01 << 6;
        constexpr uint32_t trigger_on_8_bytes = 0b10 << 6;
        constexpr uint32_t trigger_on_14_byte = 0b11 << 6;

        constexpr uint32_t bit_mask = 0x3 << 6;
      }

      namespace ttrg {
        constexpr uint32_t trigger_on_8_bytes = 0b00 << 4;
        constexpr uint32_t trigger_on_4_bytes = 0b01 << 4;
        constexpr uint32_t trigger_on_2_bytes = 0b10 << 4;
        constexpr uint32_t trigger_on_1_bytes = 0b11 << 4;

        constexpr uint32_t bit_mask = 0x3 << 4;
      }

      namespace mce {
        constexpr uint32_t modem_signals_disabled = 0 << 3;
        constexpr uint32_t modem_signals_enabled = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace tfrst {
        constexpr uint32_t reset_operation_disabled = 0 << 2;
        constexpr uint32_t reset_operation_enabled = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }

      namespace rfrst {
        constexpr uint32_t reset_operation_disabled = 0 << 1;
        constexpr uint32_t reset_operation_enabled = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace loop {
        constexpr uint32_t loopback_test_disabled = 0 << 0;
        constexpr uint32_t loopback_test_enabled = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace scfdr2 {
      constexpr inline uint32_t transmit_data_bytes(uint32_t reg) { return (reg >> 8) & 0x1f; }
      constexpr inline uint32_t receive_data_bytes(uint32_t reg) { return (reg >> 0) & 0x1f; }
    }

    namespace scsptr2 {
      namespace rtsio {
        constexpr uint32_t rtsdt_not_output_to_rts2 = 0 << 7;
        constexpr uint32_t rtsdt_output_to_rts2 = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace rtsdt {
        constexpr uint32_t input_output_data_is_low_level = 0 << 6;
        constexpr uint32_t input_output_data_is_high_level = 1 << 6;

        constexpr uint32_t bit_mask = 0x1 << 6;
      }

      namespace ctsio {
        constexpr uint32_t ctsdt_is_not_output_to_cts2 = 0 << 5;
        constexpr uint32_t ctsdt_is_output_to_cts2 = 1 << 5;

        constexpr uint32_t bit_mask = 0x1 << 5;
      }

      namespace ctsdt {
        constexpr uint32_t input_output_data_is_low_level = 0 << 4;
        constexpr uint32_t input_output_data_is_high_level = 1 << 4;

        constexpr uint32_t bit_mask = 0x1 << 4;
      }

      namespace spb2io {
        constexpr uint32_t spb2dt_is_not_output_to_txd2 = 0 << 1;
        constexpr uint32_t spb2dt_is_output_to_txd2 = 1 << 1;

        constexpr uint32_t bit_mask = 0x1 << 1;
      }

      namespace spb2dt {
        constexpr uint32_t input_output_data_is_low_level = 0 << 0;
        constexpr uint32_t input_output_data_is_high_level = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

    namespace sclsr2 {
      namespace orer {
        constexpr uint32_t overrun_error_occured = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

  }

  namespace sh {
    namespace sr {
      constexpr uint32_t md = 1 << 30;
      constexpr uint32_t rb = 1 << 29;
      constexpr uint32_t bl = 1 << 28;
      constexpr uint32_t fd = 1 << 15;
      constexpr uint32_t m = 1 << 9;
      constexpr uint32_t q = 1 << 8;
      constexpr inline uint32_t imask(uint32_t num) { return (num & 0xf) << 4; }
      constexpr uint32_t s = 1 << 1;
      constexpr uint32_t t = 1 << 0;
    }

    namespace fpscr {
      constexpr uint32_t fr = 1 << 21;
      constexpr uint32_t sz = 1 << 20;
      constexpr uint32_t pr = 1 << 19;
      constexpr uint32_t dn = 1 << 18;

      namespace cause {
        constexpr uint32_t fpu_error = 0b100000 << 12;
        constexpr uint32_t invalid_operation = 0b010000 << 12;
        constexpr uint32_t division_by_zero = 0b001000 << 12;
        constexpr uint32_t overflow = 0b000100 << 12;
        constexpr uint32_t underflow = 0b000010 << 12;
        constexpr uint32_t inexact = 0b000001 << 12;

        constexpr uint32_t bit_mask = 0x3f << 12;
      }

      namespace enabled {
        constexpr uint32_t invalid_operation = 0b10000 << 7;
        constexpr uint32_t division_by_zero = 0b01000 << 7;
        constexpr uint32_t overflow = 0b00100 << 7;
        constexpr uint32_t underflow = 0b00010 << 7;
        constexpr uint32_t inexact = 0b00001 << 7;

        constexpr uint32_t bit_mask = 0x1f << 7;
      }

      namespace flag {
        constexpr uint32_t invalid_operation = 0b10000 << 2;
        constexpr uint32_t division_by_zero = 0b01000 << 2;
        constexpr uint32_t overflow = 0b00100 << 2;
        constexpr uint32_t underflow = 0b00010 << 2;
        constexpr uint32_t inexact = 0b00001 << 2;

        constexpr uint32_t bit_mask = 0x1f << 2;
      }

      namespace rm {
        constexpr uint32_t round_to_nearest = 0b00 << 0;
        constexpr uint32_t round_to_zero = 0b01 << 0;

        constexpr uint32_t bit_mask = 0x3 << 0;
      }
    }

  }

  namespace ubc {
    namespace bamra {
      namespace bama {
        constexpr uint32_t all_bara_bits_are_included_in_break_conditions = 0b0000 << 0;
        constexpr uint32_t lower_10_bits_of_bara_are_not_included_in_break_conditions = 0b0001 << 0;
        constexpr uint32_t lower_12_bits_of_bara_are_not_included_in_break_conditions = 0b0010 << 0;
        constexpr uint32_t all_bara_bits_are_not_included_in_break_conditions = 0b0011 << 0;
        constexpr uint32_t lower_16_bits_of_bara_are_not_included_in_break_conditions = 0b1000 << 0;
        constexpr uint32_t lower_20_bits_of_bara_are_not_included_in_break_conditions = 0b1001 << 0;

        constexpr uint32_t bit_mask = 0xb << 0;
      }

      namespace basma {
        constexpr uint32_t all_basra_bits_are_included_in_break_conditions = 0 << 2;
        constexpr uint32_t no_basra_bits_are_included_in_break_conditions = 1 << 2;

        constexpr uint32_t bit_mask = 0x1 << 2;
      }
    }

    namespace bbra {
      namespace sza {
        constexpr uint32_t operand_size_is_not_included_in_break_conditions = 0b00 << 0;
        constexpr uint32_t byte_access_is_used_as_break_condition = 0b01 << 0;
        constexpr uint32_t word_access_is_used_as_break_condition = 0b10 << 0;
        constexpr uint32_t longword_access_is_used_as_break_condition = 0b11 << 0;
        constexpr uint32_t quadword_access_is_used_as_break_condition = 0b1000000 << 0;

        constexpr uint32_t bit_mask = 0x43 << 0;
      }

      namespace ida {
        constexpr uint32_t condition_comparison_is_not_performed = 0b00 << 4;
        constexpr uint32_t instruction_access_cycle_is_used_as_break_condition = 0b01 << 4;
        constexpr uint32_t operand_access_cycle_is_used_as_break_condition = 0b10 << 4;
        constexpr uint32_t instruction_access_cycle_or_operand_access_cycle_is_used_as_break_condition = 0b11 << 4;

        constexpr uint32_t bit_mask = 0x3 << 4;
      }

      namespace rwa {
        constexpr uint32_t condition_comparison_is_not_performed = 0b00 << 2;
        constexpr uint32_t read_cycle_is_used_as_break_condition = 0b01 << 2;
        constexpr uint32_t write_cycle_is_used_as_break_condition = 0b10 << 2;
        constexpr uint32_t read_cycle_or_write_cycle_is_used_as_break_condition = 0b11 << 2;

        constexpr uint32_t bit_mask = 0x3 << 2;
      }
    }

    namespace brcr {
      namespace cmfa {
        constexpr uint32_t channel_a_break_condition_is_not_matched = 0 << 15;
        constexpr uint32_t channel_a_break_condition_match_has_occured = 1 << 15;

        constexpr uint32_t bit_mask = 0x1 << 15;
      }

      namespace cmfb {
        constexpr uint32_t channel_b_break_condition_is_not_matched = 0 << 14;
        constexpr uint32_t channel_b_break_condition_match_has_occured = 1 << 14;

        constexpr uint32_t bit_mask = 0x1 << 14;
      }

      namespace pcba {
        constexpr uint32_t channel_a_pc_break_is_effected_before_instruction_execution = 0 << 10;
        constexpr uint32_t channel_a_pc_break_is_effected_after_instruction_execution = 1 << 10;

        constexpr uint32_t bit_mask = 0x1 << 10;
      }

      namespace dbeb {
        constexpr uint32_t data_bus_condition_is_not_included_in_channel_b_conditions = 0 << 7;
        constexpr uint32_t data_bus_condition_is_included_in_channel_b_conditions = 1 << 7;

        constexpr uint32_t bit_mask = 0x1 << 7;
      }

      namespace pcbb {
        constexpr uint32_t channel_b_pc_break_is_effected_before_instruction_execution = 0 << 6;
        constexpr uint32_t channel_b_pc_break_is_effected_after_instruction_execution = 1 << 6;

        constexpr uint32_t bit_mask = 0x1 << 6;
      }

      namespace seq {
        constexpr uint32_t channel_a_and_b_comparison_are_performed_as_independent_condition = 0 << 3;
        constexpr uint32_t channel_a_and_b_comparison_are_performed_as_sequential_condition = 1 << 3;

        constexpr uint32_t bit_mask = 0x1 << 3;
      }

      namespace ubde {
        constexpr uint32_t user_break_debug_function_is_not_used = 0 << 0;
        constexpr uint32_t user_break_debug_function_is_used = 1 << 0;

        constexpr uint32_t bit_mask = 0x1 << 0;
      }
    }

  }
}
