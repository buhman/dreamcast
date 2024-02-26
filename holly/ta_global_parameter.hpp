#pragma once

#include <cstdint>
#include <cstddef>

namespace ta_global_parameter {
  struct end_of_list {
    uint32_t parameter_control_word;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;
    uint32_t _res4;
    uint32_t _res5;
    uint32_t _res6;

    end_of_list(const uint32_t parameter_control_word
                )
      : parameter_control_word(parameter_control_word)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
    { }
  };
  static_assert((sizeof (end_of_list)) == 32);
  static_assert((offsetof (struct end_of_list, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct end_of_list, _res0)) == 0x04);
  static_assert((offsetof (struct end_of_list, _res1)) == 0x08);
  static_assert((offsetof (struct end_of_list, _res2)) == 0x0c);
  static_assert((offsetof (struct end_of_list, _res3)) == 0x10);
  static_assert((offsetof (struct end_of_list, _res4)) == 0x14);
  static_assert((offsetof (struct end_of_list, _res5)) == 0x18);
  static_assert((offsetof (struct end_of_list, _res6)) == 0x1c);

  struct user_tile_clip {
    uint32_t parameter_control_word;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t user_clip_x_min;
    uint32_t user_clip_y_min;
    uint32_t user_clip_x_max;
    uint32_t user_clip_y_max;

    user_tile_clip(const uint32_t parameter_control_word,
                   const uint32_t user_clip_x_min,
                   const uint32_t user_clip_y_min,
                   const uint32_t user_clip_x_max,
                   const uint32_t user_clip_y_max
                   )
      : parameter_control_word(parameter_control_word)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , user_clip_x_min(user_clip_x_min)
      , user_clip_y_min(user_clip_y_min)
      , user_clip_x_max(user_clip_x_max)
      , user_clip_y_max(user_clip_y_max)
    { }
  };
  static_assert((sizeof (user_tile_clip)) == 32);
  static_assert((offsetof (struct user_tile_clip, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct user_tile_clip, _res0)) == 0x04);
  static_assert((offsetof (struct user_tile_clip, _res1)) == 0x08);
  static_assert((offsetof (struct user_tile_clip, _res2)) == 0x0c);
  static_assert((offsetof (struct user_tile_clip, user_clip_x_min)) == 0x10);
  static_assert((offsetof (struct user_tile_clip, user_clip_y_min)) == 0x14);
  static_assert((offsetof (struct user_tile_clip, user_clip_x_max)) == 0x18);
  static_assert((offsetof (struct user_tile_clip, user_clip_y_max)) == 0x1c);

  struct object_list_set {
    uint32_t parameter_control_word;
    uint32_t object_pointer;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t bounding_box_x_min;
    uint32_t bounding_box_y_min;
    uint32_t bounding_box_x_max;
    uint32_t bounding_box_y_max;

    object_list_set(const uint32_t parameter_control_word,
                    const uint32_t object_pointer,
                    const uint32_t bounding_box_x_min,
                    const uint32_t bounding_box_y_min,
                    const uint32_t bounding_box_x_max,
                    const uint32_t bounding_box_y_max
                    )
      : parameter_control_word(parameter_control_word)
      , object_pointer(object_pointer)
      , _res0(0)
      , _res1(0)
      , bounding_box_x_min(bounding_box_x_min)
      , bounding_box_y_min(bounding_box_y_min)
      , bounding_box_x_max(bounding_box_x_max)
      , bounding_box_y_max(bounding_box_y_max)
    { }
  };
  static_assert((sizeof (object_list_set)) == 32);
  static_assert((offsetof (struct object_list_set, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct object_list_set, object_pointer)) == 0x04);
  static_assert((offsetof (struct object_list_set, _res0)) == 0x08);
  static_assert((offsetof (struct object_list_set, _res1)) == 0x0c);
  static_assert((offsetof (struct object_list_set, bounding_box_x_min)) == 0x10);
  static_assert((offsetof (struct object_list_set, bounding_box_y_min)) == 0x14);
  static_assert((offsetof (struct object_list_set, bounding_box_x_max)) == 0x18);
  static_assert((offsetof (struct object_list_set, bounding_box_y_max)) == 0x1c);

  struct polygon_type_0 {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word;
    uint32_t texture_control_word;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t data_size_for_sort_dma;
    uint32_t next_address_for_sort_dma;

