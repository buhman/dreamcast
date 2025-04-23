#include "holly/background.hpp"
#include "holly/core.hpp"
#include "holly/core_bits.hpp"
#include "holly/holly.hpp"
#include "holly/isp_tsp.hpp"
#include "holly/region_array.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/texture_memory_alloc3.hpp"
#include "holly/video_output.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "memorymap.hpp"

#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"
#include "sh7091/serial.hpp"
#include "printf/printf.h"

#include "q3bsp/q3bsp.h"
#include "pk/maps/20kdm2.bsp.h"

#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/vec4.hpp"
#include "math/mat4x4.hpp"
#include "math/geometry.hpp"

#include "interrupt.hpp"

#include "pk/textures/e7/e7walldesign01b.data.h"
#include "pk/textures/e7/e7steptop2.data.h"
#include "pk/textures/e7/e7dimfloor.data.h"
#include "pk/textures/e7/e7brickfloor01.data.h"
#include "pk/textures/e7/e7bmtrim.data.h"
#include "pk/textures/e7/e7sbrickfloor.data.h"
#include "pk/textures/e7/e7brnmetal.data.h"
#include "pk/textures/e7/e7beam02_red.data.h"
#include "pk/textures/e7/e7swindow.data.h"
#include "pk/textures/e7/e7bigwall.data.h"
#include "pk/textures/e7/e7panelwood.data.h"
#include "pk/textures/e7/e7beam01.data.h"
#include "pk/textures/gothic_floor/xstepborder5.data.h"
#include "pk/textures/liquids/lavahell.data.h"
#include "pk/textures/e7/e7steptop.data.h"
#include "pk/textures/gothic_trim/metalblackwave01.data.h"
#include "pk/textures/stone/pjrock1.data.h"
#include "pk/models/mapobjects/timlamp/timlamp.data.h"
#include "pk/textures/sfx/flame2.data.h"
#include "pk/models/mapobjects/gratelamp/gratetorch2.data.h"
#include "pk/models/mapobjects/gratelamp/gratetorch2b.data.h"

#include "pk/texture.h"

using vec2 = vec<2, float>;
using vec3 = vec<3, float>;
using vec4 = vec<4, float>;
using mat4x4 = mat<4, 4, float>;

#define _fsrra(n) (1.0f / (__builtin_sqrtf(n)))

void print_direntries(struct q3bsp_header * header)
{
  // direntries
  static const char * lump_name[] = {
    "entities",
    "textures",
    "planes",
    "nodes",
    "leafs",
    "leaffaces",
    "leafbrushes",
    "models",
    "brushes",
    "brushsides",
    "vertexes",
    "meshverts",
    "effects",
    "faces",
    "lightmaps",
    "lightvols",
    "visdata",
  };
  for (int i = 0; i < 17; i++) {
    printf("%s offset=%d length=%d\n",
           lump_name[i],
           header->direntries[i].offset,
           header->direntries[i].length);
  }
}

void print_header(void * buf)
{
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  serial::string("magic: ");
  serial::string((uint8_t *)header->magic, 4);
  serial::character('\n');
  printf("version: %x\n", header->version);

  print_direntries(header);
}

void print_textures(void * buf, int length)
{
  q3bsp_texture_t * texture = reinterpret_cast<q3bsp_texture_t *>(buf);

  int count = length / (sizeof (struct q3bsp_texture));

  for (int i = 0; i < count; i++) {
    printf("texture [%d]\n", i);
    printf("  name=%s\n", texture[i].name);
    printf("  flags=%x\n", texture[i].flags);
    printf("  contents=%x\n", texture[i].contents);
  }
}

