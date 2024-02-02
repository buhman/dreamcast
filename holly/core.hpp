#pragma once

void core_init();

void core_start_render(uint32_t frame_address,
		       uint32_t frame_linestride, // in pixels
		       uint32_t frame_size,       // in bytes
		       uint32_t frame_ix, uint32_t num_frames);
void core_start_render(uint32_t frame_ix, uint32_t num_frames);

void core_wait_end_of_render_video();
void core_flip(uint32_t frame_ix, uint32_t num_frames);
