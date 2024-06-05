#pragma once

#include <cstdint>

#include "chess/chess.hpp"

namespace render {

struct view_transform {
  float piece_rotation;
  float board_rotation;
};

template <typename T>
struct animator {
  T start;
  T end;
  int steps;
  int step;

  constexpr animator(T initial)
    : start(initial), end(initial), steps(0), step(0)
  { }

  void set_target(T target_value, int target_steps)
  {
    if (target_value != end) {
      start = interpolate();
      end = target_value;
      steps = target_steps;
      step = 0;
    }
  }

  T interpolate()
  {
    if (step == steps)
      return end;

    T value = start + ((end - start) * step) / steps;
    step += 1;
    return value;
  }
};

struct button {
  bool a;
  bool b;
  bool x;
  bool y;
};

struct cursor {
  float x;
  float y;
  struct button button[2];

  constexpr cursor()
    : x(3.5f), y(3.5f)
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

void render(const view_transform vt,
	    const chess::game_state& game_state,
	    const cursor_state cursor_state);

}
