#include <cstdint>

#include "chess.hpp"
#include "sh7091/serial.hpp"

namespace chess {

xy position_to_xy(int8_t position)
{
  int8_t y = (position >> 4) & 0xf;
  int8_t x = (position >> 0) & 0xf;
  return {x, y};
}

int8_t xy_to_position(int8_t x, int8_t y)
{
  return ((y & 0xf) << 4)
       | ((x & 0xf) << 0);
}

bool xy_inside_board(int8_t x, int8_t y)
{
  int8_t position = xy_to_position(x, y);
  return (position & 0x88) == 0;
}

int8_t piece_list_to_position(const chess::board_state& board_state,
			      chess::piece * piece)
{
  int8_t position = piece - &board_state.board[0];
  return position;
}

constexpr inline bool turn_same_side(int8_t side, int8_t type)
{
  return (type != 0) && ((side > 0) == (type > 0));
}

bool speculative_check(game_state& game_state, int8_t from_position, int8_t to_position, int8_t side);

template <int max_limit>
static move_t * moves_line(game_state& game_state, int8_t origin, move_t * moves, int dx, int dy)
{
  board_state& board = game_state.board;
  piece& self = board.board[origin];
  auto [ox, oy] = position_to_xy(origin);
  int8_t x = ox, y = oy;
  for (int limit = 0; limit < max_limit; limit++) {
    x += dx;
    y += dy;
    if (xy_inside_board(x, y)) {
      piece& other = board.board[xy_to_position(x, y)];
      bool different_sides = ((self.type ^ other.type) < 0);
      if (other.type == 0 || different_sides) {
	int8_t to_position = xy_to_position(x, y);
	if (!speculative_check(game_state, origin, to_position, self.type))
	  *moves++ = {move_type::normal, to_position};
      }
      if (other.type != 0)
	break;
    } else {
      break;
    }
  }
  return moves;
}

move_t * moves_pawn(game_state& game_state, int8_t origin, move_t * moves)
{
  board_state& board = game_state.board;
  piece& self = board.board[origin];
  int8_t direction = (self.type > 0) ? 1 : -1;

  auto [ox, oy] = position_to_xy(origin);
  int advance = 1;
  auto type = move_type::normal;
  do {
    int8_t x = ox;
    int8_t y = oy + direction * advance;
    if (xy_inside_board(x, y)) {
      piece& other = board.board[xy_to_position(x, y)];
      if (other.type == 0) {
	int8_t to_position = xy_to_position(x, y);
	if (!speculative_check(game_state, origin, to_position, self.type)) {
	  *moves++ = {type, to_position};
	}
      } else {
	break;
      }
    }
    advance += 1;
    type = move_type::pawn_double_advance;
  } while (self.movement == movement_state::not_moved && advance <= 2);

  int8_t diagonal = 1;
  for (int i = 0; i < 2; i++) {
    int8_t x = ox + diagonal;
    int8_t y = oy + direction;

    if (xy_inside_board(x, y)) {
      piece& other = board.board[xy_to_position(x, y)];
      bool different_sides = ((self.type ^ other.type) < 0);
      if (other.type != 0 && different_sides) {
	int8_t to_position = xy_to_position(x, y);
	if (!speculative_check(game_state, origin, to_position, self.type))
	  *moves++ = {move_type::normal, to_position};
      }
      if (game_state.en_passant_target != -1 && xy_to_position(x, oy) == game_state.en_passant_target) {
	// en_passant_target is always the opposite side
	int8_t to_position = xy_to_position(x, y);
	if (!speculative_check(game_state, origin, to_position, self.type))
	  *moves++ = {move_type::pawn_en_passant_capture, to_position};
      }
    }
    diagonal = -diagonal;
  }

  return moves;
}

move_t * moves_knight(game_state& game_state, int8_t origin, move_t * moves)
{
  board_state& board = game_state.board;
  piece& self = board.board[origin];
  xy delta[8] = {
    { 2, -1}, // up, left
    { 2,  1}, // up, right
    {-2, -1}, // down, left
    {-2,  1}, // down, right
    { 1, -2}, // left, up
    { 1,  2}, // right, up
    {-1, -2}, // left, down
    {-1,  2}, // right, down
  };
  auto [ox, oy] = position_to_xy(origin);
  for (int i = 0; i < 8; i++) {
    uint8_t x = ox + delta[i].x;
    uint8_t y = oy + delta[i].y;
    if (xy_inside_board(x, y)) {
      piece& other = board.board[xy_to_position(x, y)];
      bool different_sides = ((self.type ^ other.type) < 0);
      if (other.type == 0 || different_sides) {
	int8_t to_position = xy_to_position(x, y);
	if (!speculative_check(game_state, origin, to_position, self.type))
	  *moves++ = {move_type::normal, to_position};
      }
    }
  }
  return moves;
}

move_t * moves_bishop(game_state& game_state, int8_t origin, move_t * moves)
{
  // up-left
  moves = moves_line<8>(game_state, origin, moves,  1,  1);
  // up-right
  moves = moves_line<8>(game_state, origin, moves, -1,  1);
  // down-left
  moves = moves_line<8>(game_state, origin, moves,  1, -1);
  // down-right
  moves = moves_line<8>(game_state, origin, moves, -1, -1);

  return moves;
}

move_t * moves_rook(game_state& game_state, int8_t origin, move_t * moves)
{
  // up
  moves = moves_line<8>(game_state, origin, moves,  0,  1);
  // down
  moves = moves_line<8>(game_state, origin, moves,  0, -1);
  // left
  moves = moves_line<8>(game_state, origin, moves,  1,  0);
  // right
  moves = moves_line<8>(game_state, origin, moves, -1,  0);

  return moves;
}

move_t * moves_queen(game_state& game_state, int8_t origin, move_t * moves)
{
  moves = moves_bishop(game_state, origin, moves);
  moves = moves_rook(game_state, origin, moves);

  return moves;
}

move_t * moves_king(game_state& game_state, int8_t origin, move_t * moves)
{
  // up-left
  moves = moves_line<1>(game_state, origin, moves,  1,  1);
  // up-right
  moves = moves_line<1>(game_state, origin, moves, -1,  1);
  // down-left
  moves = moves_line<1>(game_state, origin, moves,  1, -1);
  // down-right
  moves = moves_line<1>(game_state, origin, moves, -1, -1);
  // up
  moves = moves_line<1>(game_state, origin, moves,  0,  1);
  // down
  moves = moves_line<1>(game_state, origin, moves,  0, -1);
  // left
  moves = moves_line<1>(game_state, origin, moves,  1,  0);
  // right
  moves = moves_line<1>(game_state, origin, moves, -1,  0);

  // castle
  board_state& board = game_state.board;
  piece& self = board.board[origin];
  if (self.movement == movement_state::not_moved && !speculative_check(game_state, origin, origin, self.type)) {
    int direction = 1;
    int distance = 2;
    move_type type = move_type::castle_short;
    auto [ox, oy] = position_to_xy(origin);
    do {
      int8_t rook_position = xy_to_position(direction == 1 ? 7 : 0, oy);
      piece& rook = board.board[rook_position];
      if (__builtin_abs(rook.type) == piece_type::rook && rook.movement == movement_state::not_moved) {
	move_t * castle_moves = moves_line<8>(game_state, origin, moves, direction, 0);
	if (castle_moves - moves == distance) {
	  int8_t to_position = xy_to_position(ox + (direction * 2), oy);
	  *moves++ = {type, to_position};
	}
      }
      distance += 1;
      direction = -direction;
      type = move_type::castle_long;
    } while (direction == -1);
  }

  return moves;
}

int moves_position(game_state& game_state, int8_t origin, move_t * moves)
{
  board_state& board = game_state.board;
  piece& self = board.board[origin];
  switch (__builtin_abs(self.type)) {
  case piece_type::pawn:   return moves_pawn(game_state, origin, moves) - moves;
  case piece_type::knight: return moves_knight(game_state, origin, moves) - moves;
  case piece_type::bishop: return moves_bishop(game_state, origin, moves) - moves;
  case piece_type::rook:   return moves_rook(game_state, origin, moves) - moves;
  case piece_type::queen:  return moves_queen(game_state, origin, moves) - moves;
  case piece_type::king:   return moves_king(game_state, origin, moves) - moves;
  default:                 return 0;
  }
}

static void board_init(game_state& game_state)
{
  using namespace piece_type;

  constexpr int8_t types[8][8] = {
    { rook  ,  knight,  bishop,  queen ,  king  ,  bishop,  knight,  rook  }, // white
    { pawn  ,  pawn  ,  pawn  ,  pawn  ,  pawn  ,  pawn  ,  pawn  ,  pawn  }, // white
    { empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty },
    { empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty },
    { empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty },
    { empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty ,  empty },
    {-pawn  , -pawn  , -pawn  , -pawn  , -pawn  , -pawn  , -pawn  , -pawn  }, // black
    {-rook  , -knight, -bishop, -queen , -king  , -bishop, -knight, -rook  }, // black
  };

  game_state.piece_list.length = 0;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int8_t type = types[y][x];
      int8_t position = xy_to_position(x, y);
      piece& p = game_state.board.board[position];
      p.type = type;
      p.movement = movement_state::not_moved;
      p.piece_list_offset = game_state.piece_list.length;
      switch (__builtin_abs(type)) {
      case empty: break;
      default:
	game_state.piece_list.piece[game_state.piece_list.length++] = &p;
	break;
      }
    }
  }
}