    polygon_type_0(const uint32_t parameter_control_word,
                   const uint32_t isp_tsp_instruction_word,
                   const uint32_t tsp_instruction_word,
                   const uint32_t texture_control_word,
                   const uint32_t data_size_for_sort_dma,
                   const uint32_t next_address_for_sort_dma
                   )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word(tsp_instruction_word)
      , texture_control_word(texture_control_word)
      , _res0(0)
      , _res1(0)
      , data_size_for_sort_dma(data_size_for_sort_dma)
      , next_address_for_sort_dma(next_address_for_sort_dma)
    { }
  };
  static_assert((sizeof (polygon_type_0)) == 32);
  static_assert((offsetof (struct polygon_type_0, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_0, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct polygon_type_0, tsp_instruction_word)) == 0x08);
  static_assert((offsetof (struct polygon_type_0, texture_control_word)) == 0x0c);
  static_assert((offsetof (struct polygon_type_0, _res0)) == 0x10);
  static_assert((offsetof (struct polygon_type_0, _res1)) == 0x14);
  static_assert((offsetof (struct polygon_type_0, data_size_for_sort_dma)) == 0x18);
  static_assert((offsetof (struct polygon_type_0, next_address_for_sort_dma)) == 0x1c);

  struct polygon_type_1 {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word;
    uint32_t texture_control_word;
    float face_color_alpha;
    float face_color_r;
    float face_color_g;
    float face_color_b;

    polygon_type_1(const uint32_t parameter_control_word,
                   const uint32_t isp_tsp_instruction_word,
                   const uint32_t tsp_instruction_word,
                   const uint32_t texture_control_word,
                   const float face_color_alpha,
                   const float face_color_r,
                   const float face_color_g,
                   const float face_color_b
                   )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word(tsp_instruction_word)
      , texture_control_word(texture_control_word)
      , face_color_alpha(face_color_alpha)
      , face_color_r(face_color_r)
      , face_color_g(face_color_g)
      , face_color_b(face_color_b)
    { }
  };
  static_assert((sizeof (polygon_type_1)) == 32);
  static_assert((offsetof (struct polygon_type_1, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_1, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct polygon_type_1, tsp_instruction_word)) == 0x08);
  static_assert((offsetof (struct polygon_type_1, texture_control_word)) == 0x0c);
  static_assert((offsetof (struct polygon_type_1, face_color_alpha)) == 0x10);
  static_assert((offsetof (struct polygon_type_1, face_color_r)) == 0x14);
  static_assert((offsetof (struct polygon_type_1, face_color_g)) == 0x18);
  static_assert((offsetof (struct polygon_type_1, face_color_b)) == 0x1c);

  struct polygon_type_2 {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word;
    uint32_t texture_control_word;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t data_size_for_sort_dma;
    uint32_t next_address_for_sort_dma;
    float face_color_alpha;
    float face_color_r;
    float face_color_g;
    float face_color_b;
    float face_offset_color_alpha;
    float face_offset_color_r;
    float face_offset_color_g;
    float face_offset_color_b;

    polygon_type_2(const uint32_t parameter_control_word,
                   const uint32_t isp_tsp_instruction_word,
                   const uint32_t tsp_instruction_word,
                   const uint32_t texture_control_word,
                   const uint32_t data_size_for_sort_dma,
                   const uint32_t next_address_for_sort_dma,
                   const float face_color_alpha,
                   const float face_color_r,
                   const float face_color_g,
                   const float face_color_b,
                   const float face_offset_color_alpha,
                   const float face_offset_color_r,
                   const float face_offset_color_g,
                   const float face_offset_color_b
                   )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word(tsp_instruction_word)
      , texture_control_word(texture_control_word)
      , _res0(0)
      , _res1(0)
      , data_size_for_sort_dma(data_size_for_sort_dma)
      , next_address_for_sort_dma(next_address_for_sort_dma)
      , face_color_alpha(face_color_alpha)
      , face_color_r(face_color_r)
      , face_color_g(face_color_g)
      , face_color_b(face_color_b)
      , face_offset_color_alpha(face_offset_color_alpha)
      , face_offset_color_r(face_offset_color_r)
      , face_offset_color_g(face_offset_color_g)
      , face_offset_color_b(face_offset_color_b)
    { }
  };
  static_assert((sizeof (polygon_type_2)) == 64);
  static_assert((offsetof (struct polygon_type_2, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_2, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct polygon_type_2, tsp_instruction_word)) == 0x08);
  static_assert((offsetof (struct polygon_type_2, texture_control_word)) == 0x0c);
  static_assert((offsetof (struct polygon_type_2, _res0)) == 0x10);
  static_assert((offsetof (struct polygon_type_2, _res1)) == 0x14);
  static_assert((offsetof (struct polygon_type_2, data_size_for_sort_dma)) == 0x18);
  static_assert((offsetof (struct polygon_type_2, next_address_for_sort_dma)) == 0x1c);
  static_assert((offsetof (struct polygon_type_2, face_color_alpha)) == 0x20);
  static_assert((offsetof (struct polygon_type_2, face_color_r)) == 0x24);
  static_assert((offsetof (struct polygon_type_2, face_color_g)) == 0x28);
  static_assert((offsetof (struct polygon_type_2, face_color_b)) == 0x2c);
  static_assert((offsetof (struct polygon_type_2, face_offset_color_alpha)) == 0x30);
  static_assert((offsetof (struct polygon_type_2, face_offset_color_r)) == 0x34);
  static_assert((offsetof (struct polygon_type_2, face_offset_color_g)) == 0x38);
  static_assert((offsetof (struct polygon_type_2, face_offset_color_b)) == 0x3c);

