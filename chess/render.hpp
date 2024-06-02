#pragma once

#include <cstdint>

#include "chess/chess.hpp"

namespace render {

struct view_transform {
  float piece_rotation;
  float board_rotation;
};

struct button {
  bool a;
  bool b;
};

struct cursor {
  float x;
  float y;
  struct button button[2];

  constexpr cursor()
    : x(4.f), y(4.f)
  {
    button[0] = {false, false};
    button[1] = {false, false};
  }
};

struct cursor_state {
  constexpr static int num_cursors = 2;

  struct cursor cur[num_cursors];
  int port_map[4];

  constexpr cursor_state()
  {
    port_map[0] = 0;
    port_map[1] = 1;
    port_map[2] = -1;
    port_map[3] = -1;
    cur[0] = cursor();
    cur[1] = cursor();
  }
};

void render(const chess::game_state& game_state,
	    const view_transform vt,
	    const cursor_state cursor_state);

}