void game_init(game_state& game_state)
{
  game_state.turn = 1; // white
  game_state.en_passant_target = -1;
  game_state.halfmove_number = 0;
  game_state.fullmove_number = 0;
  game_state.interaction.selected_position = -1;
  game_state.interaction.last_move.from_position = -1;
  game_state.interaction.last_move.to_position = -1;
  board_init(game_state);
}

int position_in_moves(moves_list& moves, int8_t position)
{
  for (int i = 0; i < moves.length; i++) {
    int8_t move_position = moves.moves[i].to_position;
    if (move_position == position) {
      return i;
    }
  }
  return -1;
}

void piece_list_delete(piece_list& list, int8_t offset)
{
  for (int i = offset + 1; i < list.length; i++) {
    list.piece[i]->piece_list_offset = i - 1;
    list.piece[i - 1] = list.piece[i];
  }
  list.length -= 1;
}

static int _speculative_check_depth = 0;

bool speculative_check(game_state& game_state, int8_t from_position, int8_t to_position, int8_t side)
{
  if (_speculative_check_depth > 0)
    return false;
  _speculative_check_depth++;

  piece& origin = game_state.board.board[from_position];
  piece& destination = game_state.board.board[to_position];
  const int8_t origin_type = origin.type;
  const int8_t destination_type = destination.type;

  origin.type = piece_type::empty;
  destination.type = origin_type;

  bool check = false;

  for (int i = 0; i < game_state.piece_list.length; i++) {
    if (i == origin.piece_list_offset) {
      continue;
    }
    piece * test_piece = game_state.piece_list.piece[i];
    if (turn_same_side(side, test_piece->type)) {
      continue;
    }
    int8_t position = piece_list_to_position(game_state.board, test_piece);
    moves_list ml;
    ml.length = moves_position(game_state, position, ml.moves);
    for (int j = 0; j < ml.length; j++) {
      int8_t move_position = ml.moves[j].to_position;
      piece& move_piece = game_state.board.board[move_position];
      if (__builtin_abs(move_piece.type) == piece_type::king) {
	serial::string("check\n");
	check = true;
	goto exit;
      }
    }
  }

 exit:
  origin.type = origin_type;
  destination.type = destination_type;

  _speculative_check_depth--;

  return check;
}

