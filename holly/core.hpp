#pragma once

void core_init();

void core_start_render(uint32_t frame_address,
		       uint32_t frame_width); // in pixels

void core_start_render(uint32_t frame_ix);

void core_start_render2(uint32_t region_array_start,
			uint32_t isp_tsp_parameters_start,
			uint32_t background_start,
			uint32_t frame_address,
			uint32_t frame_width      // in pixels
			);

void core_start_render3(uint32_t region_array_start,
			uint32_t isp_tsp_parameters_start,
			uint32_t background_start,
			uint32_t frame_address,
			uint32_t frame_width,     // in pixels
                        uint32_t bytes_per_pixel
			);

void core_wait_end_of_render_video();
void core_flip(uint32_t frame_ix);