void print_models(void * buf, int length)
{
  q3bsp_model_t * model = reinterpret_cast<q3bsp_model_t *>(buf);

  int count = length / (sizeof (struct q3bsp_model));

  for (int i = 0; i < count; i++) {
    printf("model [%d]\n", i);
    printf("  mins={%f, %f, %f}\n", model->mins[0], model->mins[2], model->mins[2]);
    printf("  maxs={%f, %f, %f}\n", model->maxs[0], model->maxs[2], model->maxs[2]);
    printf("  face=%d\n", model->face);
    printf("  n_faces=%d\n", model->n_faces);
    printf("  brush=%d\n", model->brush);
    printf("  n_brushes=%d\n", model->n_brushes);
  }
}

void print_faces(void * buf, int length)
{
  q3bsp_face_t * face = reinterpret_cast<q3bsp_face_t *>(buf);

  int count = length / (sizeof (struct q3bsp_face));

  printf("faces count: %d\n", count);
  for (int i = 0; i < count; i++) {
    printf("face [%d]\n", i);
    printf("  type=%d n_vertexes=%d n_meshverts=%d\n", face[i].type, face[i].n_vertexes, face[i].n_meshverts);
  }
}

void debug_print_q3bsp(uint8_t * buf, q3bsp_header_t * header)
{
  // header
  print_header(buf);

  {
    q3bsp_direntry * e = &header->direntries[LUMP_TEXTURES];
    print_textures(&buf[e->offset], e->length);
  }

  {
    q3bsp_direntry * e = &header->direntries[LUMP_MODELS];
    print_models(&buf[e->offset], e->length);
  }

  {
    q3bsp_direntry * e = &header->direntries[LUMP_FACES];
    print_faces(&buf[e->offset], e->length);
  }
}

struct position_normal {
  vec3 position;
  vec3 normal;
};

static position_normal vertex_cache[16384];

static inline vec3 normal_transform(mat4x4& trans, vec3 normal)
{
  vec4 n = trans * (vec4){normal.x, normal.y, normal.z, 0.f}; // no translation component
  return {n.x, n.y, n.z};
}

static inline vec3 screen_transform(vec3 v)
{
  float dim = 480 / 2.0;

  return {
    v.x / (1.f * v.z) * dim + 640 / 2.0f,
    v.y / (1.f * v.z) * dim + 480 / 2.0f,
    1 / v.z,
  };
}

void global_polygon_type_1(ta_parameter_writer& writer,
                           uint32_t obj_control_texture,
                           uint32_t texture_u_v_size,
                           uint32_t texture_control_word)
{
  const uint32_t parameter_control_word = para_control::para_type::polygon_or_modifier_volume
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::intensity_mode_1
                                        | obj_control::gouraud
                                        | obj_control_texture
                                        ;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling
                                          ;

  const uint32_t tsp_instruction_word = tsp_instruction_word::fog_control::no_fog
                                      | tsp_instruction_word::src_alpha_instr::one
                                      | tsp_instruction_word::dst_alpha_instr::zero
                                      | tsp_instruction_word::filter_mode::bilinear_filter
                                      | tsp_instruction_word::texture_shading_instruction::modulate
                                      | texture_u_v_size
                                      ;

  const float a = 1.0f;
  const float r = 1.0f;
  const float g = 1.0f;
  const float b = 1.0f;

  writer.append<ta_global_parameter::polygon_type_1>() =
    ta_global_parameter::polygon_type_1(parameter_control_word,
                                        isp_tsp_instruction_word,
                                        tsp_instruction_word,
                                        texture_control_word,
                                        a,
                                        r,
                                        g,
                                        b
                                        );
}

void global_texture(ta_parameter_writer& writer, int ix)
{
  struct pk_texture * texture = &textures[ix];

  uint32_t texture_u_v_size = tsp_instruction_word::texture_u_size::from_int(texture->width)
                            | tsp_instruction_word::texture_v_size::from_int(texture->height)
                            ;

  uint32_t texture_address = texture_memory_alloc.texture.start + texture->offset * 2;
  uint32_t texture_control_word = texture_control_word::pixel_format::_565
                                | texture_control_word::scan_order::non_twiddled
                                | texture_control_word::texture_address(texture_address / 8)
                                ;

  global_polygon_type_1(writer,
                        obj_control::texture,
                        texture_u_v_size,
                        texture_control_word);
}