void do_move(game_state& game_state, int8_t from_position, move_t& move)
{
  piece& origin = game_state.board.board[from_position];
  piece& destination = game_state.board.board[move.to_position];

  if (destination.type != 0) { // capture
    piece_list_delete(game_state.piece_list, destination.piece_list_offset);
  }

  int8_t en_passant_target = game_state.en_passant_target;
  game_state.en_passant_target = -1;

  switch (move.type) {
  case move_type::pawn_double_advance:
    serial::string("pawn double advance\n");
    game_state.en_passant_target = move.to_position;
    break;
  case move_type::castle_short:
    serial::string("move castle short\n");
    {
      auto [ox, oy] = position_to_xy(move.to_position);
      int8_t rook_from = xy_to_position(7, oy);
      move_t rook_move = {move_type::normal, xy_to_position(5, oy)};
      do_move(game_state, rook_from, rook_move);
    }
    break;
  case move_type::castle_long:
    serial::string("move castle long\n");
    {
      auto [ox, oy] = position_to_xy(move.to_position);
      int8_t rook_from = xy_to_position(0, oy);
      move_t rook_move = {move_type::normal, xy_to_position(3, oy)};
      do_move(game_state, rook_from, rook_move);
    }
    break;
  case move_type::pawn_en_passant_capture:
    {
      serial::string("move en passant capture\n");
      piece& en_passant_piece = game_state.board.board[en_passant_target];
      serial::integer<uint8_t>(en_passant_target);
      serial::integer<uint8_t>(en_passant_piece.type);
      en_passant_piece.type = piece_type::empty;
      piece_list_delete(game_state.piece_list, en_passant_piece.piece_list_offset);
    }
    break;
  default: break;
  }

  destination.type = origin.type;
  destination.movement = true;
  destination.piece_list_offset = origin.piece_list_offset;
  origin.type = piece_type::empty;
  game_state.piece_list.piece[origin.piece_list_offset] = &destination;
  game_state.interaction.last_move.from_position = from_position;
  game_state.interaction.last_move.to_position = move.to_position;

  game_state.turn = -game_state.turn;
}

