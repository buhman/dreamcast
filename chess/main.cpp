#include <bit>

#include "memorymap.hpp"
#include "twiddle.hpp"

#include "holly/texture_memory_alloc.hpp"
#include "holly/holly.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/video_output.hpp"

#include "maple/maple_bus_bits.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "chess/chess.hpp"
#include "chess/render.hpp"
#include "chess/input.hpp"

void cursor_update(struct render::cursor_state& cursor_state, uint32_t frame_ix)
{
  for (int port_ix = 0; port_ix < 4; port_ix++) {
    int cursor_ix = cursor_state.port_map[port_ix];
    if (cursor_ix == -1)
      continue;
    auto& cursor = cursor_state.cur[cursor_ix];

    auto& port = input::state.port[port_ix];
    if (port.function_type & function_type::controller) {
      auto& bus_data = port.host_response_data_transfer_ft0->bus_data;
      auto& data_fields = bus_data.data_fields;
      if (std::byteswap(data_fields.function_type) & function_type::controller) {
	auto& data = data_fields.data;
	cursor.x += static_cast<float>(data.analog_coordinate_axis[2] - 0x80) *  0.0035;
	cursor.y += static_cast<float>(data.analog_coordinate_axis[3] - 0x80) * -0.0035;
	cursor.button[frame_ix].a = ft0::data_transfer::digital_button::a(data.digital_button) == 0;
	cursor.button[frame_ix].b = ft0::data_transfer::digital_button::b(data.digital_button) == 0;
      }
    }
    if (port.function_type & function_type::pointing) {
      auto& bus_data = port.host_response_data_transfer_ft9->bus_data;
      auto& data_fields = bus_data.data_fields;
      if (std::byteswap(data_fields.function_type) & function_type::pointing) {
	auto& data = data_fields.data;
	cursor.x += static_cast<float>(data.analog_coordinate_axis[0] - 0x200) *  0.0035;
	cursor.y += static_cast<float>(data.analog_coordinate_axis[1] - 0x200) * -0.0035;
	cursor.button[frame_ix].a = ft9::data_transfer::digital_button::a(data.digital_button) == 0;
	cursor.button[frame_ix].b = ft9::data_transfer::digital_button::b(data.digital_button) == 0;
      }
    }
  }
}

void cursor_events(chess::game_state& game_state, struct render::cursor_state& cursor_state, uint32_t frame_ix)
{
  for (int cursor_ix = 0; cursor_ix < render::cursor_state::num_cursors; cursor_ix++) {
    auto& cursor = cursor_state.cur[cursor_ix];
    const uint32_t last_frame = (frame_ix + 1) & 1;
    const int8_t x = cursor.x + 0.5f;
    const int8_t y = cursor.y + 0.5f;
    if (cursor.button[last_frame].a != cursor.button[frame_ix].a && cursor.button[frame_ix].a) {
      chess::clear_annotations(game_state);
      chess::select_position(game_state, x, y);
    }
    if (cursor.button[last_frame].b != cursor.button[frame_ix].b && cursor.button[frame_ix].b) {
      chess::annotate_position(game_state, x, y);
    }
  }
}

void main()
{
  video_output::set_mode_vga();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::_16x4byte
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::no_list;

  constexpr struct opb_size opb_size = { .opaque = 0
                                       , .opaque_modifier = 0
                                       , .translucent = 16 * 4
                                       , .translucent_modifier = 0
                                       , .punch_through = 0
                                       };

  holly.SOFTRESET = softreset::pipeline_soft_reset
                  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();
  region_array2(640 / 32, 480 / 32, opb_size);
  background_parameter(0xff0000ff);

  chess::game_state game_state;
  chess::game_init(game_state);

  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  input::state_init();

  uint32_t frame_ix = 0;

  struct render::view_transform vt;
  vt.piece_rotation = 0.f;
  vt.board_rotation = 0.f;

  struct render::cursor_state cursor_state = render::cursor_state();

  while (true) {
    input::state_update(send_buf, recv_buf);
    cursor_update(cursor_state, frame_ix);
    cursor_events(game_state, cursor_state, frame_ix);

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    render::render(game_state, vt, cursor_state);

    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    ta_wait_translucent_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
