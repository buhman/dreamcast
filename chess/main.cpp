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

    float invert = port_ix % 2 == 0 ? 1.f : -1.f;

    auto& port = input::state.port[port_ix];
    if (port.function_type & function_type::controller) {
      auto& bus_data = port.host_response_data_transfer_ft0->bus_data;
      auto& data_fields = bus_data.data_fields;
      if (std::byteswap(data_fields.function_type) & function_type::controller) {
	auto& data = data_fields.data;
	cursor.x += static_cast<float>(data.analog_coordinate_axis[2] - 0x80) *  0.0015 * invert;
	cursor.y += static_cast<float>(data.analog_coordinate_axis[3] - 0x80) * -0.0015 * invert;
	cursor.button[frame_ix].a = ft0::data_transfer::digital_button::a(data.digital_button) == 0;
	cursor.button[frame_ix].b = ft0::data_transfer::digital_button::b(data.digital_button) == 0;
	cursor.button[frame_ix].x = ft0::data_transfer::digital_button::x(data.digital_button) == 0;
	cursor.button[frame_ix].y = ft0::data_transfer::digital_button::y(data.digital_button) == 0;
	bool start = ft0::data_transfer::digital_button::start(data.digital_button) == 0;
	if (start) {
	  cursor.x = 3.5f;
	  cursor.y = 3.5f;
	}
      }
    }
    if (port.function_type & function_type::pointing) {
      auto& bus_data = port.host_response_data_transfer_ft9->bus_data;
      auto& data_fields = bus_data.data_fields;
      if (std::byteswap(data_fields.function_type) & function_type::pointing) {
	auto& data = data_fields.data;
	cursor.x += static_cast<float>(data.analog_coordinate_axis[0] - 0x200) *  0.0065 * invert;
	cursor.y += static_cast<float>(data.analog_coordinate_axis[1] - 0x200) * -0.0065 * invert;
	cursor.button[frame_ix].a = ft9::data_transfer::digital_button::a(data.digital_button) == 0;
	cursor.button[frame_ix].b = ft9::data_transfer::digital_button::b(data.digital_button) == 0;
	bool w = ft9::data_transfer::digital_button::w(data.digital_button) == 0;
	if (w) {
	  cursor.x = 3.5f;
	  cursor.y = 3.5f;
	}
      }
    }

    if (cursor.x < -2.0f) cursor.x = -2.0f;
    if (cursor.x >  9.0f) cursor.x =  9.0f;
    if (cursor.y < -0.5f) cursor.y = -0.5f;
    if (cursor.y >  7.5f) cursor.y =  7.5f;
  }
}

static bool piece_rotation = false;
static bool board_rotation = false;

void promotion_select(chess::game_state& game_state,
		      int side,
		      int promotion_ix)
{
  if (promotion_ix < 0 || promotion_ix >= 4) {
    return;
  }
  game_state.interaction.promotion_ix[side] = promotion_ix;
}

void cursor_events(chess::game_state& game_state, struct render::cursor_state& cursor_state, uint32_t frame_ix)
{
  for (int cursor_ix = 0; cursor_ix < render::cursor_state::num_cursors; cursor_ix++) {
    auto& cursor = cursor_state.cur[cursor_ix];
    const uint32_t last_frame = (frame_ix + 1) & 1;
    const float x = cursor.x + 0.5f;
    const int8_t y = cursor.y + 0.5f;
    if (cursor.button[last_frame].a != cursor.button[frame_ix].a && cursor.button[frame_ix].a) {
      chess::clear_annotations(game_state);
      if (x > 8.5) {
	serial::string("white side\n");
	int8_t promotion_ix = y;
	promotion_select(game_state, 0, promotion_ix);
      } else if (x < -0.5) {
	serial::string("black side\n");
	int8_t promotion_ix = 7 - y;
	promotion_select(game_state, 1, promotion_ix);
      } else {
	chess::select_position(game_state, x, y);
      }
    }
    if (cursor.button[last_frame].b != cursor.button[frame_ix].b && cursor.button[frame_ix].b) {
      chess::annotate_position(game_state, x, y);
    }
    if (cursor.button[last_frame].x != cursor.button[frame_ix].x && cursor.button[frame_ix].x) {
      piece_rotation = !piece_rotation;
    }
    if (cursor.button[last_frame].y != cursor.button[frame_ix].y && cursor.button[frame_ix].y) {
      board_rotation = !board_rotation;
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
  background_parameter(0xff223311);

  chess::game_state game_state;
  chess::game_init(game_state);

  uint32_t send_buf[1024] __attribute__((aligned(32)));
  uint32_t recv_buf[1024] __attribute__((aligned(32)));

  input::state_init();

  uint32_t frame_ix = 0;

  struct render::view_transform vt;
  vt.piece_rotation = 0.f;
  vt.board_rotation = 0.f;

  auto piece_rotation_animator = render::animator<float>(0.f);
  auto board_rotation_animator = render::animator<float>(0.f);

  struct render::cursor_state cursor_state = render::cursor_state();

  constexpr float pi = 3.141592653589793f;
  piece_rotation = false;
  board_rotation = false;

  while (true) {
    input::state_update(send_buf, recv_buf);
    cursor_update(cursor_state, frame_ix);
    cursor_events(game_state, cursor_state, frame_ix);

    //float target = game_state.turn == 1 ? 0.f : pi;
    piece_rotation_animator.set_target(piece_rotation ? pi : 0.f, 60);
    board_rotation_animator.set_target(board_rotation ? pi : 0.f, 60);
    vt.board_rotation = board_rotation_animator.interpolate();
    vt.piece_rotation = piece_rotation_animator.interpolate();

    ta_polygon_converter_init(opb_size.total(),
                              ta_alloc,
                              640 / 32,
                              480 / 32);

    render::render(vt, game_state, cursor_state);

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