void select_position(game_state& game_state, int8_t x, int8_t y)
{
  int8_t position = xy_to_position(x, y);

  if (turn_same_side(game_state.turn, game_state.board.board[position].type)) {
    if (game_state.interaction.selected_position != position) {
      game_state.interaction.selected_position = position;
      game_state.interaction.moves.length = moves_position(game_state,
							   position,
							   game_state.interaction.moves.moves);
      return;
    }
  } else { // not same side or empty
    if (game_state.interaction.selected_position != -1) {
      int moves_ix = position_in_moves(game_state.interaction.moves, position);
      if (moves_ix >= 0) {
	do_move(game_state,
		game_state.interaction.selected_position, // from
		game_state.interaction.moves.moves[moves_ix]); // to
	// fall through to deselect
      }
    }
  }

  game_state.interaction.selected_position = -1;
  game_state.interaction.moves.length = 0;
}

void clear_annotations(game_state& game_state)
{
  game_state.interaction.annotation_list.length = 0;
}

void delete_annotation(annotation_list& annotation_list, int i)
{
  annotation_list.length -= 1;
  for (int j = i; j < annotation_list.length; j++) {
    annotation_list.annotation[j] = annotation_list.annotation[j + 1];
  }
}

void annotate_position(game_state& game_state, int8_t x, int8_t y)
{
  int8_t position = xy_to_position(x, y);
  auto& annotation_list = game_state.interaction.annotation_list;

  for (int i = 0; i < annotation_list.length; i++) {
    if (annotation_list.annotation[i].from_position == position) {
      delete_annotation(annotation_list, i);
      return;
    }
  }

  annotation_list.annotation[annotation_list.length++].from_position = position;
}

}