  struct polygon_type_3 {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word_0;
    uint32_t texture_control_word_0;
    uint32_t tsp_instruction_word_1;
    uint32_t texture_control_word_1;
    uint32_t data_size_for_sort_dma;
    uint32_t next_address_for_sort_dma;

    polygon_type_3(const uint32_t parameter_control_word,
                   const uint32_t isp_tsp_instruction_word,
                   const uint32_t tsp_instruction_word_0,
                   const uint32_t texture_control_word_0,
                   const uint32_t tsp_instruction_word_1,
                   const uint32_t texture_control_word_1,
                   const uint32_t data_size_for_sort_dma,
                   const uint32_t next_address_for_sort_dma
                   )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word_0(tsp_instruction_word_0)
      , texture_control_word_0(texture_control_word_0)
      , tsp_instruction_word_1(tsp_instruction_word_1)
      , texture_control_word_1(texture_control_word_1)
      , data_size_for_sort_dma(data_size_for_sort_dma)
      , next_address_for_sort_dma(next_address_for_sort_dma)
    { }
  };
  static_assert((sizeof (polygon_type_3)) == 32);
  static_assert((offsetof (struct polygon_type_3, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_3, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct polygon_type_3, tsp_instruction_word_0)) == 0x08);
  static_assert((offsetof (struct polygon_type_3, texture_control_word_0)) == 0x0c);
  static_assert((offsetof (struct polygon_type_3, tsp_instruction_word_1)) == 0x10);
  static_assert((offsetof (struct polygon_type_3, texture_control_word_1)) == 0x14);
  static_assert((offsetof (struct polygon_type_3, data_size_for_sort_dma)) == 0x18);
  static_assert((offsetof (struct polygon_type_3, next_address_for_sort_dma)) == 0x1c);

  struct polygon_type_4 {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word_0;
    uint32_t texture_control_word_0;
    uint32_t tsp_instruction_word_1;
    uint32_t texture_control_word_1;
    uint32_t data_size_for_sort_dma;
    uint32_t next_address_for_sort_dma;
    float face_color_alpha_0;
    float face_color_r_0;
    float face_color_g_0;
    float face_color_b_0;
    float face_color_alpha_1;
    float face_color_r_1;
    float face_color_g_1;
    float face_color_b_1;

