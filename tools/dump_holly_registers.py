import struct
import sys

registers = [
    ("ID", 0x0),
    ("REVISION", 0x4),
    ("SOFTRESET", 0x8),
    ("STARTRENDER", 0x14),
    ("TEST_SELECT", 0x18),
    ("PARAM_BASE", 0x20),
    ("REGION_BASE", 0x2c),
    ("SPAN_SORT_CFG", 0x30),
    ("VO_BORDER_COL", 0x40),
    ("FB_R_CTRL", 0x44),
    ("FB_W_CTRL", 0x48),
    ("FB_W_LINESTRIDE", 0x4c),
    ("FB_R_SOF1", 0x50),
    ("FB_R_SOF2", 0x54),
    ("FB_R_SIZE", 0x5c),
    ("FB_W_SOF1", 0x60),
    ("FB_W_SOF2", 0x64),
    ("FB_X_CLIP", 0x68),
    ("FB_Y_CLIP", 0x6c),
    ("FPU_SHAD_SCALE", 0x74),
    ("FPU_CULL_VAL", 0x78),
    ("FPU_PARAM_CFG", 0x7c),
    ("HALF_OFFSET", 0x80),
    ("FPU_PERP_VAL", 0x84),
    ("ISP_BACKGND_D", 0x88),
    ("ISP_BACKGND_T", 0x8c),
    ("ISP_FEED_CFG", 0x98),
    ("SDRAM_REFRESH", 0xa0),
    ("SDRAM_ARB_CFG", 0xa4),
    ("SDRAM_CFG", 0xa8),
    ("FOG_COL_RAM", 0xb0),
    ("FOG_COL_VERT", 0xb4),
    ("FOG_DENSITY", 0xb8),
    ("FOG_CLAMP_MAX", 0xbc),
    ("FOG_CLAMP_MIN", 0xc0),
    ("SPG_TRIGGER_POS", 0xc4),
    ("SPG_HBLANK_INT", 0xc8),
    ("SPG_VBLANK_INT", 0xcc),
    ("SPG_CONTROL", 0xd0),
    ("SPG_HBLANK", 0xd4),
    ("SPG_LOAD", 0xd8),
    ("SPG_VBLANK", 0xdc),
    ("SPG_WIDTH", 0xe0),
    ("TEXT_CONTROL", 0xe4),
    ("VO_CONTROL", 0xe8),
    ("VO_STARTX", 0xec),
    ("VO_STARTY", 0xf0),
    ("SCALER_CTL", 0xf4),
    ("PAL_RAM_CTRL", 0x108),
    ("SPG_STATUS", 0x10c),
    ("FB_BURSTCTRL", 0x110),
    ("FB_C_SOF", 0x114),
    ("Y_COEFF", 0x118),
    ("PT_ALPHA_REF", 0x11c),
    ("TA_OL_BASE", 0x124),
    ("TA_ISP_BASE", 0x128),
    ("TA_OL_LIMIT", 0x12c),
    ("TA_ISP_LIMIT", 0x130),
    ("TA_NEXT_OPB", 0x134),
    ("TA_ITP_CURRENT", 0x138),
    ("TA_GLOB_TILE_CLIP", 0x13c),
    ("TA_ALLOC_CTRL", 0x140),
    ("TA_LIST_INIT", 0x144),
    ("TA_YUV_TEX_BASE", 0x148),
    ("TA_YUV_TEX_CTRL", 0x14c),
    ("TA_YUV_TEX_CNT", 0x150),
    ("TA_LIST_CONT", 0x160),
    ("TA_NEXT_OPB_INIT", 0x164),
    #FOG_TABLE 0x200
    #TA_OL_POINTERS 0x600
    #PALETTE_RAM 0x1000
]