void transform_vertices(uint8_t * buf, int length, mat4x4& trans)
{
  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(buf);

  int count = length / (sizeof (struct q3bsp_vertex));

  for (int i = 0; i < count; i++) {
    vec3 v = {vert[i].position[0], vert[i].position[1], vert[i].position[2]};
    vec3 n = {vert[i].normal[0], vert[i].normal[1], vert[i].normal[2]};

    //printf("%f %f %f\n", v.x, v.y, v.z);

    vertex_cache[i].position = screen_transform(trans * v);
    vertex_cache[i].normal = normal_transform(trans, n);
  }
}

static inline void render_tri_type_2(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     float ai,
                                     float bi,
                                     float ci)
{
  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        ai);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bi);

  writer.append<ta_vertex_parameter::polygon_type_2>() =
    ta_vertex_parameter::polygon_type_2(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ci);
}

static inline void render_tri_type_7(ta_parameter_writer& writer,
                                     vec3 ap,
                                     vec3 bp,
                                     vec3 cp,
                                     vec2 at,
                                     vec2 bt,
                                     vec2 ct,
                                     float ai,
                                     float bi,
                                     float ci)
{
  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        ap.x, ap.y, ap.z,
                                        at.x, at.y,
                                        ai,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(false),
                                        bp.x, bp.y, bp.z,
                                        bt.x, bt.y,
                                        bi,
                                        0);

  writer.append<ta_vertex_parameter::polygon_type_7>() =
    ta_vertex_parameter::polygon_type_7(polygon_vertex_parameter_control_word(true),
                                        cp.x, cp.y, cp.z,
                                        ct.x, ct.y,
                                        ci,
                                        0);
}

static inline float inverse_length(vec3 v)
{
  float f = dot(v, v);
  return _fsrra(f);
}

float light_intensity(vec3 light_vec, vec3 n)
{
  float n_dot_l = dot(n, light_vec);

  float intensity = 0.4f;
  if (n_dot_l > 0) {
    intensity += 0.5f * n_dot_l * (inverse_length(n) * inverse_length(light_vec));
    if (intensity > 1.0f)
      intensity = 1.0f;
  }
  return intensity;
}

void transfer_faces(uint8_t * buf, q3bsp_header_t * header, ta_parameter_writer& writer)
{
  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  q3bsp_direntry * me = &header->direntries[LUMP_MESHVERTS];
  q3bsp_direntry * fe = &header->direntries[LUMP_FACES];

  q3bsp_vertex_t * vert = reinterpret_cast<q3bsp_vertex_t *>(&buf[ve->offset]);
  q3bsp_meshvert_t * meshvert = reinterpret_cast<q3bsp_meshvert_t *>(&buf[me->offset]);
  q3bsp_face_t * face = reinterpret_cast<q3bsp_face_t *>(&buf[fe->offset]);

  int face_count = fe->length / (sizeof (struct q3bsp_face));

  const vec3 light_vec = {20, 20, 20};

  int textures_length = (sizeof (textures)) / (sizeof (textures[0]));
  int last_texture = -1;

  for (int i = 0; i < face_count; i++) {
    int meshvert_ix = face[i].meshvert;
    q3bsp_meshvert_t * mv = &meshvert[meshvert_ix];

    int triangles = face[i].n_meshverts / 3;

    bool has_texture = 1 &&
                       (face[i].texture >= 0) &&
                       (face[i].texture < textures_length) &&
                       (textures[face[i].texture].size != 0);

    if (face[i].texture != last_texture) {
      last_texture = face[i].texture;
      if (has_texture) {
        global_texture(writer, face[i].texture);
      } else {
        global_polygon_type_1(writer, 0, 0, 0);
      }
    }

    for (int j = 0; j < triangles; j++) {

      int aix = mv[j * 3 + 0].offset + face[i].vertex;
      int bix = mv[j * 3 + 1].offset + face[i].vertex;
      int cix = mv[j * 3 + 2].offset + face[i].vertex;

      vec3 ap = vertex_cache[aix].position;
      vec3 bp = vertex_cache[bix].position;
      vec3 cp = vertex_cache[cix].position;

      if (ap.z < 0 || bp.z < 0 || cp.z < 0) {
        continue;
      }

      vec3 n = vertex_cache[aix].normal;
      float li = light_intensity(light_vec, n);

      /*
      printf("{%f %f %f} {%f %f %f} {%f %f %f}\n",
             ap.x, ap.y, ap.z,
             bp.x, bp.y, bp.z,
             cp.x, cp.y, cp.z);
      */


      if (has_texture) {
        float v_mul = textures[face[i].texture].v_mul;
        vec2 at = {vert[aix].texcoord[0], vert[aix].texcoord[1] * v_mul};
        vec2 bt = {vert[bix].texcoord[0], vert[bix].texcoord[1] * v_mul};
        vec2 ct = {vert[cix].texcoord[0], vert[cix].texcoord[1] * v_mul};
        //printf("{%f %f} {%f %f} {%f %f}\n", at.x, at.y, bt.x, bt.y, ct.x, ct.y);


        render_tri_type_7(writer,
                          ap,
                          bp,
                          cp,
                          at,
                          bt,
                          ct,
                          li,
                          li,
                          li);
      } else {
        render_tri_type_2(writer,
                          ap,
                          bp,
                          cp,
                          li,
                          li,
                          li);
      }
    }
  }
}

