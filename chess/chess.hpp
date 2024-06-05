#pragma once

#include <cstdint>

namespace chess {

namespace piece_type {
constexpr int8_t empty = 0;
constexpr int8_t pawn = 1;
constexpr int8_t knight = 2;
constexpr int8_t bishop = 3;
constexpr int8_t rook = 4;
constexpr int8_t queen = 5;
constexpr int8_t king = 6;
};

constexpr int8_t promotion_types[4] = {
  piece_type::knight,
  piece_type::bishop,
  piece_type::rook,
  piece_type::queen,
};

namespace movement_state {
constexpr int8_t not_moved = 0;
constexpr int8_t moved = 1;
}

struct xy {
  int8_t x;
  int8_t y;
};

struct piece {
  int8_t type;      // piece_type
  int8_t movement;  // movement_state
  int8_t piece_list_offset;
};

struct piece_list {
  struct piece * piece[8 * 8];
  int32_t length;
};

struct board_state {
  struct piece board[8 * 16];
};

enum struct move_type : int8_t {
  normal,
  castle_short,
  castle_long,
  pawn_double_advance,
  pawn_en_passant_capture,
  pawn_promote,
};

struct move_t {
  enum move_type type;
  int8_t to_position;
};

struct moves_list {
  move_t moves[27];
  int8_t length;
};

enum struct annotation_type : int8_t {
  highlight,
  arrow,
};

struct annotation {
  annotation_type type;
  int8_t from_position;
  int8_t to_position;
};

struct annotation_list {
  struct annotation annotation[127];
  int8_t length;
};

struct interaction_state {
  struct {
    int8_t from_position;
    int8_t to_position;
  } last_move;
  int8_t selected_position;
  struct moves_list moves;
  struct annotation_list annotation_list;
  int8_t promotion_ix[2];
};

struct game_state {
  struct board_state board;
  struct piece_list piece_list;
  struct interaction_state interaction;
  int8_t turn;
  int8_t en_passant_target; // index into board_state.board
  uint16_t halfmove_number;
  uint16_t fullmove_number;
};

xy position_to_xy(int8_t position);
int8_t xy_to_position(int8_t x, int8_t y);
int8_t piece_list_to_position(const chess::board_state& board_state,
			      chess::piece * piece);
void game_init(game_state& game_state);
void select_position(game_state& game_state, int8_t x, int8_t y);
void annotate_position(game_state& game_state, int8_t x, int8_t y);
void clear_annotations(game_state& game_state);

}