bits = {
    "SPAN_SORT_CFG": [
        ("cache_bypass", 1, 16),
        ("offset_sort_enable", 1, 8),
        ("span_sort_enable", 1, 0),
    ],
    "VO_BORDER_COL": [
        ("chroma", 1, 24),
        ("red", 0xff, 16),
        ("green", 0xff, 8),
        ("blue", 0xff, 0),
    ],
    "FB_R_CTRL": [
        ("vclk_div", 1, 23),
        ("fb_strip_buf_en", 1, 22),
        ("fb_stripsize", 0x3e, 16),
        ("fb_chroma_threshold", 0xff, 8),
        ("fb_concat", 0x3, 4),
        ("fb_depth", 0x3, 2),
        ("fb_line_double", 1, 1),
        ("fb_enable", 1, 0),
    ],
    "FB_W_CTRL": [
        ("fb_alpha_threshold", 0xff, 16),
        ("fb_kval", 0xff, 8),
        ("fb_dither", 1, 3),
        ("fb_packmode", 0x7, 0),
    ],
    "FB_R_SIZE": [
        ("fb_modulus", 0x3ff, 20),
        ("fb_y_size", 0x3ff, 10),
        ("fb_x_size", 0x3ff, 0),
    ],
    "FB_X_CLIP": [
        ("fb_x_clip_max", 0x7ff, 16),
        ("fb_x_clip_min", 0x7ff, 0),
    ],
    "FB_Y_CLIP": [
        ("fb_y_clip_max", 0x7ff, 16),
        ("fb_y_clip_min", 0x7ff, 0),
    ],
    "FPU_SHAD_SCALE": [
        ("simple_shadow_enable", 1, 8),
        ("scale_factor_for_shadows", 0xff, 0),
    ],
    "FPU_PARAM_CFG": [
        ("region_header_type", 1, 21),
        ("tsp_parameter_burst_threshold", 0x3f, 14),
        ("isp_parameter_burst_threshold", 0x3f, 8),
        ("pointer_burst_size", 0xf, 4),
        ("pointer_first_burst_size", 0xf, 0)
    ],
    "HALF_OFFSET": [
        ("tsp_texel_sampling_position", 1, 2),
        ("tsp_pixel_sampling_position", 1, 1),
        ("fpu_pixel_sampling_position", 1, 0),
    ],
    "ISP_BACKGND_T": [
        ("cache_bypass", 1, 28),
        ("shadow", 1, 27),
        ("skip", 7, 24),
        ("tag_address", 0x1fffff, 3),
        ("tag_offset", 0x7, 0),
    ],
    "ISP_FEED_CFG": [
        ("cache_size_for_translucency", 0x3ff, 14),
        ("punch_through_chunk_size", 0x3ff, 4),
        ("discard_mode", 1, 3),
        ("pre_sort_mode", 1, 0),
    ],
    "FOG_COL_RAM": [
        ("red", 0xff, 16),
        ("green", 0xff, 8),
        ("blue", 0xff, 0),
    ],
    "FOG_COL_VERT": [
        ("red", 0xff, 16),
        ("green", 0xff, 8),
        ("blue", 0xff, 0),
    ],
    "FOG_DENSITY": [
        ("fog_scale_mantissa", 0xff, 8),
        ("fog_scale_exponent", 0xff, 0),
    ],
    "FOG_CLAMP_MAX": [
        ("alpha", 0xff, 24),
        ("red", 0xff, 16),
        ("green", 0xff, 8),
        ("blue", 0xff, 0),
    ],
    "FOG_CLAMP_MIN": [
        ("alpha", 0xff, 24),
        ("red", 0xff, 16),
        ("green", 0xff, 8),
        ("blue", 0xff, 0),
    ],
    "SPG_TRIGGER_POS": [
        ("trigger_v_count", 0x3ff, 16),
        ("trigger_h_count", 0x3ff, 0),
    ],
    "SPG_HBLANK_INT": [
        ("hblank_int_mode", 0x3, 12),
        ("line_comp_val", 0x3ff, 0),
    ],
    "SPG_VBLANK_INT": [
        ("vblank_out_interrupt_line_number", 0x3ff, 16),
        ("vblank_in_interrupt_line_number", 0x3ff, 0),
    ],
    "SPG_CONTROL": [
        ("csync", 1, 9),
        ("sync_direction", 1, 8),
        ("pal", 1, 7),
        ("ntsc", 1, 6),
        ("force_field2", 1, 5),
        ("interlace", 1, 4),
        ("spg_lock", 1, 3),
        ("mcsync_pol", 0x1, 2),
        ("mvsync_pol", 0x1, 1),
        ("mhsync_pol", 0x1, 0),
    ],
    "SPG_HBLANK": [
        ("hbend", 0x3ff, 16),
        ("hbend", 0x3ff, 0),
    ],
    "SPG_LOAD": [
        ("vcount", 0x3ff, 16),
        ("hcount", 0x3ff, 0),
    ],
    "SPG_VBLANK": [
        ("vcount", 0x3ff, 16),
        ("hcount", 0x3ff, 0),
    ],
    "SPG_WIDTH": [
        ("eqwidth", 0x3ff, 22),
        ("bpwidth", 0x3ff, 12),
        ("vswidth", 0xf, 8),
        ("hswidth", 0x7f, 0),
    ],
    "TEXT_CONTROL": [
        ("code_book_endian", 1, 17),
        ("index_endian", 1, 17),
        ("bank_bit", 0x1f, 8),
        ("stride", 0x1f, 0),
    ],
    "VO_CONTROL": [
        ("pclk_delay_reset", 1, 21),
        ("pclk_delay", 0x1f, 16),
        ("pixel_double", 1, 8),
        ("field_mode", 0xf, 4),
        ("blank_video", 1, 3),
        ("blank_pol", 1, 2),
        ("vsync_pol", 1, 1),
        ("hsync_pol", 1, 0),
    ],
    "VO_STARTY": [
        ("vertical_start_position_on_field_2", 0x3ff, 16),
        ("vertical_start_position_on_field_1", 0x3ff, 0),
    ],
    "SCALER_CTL": [
        ("field_select", 1, 18),
        ("interlace", 1, 17),
        ("horizontal_scaling_enable", 1, 16),
        ("vertical_scale_factor", 0xffff, 0),
    ],
    "FB_BURSTCTRL": [
        ("wr_burst", 0xf, 16),
        ("vid_lat", 0x7f, 8),
        ("vid_burst", 0x3f, 0),
    ],
    "Y_COEFF": [
        ("coefficient_1", 0xff, 8),
        ("coefficient_0_2", 0xff, 0),
    ],
    "TA_GLOB_TILE_CLIP": [
        ("tile_y_num", 0xf, 16),
        ("tile_x_num", 0x1f, 0),
    ],
    "TA_ALLOC_CTRL": [
        ("opb_mode", 1, 20),
        ("pt_opb", 0x3, 16),
        ("tm_opb", 0x3, 12),
        ("t_opb", 0x3, 8),
        ("om_opb", 0x3, 4),
        ("o_opb", 0x3, 0),
    ],
    "TA_YUV_TEX_CTRL": [
        ("yuv_form", 0x1, 24),
        ("yuv_tex", 0x1, 16),
        ("yuv_v_size", 0x3f, 8),
        ("yuv_u_size", 0x3f, 0),
    ],
    "TA_OL_POINTERS": [
        ("entry", 31, 0x1),
        ("sprite", 30, 0x1),
        ("triangle", 29, 0x1),
        ("number_of_triangles_quads", 25, 0xf),
        ("shadow", 24, 0x1),
        ("pointer_address", 2, 0x3fffff),
        ("skip", 0, 0x3),
    ]
}

with open(sys.argv[1], 'rb') as f:
    buf = f.read()

assert len(buf) == 8192, len(buf)

max_length = max(len(name) for name, _ in registers)

for name, offset in registers:
    assert offset % 4 == 0, (name, offset)

    value, = struct.unpack("<I", buf[offset:offset+4])
    print(name.rjust(max_length), f"{value:08x}")
    if name not in bits:
        continue
    for bit_name, mask, shift in bits[name]:
        bit_value = (value >> shift) & mask
        print(" " * max_length, " ", bit_name, bit_value)
