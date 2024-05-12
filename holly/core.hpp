#pragma once

void core_init();

void core_start_render(uint32_t frame_address,
		       uint32_t frame_width); // in pixels

void core_start_render(uint32_t frame_ix);

void core_wait_end_of_render_video();
void core_flip(uint32_t frame_ix);
