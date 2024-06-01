#pragma once

#include <cstdint>

#include "chess.hpp"

namespace render {

void draw_board();
void draw_illumination(int8_t position);
void draw_moves(chess::moves_list& moves);
void draw_pieces(const chess::game_state& game_state);
void draw_cursor(float x, float y);

}