void transfer_scene(ta_parameter_writer& writer, const mat4x4& screen_trans)
{
  uint8_t * buf = reinterpret_cast<uint8_t *>(&_binary_pk_maps_20kdm2_bsp_start);
  q3bsp_header_t * header = reinterpret_cast<q3bsp_header_t *>(buf);

  //debug_print_q3bsp(buf, header);
  //while(1);

  mat4x4 trans = screen_trans;

  q3bsp_direntry * ve = &header->direntries[LUMP_VERTEXES];
  transform_vertices(&buf[ve->offset], ve->length, trans);

  transfer_faces(buf, header, writer);

  writer.append<ta_global_parameter::end_of_list>() =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
}

uint8_t __attribute__((aligned(32))) ta_parameter_buf[1024 * 1024];

constexpr inline mat4x4 rotate_x(float t)
{
  mat4x4 r = {
    1, 0, 0, 0,
    0, cos(t), -sin(t), 0,
    0, sin(t), cos(t), 0,
    0, 0, 0, 1,
  };
  return r;
}

constexpr inline mat4x4 rotate_y(float t)
{
  mat4x4 r = {
     cos(t), 0, sin(t), 0,
    0, 1, 0, 0,
    -sin(t), 0, cos(t), 0,
    0, 0, 0, 1,
  };
  return r;
}

constexpr inline mat4x4 rotate_z(float t)
{
  mat4x4 r = {
    cos(t), -sin(t), 0, 0,
    sin(t), cos(t), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };
  return r;
}

void transfer_ta_fifo_texture_memory_32byte(void * dst, void * src, int length)
{
  uint32_t out_addr = (uint32_t)dst;
  sh7091.CCN.QACR0 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);
  sh7091.CCN.QACR1 = ((reinterpret_cast<uint32_t>(out_addr) >> 24) & 0b11100);

  volatile uint32_t * base = &store_queue[(out_addr & 0x03ffffc0) / 4];
  uint32_t * src32 = reinterpret_cast<uint32_t *>(src);

  length = (length + 31) & ~31; // round up to nearest multiple of 32
  while (length > 0) {
    base[0] = src32[0];
    base[1] = src32[1];
    base[2] = src32[2];
    base[3] = src32[3];
    base[4] = src32[4];
    base[5] = src32[5];
    base[6] = src32[6];
    base[7] = src32[7];
    asm volatile ("pref @%0"
                  :                // output
                  : "r" (&base[0]) // input
                  : "memory");
    length -= 32;
    base += 8;
    src32 += 8;
  }
}

