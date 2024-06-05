#include "chess/bishop.hpp"
#include "chess/king.hpp"
#include "chess/knight.hpp"
#include "chess/pawn.hpp"
#include "chess/queen.hpp"
#include "chess/rook.hpp"
#include "chess/square.hpp"
#include "chess/circle.hpp"
#include "chess/pointer.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "holly/ta_parameter.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"

#include "chess/chess.hpp"
#include "chess/render.hpp"

namespace render {

constexpr float board_z = 100.f;
constexpr float illumination_z = 99.f;
constexpr float piece_z = 70.f;
constexpr float move_z = 50.f;
constexpr float cursor_z = 0.f;

vec3 __attribute__ ((noinline))
transform(view_transform vt,
	  const vec3 v,
	  float cell_x,
	  float cell_y,
	  float z_offset,
	  float scale,
	  float rotation)
{
  float x0 = v.x;
  float y0 = -v.y;

  // piece rotation
  float x1 = x0 * __builtin_cosf(rotation) - y0 * __builtin_sinf(rotation);
  float y1 = x0 * __builtin_sinf(rotation) + y0 * __builtin_cosf(rotation);

  float x2 = x1 * 28.f * scale;
  float y2 = y1 * 28.f * scale;

  float x3 = x2 + (cell_x    ) * 56.f - (196.f);
  float y3 = y2 + (7 - cell_y) * 56.f - (196.f);

  float x4 = x3 * __builtin_cosf(vt.board_rotation) - y3 * __builtin_sinf(vt.board_rotation);
  float y4 = x3 * __builtin_sinf(vt.board_rotation) + y3 * __builtin_cosf(vt.board_rotation);

  float x5 = x4 + 124.f + (196.f);
  float y5 = y4 +  44.f + (196.f);

  float z = v.z + z_offset;

  return {x5, y5, 1.f/(z + 10.0f)};
}

static void draw_model(const view_transform vt,
		       vec3 const * const vertices,
		       face_v const * const faces,
		       uint32_t num_faces,
		       uint32_t base_color,
		       float cell_x,
		       float cell_y,
		       float z_offset,
		       float scale,
		       float rotation,
		       bool always,
		       bool alpha = false)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::translucent
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = (always ? isp_tsp_instruction_word::depth_compare_mode::greater
					            : isp_tsp_instruction_word::depth_compare_mode::greater)
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = (alpha ? tsp_instruction_word::src_alpha_instr::src_alpha
 					       : tsp_instruction_word::src_alpha_instr::one)
                                      | (alpha ? tsp_instruction_word::dst_alpha_instr::inverse_src_alpha
					       : tsp_instruction_word::dst_alpha_instr::zero)
                                      | tsp_instruction_word::fog_control::no_fog
				      | tsp_instruction_word::use_alpha;

  for (uint32_t face_ix = 0; face_ix < num_faces; face_ix++) {

    *reinterpret_cast<ta_global_parameter::polygon_type_0 *>(store_queue) =
      ta_global_parameter::polygon_type_0(parameter_control_word,
					  isp_tsp_instruction_word,
					  tsp_instruction_word,
					  0, // texture_control_word
					  0, // data_size_for_sort_dma
					  0  // next_address_for_sort_dma
					  );
    sq_transfer_32byte(ta_fifo_polygon_converter);

    auto& face = faces[face_ix];

    constexpr uint32_t strip_length = 3;
    for (uint32_t i = 0; i < strip_length; i++) {
      uint32_t vertex_ix = face[i].vertex;
      const auto vertex = transform(vt,
				    vertices[vertex_ix],
				    cell_x,
				    cell_y,
				    z_offset,
				    scale,
				    rotation);

      bool end_of_strip = i == strip_length - 1;
      float z = __builtin_fabsf(vertices[vertex_ix].z);
      uint32_t color = (z < 0.9f && z > 0.1f) ? 0xff000000 : base_color;
      *reinterpret_cast<ta_vertex_parameter::polygon_type_0 *>(store_queue) =
	ta_vertex_parameter::polygon_type_0(polygon_vertex_parameter_control_word(end_of_strip),
					    vertex.x, vertex.y, vertex.z,
					    color);
      sq_transfer_32byte(ta_fifo_polygon_converter);
    }
  }
}

void draw_board(const view_transform vt)
{
  for (int i = 0; i < 8 * 8; i++) {
    int x = i % 8;
    int y = i / 8;
    bool white = (x % 2 == 0) ^ (y % 2 == 0);

    constexpr uint32_t cream = 0xffebecd0;
    constexpr uint32_t green = 0xff739552;
    uint32_t color = white ? cream : green;

    draw_model(vt,
	       square::vertices,
	       square::faces,
	       square::num_faces,
	       color,
	       x, y,
	       board_z,
	       1.0f,  // scale
	       0.0f,  // rotation
	       true); // always

  }
}

void draw_illumination(const view_transform vt,
		       int8_t position, uint32_t highlight_color)
{
  auto [x, y] = chess::position_to_xy(position);

  draw_model(vt,
	     square::vertices,
	     square::faces,
	     square::num_faces,
	     highlight_color,
	     x, y,
	     illumination_z,
	     1.0f,  // scale
	     0.0f,  // rotation
	     true,  // always
	     true); // alpha
}

