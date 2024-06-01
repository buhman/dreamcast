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

#include "render.hpp"
#include "input.hpp"

struct button {
  bool a;
  bool b;
};

struct cursor {
  float x;
  float y;
  struct button button[2];
};

void cursor_update(chess::game_state& game_state, struct cursor& cursor, uint32_t frame_ix)
{
  for (int port_ix = 0; port_ix < 4; port_ix++) {
    auto& port = input::state.port[port_ix];
    if (port.function_type & function_type::controller) {

    }
    if (port.function_type & function_type::pointing) {
      auto& bus_data = port.host_response_data_transfer_ft9->bus_data;
      auto& data_fields = bus_data.data_fields;
      if (std::byteswap(data_fields.function_type) & function_type::pointing) {
	auto& data = data_fields.data;
	cursor.x += static_cast<float>(data.analog_coordinate_axis[0] - 0x200) *  0.0035;
	cursor.y += static_cast<float>(data.analog_coordinate_axis[1] - 0x200) * -0.0035;
	cursor.button[frame_ix].a = ft9::data_transfer::button::a(data.button) == 0;
	cursor.button[frame_ix].b = ft9::data_transfer::button::b(data.button) == 0;
      }
    }
  }

  uint32_t last_frame = (frame_ix + 1) & 1;
  if (cursor.button[last_frame].a != cursor.button[frame_ix].a && cursor.button[frame_ix].a) {
    int8_t x = cursor.x + 0.5f;
    int8_t y = cursor.y + 0.5f;
    chess::select_position(game_state, x, y);
  }
  if (cursor.button[last_frame].b != cursor.button[frame_ix].b && cursor.button[frame_ix].b) {
    serial::string("b\n");
  }
}

void main()
{
  video_output::set_mode_vga();

  constexpr uint32_t ta_alloc = ta_alloc_ctrl::pt_opb::no_list
                              | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr struct opb_size opb_size = { .opaque = 16 * 4
                                       , .opaque_modifier = 0
                                       , .translucent = 0
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

  struct cursor cursor;
  cursor.x = 4.f;
  cursor.y = 4.f;

  while (true) {
    input::state_update(send_buf, recv_buf);
    cursor_update(game_state, cursor, frame_ix);

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    render::draw_board();
    if (game_state.interaction.selected_position != -1)
      render::draw_illumination(game_state.interaction.selected_position);
    render::draw_pieces(game_state);
    render::draw_moves(game_state.interaction.moves);
    render::draw_cursor(cursor.x, cursor.y);

    *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
      ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
    sq_transfer_32byte(ta_fifo_polygon_converter);

    ta_wait_opaque_list();

    core_start_render(frame_ix);
    core_wait_end_of_render_video();

    while (!spg_status::vsync(holly.SPG_STATUS));
    core_flip(frame_ix);
    while (spg_status::vsync(holly.SPG_STATUS));

    frame_ix = (frame_ix + 1) & 1;
  }
}
