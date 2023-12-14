#pragma once

void core_init();
void core_start_render(uint32_t frame_ix, uint32_t num_frames);
void core_wait_end_of_render_video(uint32_t frame_ix, uint32_t num_frames);
