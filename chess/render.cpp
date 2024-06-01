#include "chess/bishop.hpp"
#include "chess/king.hpp"
#include "chess/knight.hpp"
#include "chess/pawn.hpp"
#include "chess/queen.hpp"
#include "chess/rook.hpp"
#include "chess/square.hpp"
#include "chess/circle.hpp"
#include "chess/cursor.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"

#include "holly/ta_parameter.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"

#include "render.hpp"

namespace render {

static inline vec3 transform(const vec3 v, float cell_x, float cell_y, float scale)
{
  float x = v.x * 28.f * scale;
  float y = -v.y * 28.f * scale;
  x += 124.f + cell_x * 56.f;
  y +=  44.f + (7 - cell_y) * 56.f;

  return {x, y, 1.f/(v.z + 10.0f)};
}

static void draw_model(vec3 const * const vertices,
		       face_v const * const faces,
		       uint32_t num_faces,
		       uint32_t base_color,
		       float cell_x,
		       float cell_y,
		       float scale,
		       bool always)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color;

  const uint32_t isp_tsp_instruction_word = (always ? isp_tsp_instruction_word::depth_compare_mode::always
					            : isp_tsp_instruction_word::depth_compare_mode::greater)
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::fog_control::no_fog;


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
      const auto vertex = transform(vertices[vertex_ix], cell_x, cell_y, scale);

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

void draw_board()
{
  for (int i = 0; i < 8 * 8; i++) {
    int x = i % 8;
    int y = i / 8;
    bool white = (x % 2 == 0) ^ (y % 2 == 0);

    constexpr uint32_t cream = 0xffebecd0;
    constexpr uint32_t green = 0xff739552;
    uint32_t color = white ? cream : green;

    draw_model(square::vertices,
	       square::faces,
	       square::num_faces,
	       color,
	       x, y,
	       1.0f,  // scale
	       true); // always

  }
}

void draw_illumination(int8_t position)
{
  auto [x, y] = chess::position_to_xy(position);

  constexpr uint32_t cream = 0xfff5f681;
  draw_model(square::vertices,
	     square::faces,
	     square::num_faces,
	     cream,
	     x, y,
	     1.0f,  // scale
	     true); // always
}

void draw_moves(chess::moves_list& moves)
{
  for (int i = 0; i < moves.length; i++) {
    chess::xy move_xy = chess::position_to_xy(moves.moves[i].to_position);

    uint32_t color = 0xff777777;
    draw_model(circle::vertices,
	       circle::faces,
	       circle::num_faces,
	       color,
	       move_xy.x, move_xy.y,
	       0.5f,  // scale
	       true); // always
  }
}

void draw_piece(int8_t type, int8_t x, int8_t y)
{
  bool white = type > 0;
  uint32_t color = white ? 0xffdddddd : 0xff444444;
  switch (__builtin_abs(type)) {
  case chess::piece_type::pawn:
    draw_model(pawn::vertices,
	       pawn::faces,
	       pawn::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  case chess::piece_type::knight:
    draw_model(knight::vertices,
	       knight::faces,
	       knight::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  case chess::piece_type::bishop:
    draw_model(bishop::vertices,
	       bishop::faces,
	       bishop::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  case chess::piece_type::rook:
    draw_model(rook::vertices,
	       rook::faces,
	       rook::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  case chess::piece_type::queen:
    draw_model(queen::vertices,
	       queen::faces,
	       queen::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  case chess::piece_type::king:
    draw_model(king::vertices,
	       king::faces,
	       king::num_faces,
	       color, x, y,
	       1.0f,  // scale
	       false); // always
    break;
  default:
    break;
  }
}

void draw_pieces(const chess::game_state& game_state)
{
  const auto& piece_list = game_state.piece_list;
  for (int i = 0; i < piece_list.length; i++) {
    chess::piece * piece = piece_list.piece[i];
    int8_t position = piece_list_to_position(game_state.board, piece);
    auto [x, y] = chess::position_to_xy(position);
    draw_piece(piece->type, x, y);
  }
}

void draw_cursor(float x, float y)
{
  uint32_t color = 0xffff00ff;
  draw_model(cursor::vertices,
	     cursor::faces,
	     cursor::num_faces,
	     color,
	     x, y,
	     1.0f,  // scale
	     true); // always
}

} // namespace render
