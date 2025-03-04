#include <bit>

#include "maple/maple_bus_ft6_key_scan_codes.hpp"
#include "maple/maple_bus_commands.hpp"
#include "maple/maple_bus_bits.hpp"
#include "maple/maple_host_command_writer.hpp"

#include "sh7091/serial.hpp"

#include "keyboard.hpp"

void do_device_request()
{
  static uint8_t send_buf[1024] __attribute__((aligned(32)));
  static uint8_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  using command_type = maple::device_request;
  using response_type = maple::device_status;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>();

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  for (uint8_t port = 0; port < 4; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != response_type::command_code) {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  disconnected\n");
    } else {
      serial::string("port: ");
      serial::integer<uint8_t>(port);
      serial::string("  ft:    ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.ft));
      serial::string("  fd[0]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[0]));
      serial::string("  fd[1]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[1]));
      serial::string("  fd[2]: ");
      serial::integer<uint32_t>(std::byteswap(data_fields.device_id.fd[2]));
      serial::string("  source_ap.lm_bus: ");
      serial::integer<uint8_t>(bus_data.source_ap & ap::lm_bus::bit_mask);
    }
  }
}

void keyboard_do_get_condition(ft6::data_transfer::data_format& data)
{
  static uint8_t send_buf[1024] __attribute__((aligned(32)));
  static uint8_t recv_buf[1024] __attribute__((aligned(32)));

  auto writer = maple::host_command_writer(send_buf, recv_buf);

  {
    using command_type = maple::device_request;
    using response_type = maple::device_status;

    writer.append_command_all_ports<command_type, response_type>(false);
  }

  using command_type = maple::get_condition;
  using response_type = maple::data_transfer<ft6::data_transfer::data_format>;

  auto [host_command, host_response]
    = writer.append_command_all_ports<command_type, response_type>(true);

  host_command->bus_data.data_fields.function_type = std::byteswap(function_type::keyboard);

  maple::dma_start(send_buf, writer.send_offset,
                   recv_buf, writer.recv_offset);
  maple::dma_wait_complete();

  for (uint8_t port = 0; port < 1; port++) {
    auto& bus_data = host_response[port].bus_data;
    auto& data_fields = bus_data.data_fields;
    if (bus_data.command_code != response_type::command_code) {
      serial::integer<uint32_t>(bus_data.command_code);
      serial::integer<uint32_t>(data_fields.function_type);
      continue;
    }
    if ((std::byteswap(data_fields.function_type) & function_type::keyboard) == 0) {
      continue;
    }

    data.modifier_key = data_fields.data.modifier_key;
    data.led_state = data_fields.data.led_state;
    data.scan_code_array[0] = data_fields.data.scan_code_array[0];
    data.scan_code_array[1] = data_fields.data.scan_code_array[1];
    data.scan_code_array[2] = data_fields.data.scan_code_array[2];
    data.scan_code_array[3] = data_fields.data.scan_code_array[3];
    data.scan_code_array[4] = data_fields.data.scan_code_array[4];
    data.scan_code_array[5] = data_fields.data.scan_code_array[5];

    return;
  }
}

void keyboard_debug(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix)
{
  uint32_t this_frame = (frame_ix + 0) & 1;
  bool difference = false;
  for (int i = 0; i < 6; i++) {
    if (keyboards[0].scan_code_array[i] != keyboards[1].scan_code_array[i]) {
      difference = true;
      break;
    }
  }
  if (difference) {
    for (int i = 0; i < 6; i++) {
      //serial::integer<uint8_t>(keyboards[this_frame].scan_code_array[i], ' ');
    }
    //serial::string("\n");
  }
}

constexpr inline bool is_modified(uint32_t modifier, uint32_t bit)
{
  uint32_t mask = ~(bit);
  return ((modifier & bit) != 0) && ((modifier & mask) == 0);
}

constexpr inline bool is_unmodified(uint32_t modifier)
{
  return modifier == 0;
}

constexpr inline bool is_shifted(uint32_t modifier)
{
  uint32_t bit = ft6::data_transfer::modifier_key::right_shift() | ft6::data_transfer::modifier_key::left_shift();
  return is_modified(modifier, bit);
}

constexpr inline bool is_left_alted(uint32_t modifier)
{
  uint32_t bit = (ft6::data_transfer::modifier_key::left_alt());
  return is_modified(modifier, bit);
}

constexpr inline bool is_ctrled(uint32_t modifier)
{
  uint32_t bit = (ft6::data_transfer::modifier_key::left_control() | ft6::data_transfer::modifier_key::right_control());
  return is_modified(modifier, bit);
}

void keyboard_update(ft6::data_transfer::data_format * keyboards, uint32_t frame_ix, gap_buffer& gb, viewport_window& vw)
{
  uint32_t this_frame = (frame_ix + 0) & 1;
  uint32_t next_frame = (frame_ix + 1) & 1;
  for (int i = 0; i < 6; i++) {
    if (keyboards[this_frame].scan_code_array[i] == ft6::scan_code::no_operation)
      break;
    if (i < 5 && keyboards[this_frame].scan_code_array[i + 1] != ft6::scan_code::no_operation)
      continue;
    bool make = true;
    for (int j = 0; j < 6; j++) {
      if (keyboards[next_frame].scan_code_array[j] == keyboards[this_frame].scan_code_array[i]) {
	make = false;
	break;
      }
    }
    if (make) {
      // make
      int modifier_key = keyboards[this_frame].modifier_key;
      uint8_t scan_code = keyboards[this_frame].scan_code_array[i];

      if (is_shifted(modifier_key) || is_unmodified(modifier_key)) {
        if (scan_code >= ft6::scan_code::first_printable && scan_code <= ft6::scan_code::last_printable) {
          bool shifted = is_shifted(modifier_key);
          char_type code_point = ft6::scan_code::code_point[scan_code][shifted];
          if (code_point != 0) {
            gap_append(gb, code_point);
            continue;
          }
        }
      }

      if (is_unmodified(modifier_key)) {
        switch (scan_code) {
        case ft6::scan_code::_return:   gap_append(gb, '\n'); break;
        case ft6::scan_code::backspace: gap_pop(gb);          break;
        case ft6::scan_code::spacebar:  gap_append(gb, ' ');  break;

        case ft6::scan_code::left_arrow:  gap_cursor_pos(gb, -1);      break;
        case ft6::scan_code::right_arrow: gap_cursor_pos(gb, 1);       break;
        case ft6::scan_code::up_arrow:    gap_cursor_pos_line(gb, -1); break;
        case ft6::scan_code::down_arrow:  gap_cursor_pos_line(gb, 1);  break;

        default: break;
        }
      }

      if (is_left_alted(modifier_key)) {
        switch (scan_code) {
        case ft6::scan_code::v_V:
          vw.first_line -= 1;
          if (vw.first_line < 0)
            vw.first_line = 0;
          break;
        default: break;
        }
      }

      if (is_ctrled(modifier_key)) {
        switch (scan_code) {
        case ft6::scan_code::v_V:
          vw.first_line += 1;
          if ((vw.first_line - 1) >= gb.line.length) {
            vw.first_line = gb.line.length;
          }
          break;
        default: break;
        }
      }
    }
  }
}