void transfer_textures()
{
  system.LMMODE0 = 0; // 64-bit address space
  system.LMMODE1 = 0; // 64-bit address space

  int textures_length = (sizeof (textures)) / (sizeof (textures[0]));
  for (int i = 0; i < textures_length; i++) {
    uint32_t offset = texture_memory_alloc.texture.start + textures[i].offset * 2;
    void * dst = reinterpret_cast<void *>(&ta_fifo_texture_memory[offset / 4]);
    void * src = textures[i].start;
    uint32_t size = textures[i].size;
    transfer_ta_fifo_texture_memory_32byte(dst, src, size);
  }
}

int main()
{
  serial::init(0);

  interrupt_init();
  transfer_textures();

  constexpr uint32_t ta_alloc = 0
                              | ta_alloc_ctrl::pt_opb::no_list
			      | ta_alloc_ctrl::tm_opb::no_list
                              | ta_alloc_ctrl::t_opb::no_list
                              | ta_alloc_ctrl::om_opb::no_list
                              | ta_alloc_ctrl::o_opb::_16x4byte;

  constexpr int ta_cont_count = 1;
  constexpr struct opb_size opb_size[ta_cont_count] = {
    {
      .opaque = 16 * 4,
      .opaque_modifier = 0,
      .translucent = 0,
      .translucent_modifier = 0,
      .punch_through = 0
    }
  };

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  system.IML6NRM = istnrm::end_of_render_tsp;

  const int framebuffer_width = 640;
  const int framebuffer_height = 480;
  const int tile_width = framebuffer_width / 32;
  const int tile_height = framebuffer_height / 32;

  for (int i = 0; i < 2; i++) {
    region_array_multipass(tile_width,
                           tile_height,
                           opb_size,
                           ta_cont_count,
                           texture_memory_alloc.region_array[i].start,
                           texture_memory_alloc.object_list[i].start);

    background_parameter2(texture_memory_alloc.background[i].start,
                          0xff202040);
  }

  ta_parameter_writer writer = ta_parameter_writer(ta_parameter_buf);

  video_output::set_mode_vga();

  int ta = 0;
  int core = 0;

  mat4x4 screen_trans = {
    1,  0, 0, -1000,
    0,  1, 0, -1000,
    0,  0, 1, 1000,
    0,  0, 0, 1,
  };

  mat4x4 ztrans = {
    1,  0, 0, -1,
    0,  1, 0, -1,
    0,  0, 1, 1,
    0,  0, 0, 1,
  };

  while (1) {
    //screen_trans = ztrans * screen_trans;
    screen_trans = rotate_z(0.01) * screen_trans;

    ta_polygon_converter_init2(texture_memory_alloc.isp_tsp_parameters[ta].start,
			       texture_memory_alloc.isp_tsp_parameters[ta].end,
			       texture_memory_alloc.object_list[ta].start,
			       texture_memory_alloc.object_list[ta].end,
			       opb_size[0].total(),
			       ta_alloc,
			       tile_width,
			       tile_height);

    writer.offset = 0;
    transfer_scene(writer, screen_trans);
    ta_polygon_converter_writeback(writer.buf, writer.offset);
    ta_polygon_converter_transfer(writer.buf, writer.offset);
    ta_wait_opaque_list();

    render_done = 0;
    core_start_render2(texture_memory_alloc.region_array[core].start,
                       texture_memory_alloc.isp_tsp_parameters[core].start,
                       texture_memory_alloc.background[core].start,
                       texture_memory_alloc.framebuffer[core].start,
                       framebuffer_width);
    while (render_done == 0) {
      asm volatile ("nop");
    };

    while (spg_status::vsync(holly.SPG_STATUS));
    while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc.framebuffer[ta].start;
  }
}