    polygon_type_4(const uint32_t parameter_control_word,
                   const uint32_t isp_tsp_instruction_word,
                   const uint32_t tsp_instruction_word_0,
                   const uint32_t texture_control_word_0,
                   const uint32_t tsp_instruction_word_1,
                   const uint32_t texture_control_word_1,
                   const uint32_t data_size_for_sort_dma,
                   const uint32_t next_address_for_sort_dma,
                   const float face_color_alpha_0,
                   const float face_color_r_0,
                   const float face_color_g_0,
                   const float face_color_b_0,
                   const float face_color_alpha_1,
                   const float face_color_r_1,
                   const float face_color_g_1,
                   const float face_color_b_1
                   )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word_0(tsp_instruction_word_0)
      , texture_control_word_0(texture_control_word_0)
      , tsp_instruction_word_1(tsp_instruction_word_1)
      , texture_control_word_1(texture_control_word_1)
      , data_size_for_sort_dma(data_size_for_sort_dma)
      , next_address_for_sort_dma(next_address_for_sort_dma)
      , face_color_alpha_0(face_color_alpha_0)
      , face_color_r_0(face_color_r_0)
      , face_color_g_0(face_color_g_0)
      , face_color_b_0(face_color_b_0)
      , face_color_alpha_1(face_color_alpha_1)
      , face_color_r_1(face_color_r_1)
      , face_color_g_1(face_color_g_1)
      , face_color_b_1(face_color_b_1)
    { }
  };
  static_assert((sizeof (polygon_type_4)) == 64);
  static_assert((offsetof (struct polygon_type_4, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct polygon_type_4, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct polygon_type_4, tsp_instruction_word_0)) == 0x08);
  static_assert((offsetof (struct polygon_type_4, texture_control_word_0)) == 0x0c);
  static_assert((offsetof (struct polygon_type_4, tsp_instruction_word_1)) == 0x10);
  static_assert((offsetof (struct polygon_type_4, texture_control_word_1)) == 0x14);
  static_assert((offsetof (struct polygon_type_4, data_size_for_sort_dma)) == 0x18);
  static_assert((offsetof (struct polygon_type_4, next_address_for_sort_dma)) == 0x1c);
  static_assert((offsetof (struct polygon_type_4, face_color_alpha_0)) == 0x20);
  static_assert((offsetof (struct polygon_type_4, face_color_r_0)) == 0x24);
  static_assert((offsetof (struct polygon_type_4, face_color_g_0)) == 0x28);
  static_assert((offsetof (struct polygon_type_4, face_color_b_0)) == 0x2c);
  static_assert((offsetof (struct polygon_type_4, face_color_alpha_1)) == 0x30);
  static_assert((offsetof (struct polygon_type_4, face_color_r_1)) == 0x34);
  static_assert((offsetof (struct polygon_type_4, face_color_g_1)) == 0x38);
  static_assert((offsetof (struct polygon_type_4, face_color_b_1)) == 0x3c);

  struct sprite {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t tsp_instruction_word;
    uint32_t texture_control_word;
    uint32_t base_color;
    uint32_t offset_color;
    uint32_t data_size_for_sort_dma;
    uint32_t next_address_for_sort_dma;

    sprite(const uint32_t parameter_control_word,
           const uint32_t isp_tsp_instruction_word,
           const uint32_t tsp_instruction_word,
           const uint32_t texture_control_word,
           const uint32_t base_color,
           const uint32_t offset_color,
           const uint32_t data_size_for_sort_dma,
           const uint32_t next_address_for_sort_dma
           )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , tsp_instruction_word(tsp_instruction_word)
      , texture_control_word(texture_control_word)
      , base_color(base_color)
      , offset_color(offset_color)
      , data_size_for_sort_dma(data_size_for_sort_dma)
      , next_address_for_sort_dma(next_address_for_sort_dma)
    { }
  };
  static_assert((sizeof (sprite)) == 32);
  static_assert((offsetof (struct sprite, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct sprite, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct sprite, tsp_instruction_word)) == 0x08);
  static_assert((offsetof (struct sprite, texture_control_word)) == 0x0c);
  static_assert((offsetof (struct sprite, base_color)) == 0x10);
  static_assert((offsetof (struct sprite, offset_color)) == 0x14);
  static_assert((offsetof (struct sprite, data_size_for_sort_dma)) == 0x18);
  static_assert((offsetof (struct sprite, next_address_for_sort_dma)) == 0x1c);

  struct modifier_volume {
    uint32_t parameter_control_word;
    uint32_t isp_tsp_instruction_word;
    uint32_t _res0;
    uint32_t _res1;
    uint32_t _res2;
    uint32_t _res3;
    uint32_t _res4;
    uint32_t _res5;

    modifier_volume(const uint32_t parameter_control_word,
                    const uint32_t isp_tsp_instruction_word
                    )
      : parameter_control_word(parameter_control_word)
      , isp_tsp_instruction_word(isp_tsp_instruction_word)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
    { }
  };
  static_assert((sizeof (modifier_volume)) == 32);
  static_assert((offsetof (struct modifier_volume, parameter_control_word)) == 0x00);
  static_assert((offsetof (struct modifier_volume, isp_tsp_instruction_word)) == 0x04);
  static_assert((offsetof (struct modifier_volume, _res0)) == 0x08);
  static_assert((offsetof (struct modifier_volume, _res1)) == 0x0c);
  static_assert((offsetof (struct modifier_volume, _res2)) == 0x10);
  static_assert((offsetof (struct modifier_volume, _res3)) == 0x14);
  static_assert((offsetof (struct modifier_volume, _res4)) == 0x18);
  static_assert((offsetof (struct modifier_volume, _res5)) == 0x1c);

}