void draw_moves(const view_transform vt,
		const chess::moves_list& moves)
{
  for (int i = 0; i < moves.length; i++) {
    chess::xy move_xy = chess::position_to_xy(moves.moves[i].to_position);

    uint32_t color = 0x37000000;
    draw_model(vt,
	       circle::vertices,
	       circle::faces,
	       circle::num_faces,
	       color,
	       move_xy.x, move_xy.y,
	       move_z,
	       0.4f,  // scale
	       0.0f,  // rotation
	       true,  // always
	       true); // alpha
  }
}

void draw_piece(const view_transform vt,
		int8_t type, float x, float y)
{
  bool white = type > 0;
  uint32_t color = white ? 0xffdddddd : 0xff444444;
  switch (__builtin_abs(type)) {
  case chess::piece_type::pawn:
    draw_model(vt,
	       pawn::vertices,
	       pawn::faces,
	       pawn::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  case chess::piece_type::knight:
    draw_model(vt,
	       knight::vertices,
	       knight::faces,
	       knight::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  case chess::piece_type::bishop:
    draw_model(vt,
	       bishop::vertices,
	       bishop::faces,
	       bishop::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  case chess::piece_type::rook:
    draw_model(vt,
	       rook::vertices,
	       rook::faces,
	       rook::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  case chess::piece_type::queen:
    draw_model(vt,
	       queen::vertices,
	       queen::faces,
	       queen::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  case chess::piece_type::king:
    draw_model(vt,
	       king::vertices,
	       king::faces,
	       king::num_faces,
	       color, x, y,
	       piece_z,
	       1.0f,  // scale
	       vt.piece_rotation,  // rotation
	       false); // always
    break;
  default:
    break;
  }
}

void draw_pieces(const view_transform vt,
		 const chess::game_state& game_state)
{
  const auto& piece_list = game_state.piece_list;
  for (int i = 0; i < piece_list.length; i++) {
    chess::piece * piece = piece_list.piece[i];
    int8_t position = piece_list_to_position(game_state.board, piece);
    auto [x, y] = chess::position_to_xy(position);
    draw_piece(vt,
	       piece->type,
	       x, y);
  }
}

uint32_t cursor_colors[4] = {
  0xffff00ff,
  0xff0000ff,
  0xff00ff00,
  0xffff0000,
};

void draw_cursor(const view_transform vt,
		 float x, float y,
		 int cursor_index)
{
  constexpr float pi = 3.141592653589793f;

  draw_model(vt,
	     pointer::vertices,
	     pointer::faces,
	     pointer::num_faces,
	     cursor_colors[cursor_index % 4],
	     x, y,
	     cursor_z,
	     1.0f,  // scale
	     pi * cursor_index, // rotation
	     true); // always
}


void draw_annotation(const view_transform vt,
		     const chess::annotation_list& annotation_list)
{
  constexpr uint32_t red_highlight = 0xcceb6150;

  for (int i = 0; i < annotation_list.length; i++) {
    auto& annotation = annotation_list.annotation[i];
    switch (annotation.type) {
    case chess::annotation_type::highlight:
      render::draw_illumination(vt,
				annotation.from_position,
				red_highlight);
      break;
    default: break;
    }
  }
}

void draw_interaction(const view_transform vt,
		      const chess::interaction_state& interaction)
{
  constexpr uint32_t yellow_highlight = 0x7fffff33;

  if (interaction.selected_position != -1)
    render::draw_illumination(vt, interaction.selected_position, yellow_highlight);
  if (interaction.last_move.from_position != -1)
    render::draw_illumination(vt, interaction.last_move.from_position, yellow_highlight);
  if (interaction.last_move.to_position != -1)
    render::draw_illumination(vt, interaction.last_move.to_position, yellow_highlight);
}

void draw_promotion_selection(const view_transform vt,
			      const chess::game_state& game_state)
{
  constexpr uint32_t yellow_highlight = 0x7fffff33;

  draw_model(vt,
	     square::vertices,
	     square::faces,
	     square::num_faces,
	     yellow_highlight,
	     8.5f, game_state.interaction.promotion_ix[0],
	     illumination_z,
	     1.0f,  // scale
	     0.0f,  // rotation
	     true,  // always
	     true); // alpha

  for (int i = 0; i < 4; i++) {
    draw_piece(vt,
	       chess::promotion_types[i],
	       8.5f, i);
  }

  draw_model(vt,
	     square::vertices,
	     square::faces,
	     square::num_faces,
	     yellow_highlight,
	     -1.5f, 7 - game_state.interaction.promotion_ix[1],
	     illumination_z,
	     1.0f,  // scale
	     0.0f,  // rotation
	     true,  // always
	     true); // alpha

  for (int i = 0; i < 4; i++) {
    draw_piece(vt,
	       -chess::promotion_types[i],
	       -1.5f, 7 - i);
  }
}

void render(const view_transform vt,
	    const chess::game_state& game_state,
	    const cursor_state cursor_state)
{
  render::draw_board(vt);

  render::draw_interaction(vt, game_state.interaction);
  render::draw_annotation(vt, game_state.interaction.annotation_list);

  render::draw_pieces(vt, game_state);
  render::draw_promotion_selection(vt, game_state);

  render::draw_moves(vt, game_state.interaction.moves);
  for (int i = 0; i < cursor_state::num_cursors; i++) {
    auto& cursor = cursor_state.cur[i];
    render::draw_cursor(vt, cursor.x, cursor.y, i);
  }
}

} // namespace render
