#include "memorymap.hpp"

#include "holly/isp_tsp.hpp"
#include "holly/ta_parameter.hpp"
#include "holly/ta_global_parameter.hpp"
#include "holly/ta_vertex_parameter.hpp"
#include "holly/ta_bits.hpp"
#include "holly/ta_fifo_polygon_converter.hpp"
#include "holly/holly.hpp"
#include "holly/core_bits.hpp"
#include "holly/core.hpp"
#include "holly/region_array.hpp"
#include "holly/background.hpp"
#include "holly/video_output.hpp"
#include "holly/texture_memory_alloc2.hpp"

#include "sh7091/store_queue.hpp"
#include "sh7091/serial.hpp"
#include "sh7091/sh7091.hpp"
#include "sh7091/sh7091_bits.hpp"

#include "systembus.hpp"
#include "systembus_bits.hpp"

#include "aica/aica.hpp"

#include "gdrom/gdrom.hpp"
#include "gdrom/gdrom_bits.hpp"
#include "gdrom/command_packet_format.hpp"
#include "gdrom/toc.hpp"

#include "iso9660/primary_volume_descriptor.hpp"
#include "iso9660/directory_record.hpp"

#include "math/fft.hpp"

#include "color_format.hpp"
#include "twiddle.hpp"

extern void * _binary_start __asm("_binary_example_arm_sh4_interrupt_bin_start");
extern void * _binary_size __asm("_binary_example_arm_sh4_interrupt_bin_size");

constexpr uint32_t mcipd__sh4_interrupt = (1 << 5);
constexpr uint32_t miceb__sh4_interrupt = (1 << 5);
constexpr uint32_t scipd__arm_interrupt = (1 << 5);

constexpr int32_t sectors_per_chunk = 16;
constexpr int32_t chunk_size = 2048 * sectors_per_chunk;
constexpr int32_t samples_per_chunk = chunk_size / 2;
constexpr int32_t samples_per_line = 1024;
constexpr int32_t lines_per_chunk = samples_per_chunk / (samples_per_line * 2);

struct vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

// screen space coordinates
const struct vertex v[4] = {
  { 64.f,  0.f, 0.1f, 0.0f, 0.0f },
  { 576.f,  0.f, 0.1f, 1.0f, 0.0f },
  { 576.f,  512.f, 0.1f, 1.0f, 1.0f / 2 },
  { 64.f,  512.f, 0.1f, 0.0f, 1.0f / 2 },
};

static int __x = 0;

void transfer_scene()
{
  const uint32_t parameter_control_word = para_control::para_type::sprite
                                        | para_control::list_type::opaque
                                        | obj_control::col_type::packed_color
                                        | obj_control::texture
                                        | obj_control::_16bit_uv;

  const uint32_t isp_tsp_instruction_word = isp_tsp_instruction_word::depth_compare_mode::greater
                                          | isp_tsp_instruction_word::culling_mode::no_culling;

  const uint32_t tsp_instruction_word = tsp_instruction_word::src_alpha_instr::one
				      | tsp_instruction_word::dst_alpha_instr::zero
				      | tsp_instruction_word::dst_select::primary_accumulation_buffer
				      | tsp_instruction_word::fog_control::no_fog
				      | tsp_instruction_word::texture_shading_instruction::modulate
				      | tsp_instruction_word::texture_u_size::from_int(samples_per_line)
				      | tsp_instruction_word::texture_v_size::from_int(samples_per_line)
                                      | tsp_instruction_word::filter_mode::point_sampled;

  const uint32_t texture_address = texture_memory_alloc::texture.start + 0;
  const uint32_t texture_control_word = texture_control_word::pixel_format::_8bpp_palette
                                      | texture_control_word::scan_order::twiddled
                                      | texture_control_word::texture_address(texture_address / 8);

  constexpr uint32_t base_color = 0xffffffff;
  *reinterpret_cast<ta_global_parameter::sprite *>(store_queue) =
    ta_global_parameter::sprite(parameter_control_word,
                                isp_tsp_instruction_word,
                                tsp_instruction_word,
                                texture_control_word,
                                base_color,
                                0,  // offset_color
                                0,  // data_size_for_sort_dma
                                0); // next_address_for_sort_dma
  sq_transfer_32byte(ta_fifo_polygon_converter);

  /*
  float u[3];
  for (int i = 0; i < 3; i++) {
    u[i] = v[i].u + (float)((((__x / 2) * 2) + 3) - (samples_per_line - 2)) / (float)(samples_per_line - 2);
    while (u[i] >= 1) u[i] -= 1;
  }
  */

  *reinterpret_cast<ta_vertex_parameter::sprite_type_1 *>(store_queue) =
    ta_vertex_parameter::sprite_type_1(para_control::para_type::vertex_parameter,
				       v[0].x,
				       v[0].y,
				       v[0].z,
				       v[1].x,
				       v[1].y,
				       v[1].z,
				       v[2].x,
				       v[2].y,
				       v[2].z,
				       v[3].x,
				       v[3].y,
				       uv_16bit(v[0].u, v[0].v),
				       uv_16bit(v[1].u, v[1].v),
				       uv_16bit(v[2].u, v[2].v)
				       );
  sq_transfer_64byte(ta_fifo_polygon_converter);

  *reinterpret_cast<ta_global_parameter::end_of_list *>(store_queue) =
    ta_global_parameter::end_of_list(para_control::para_type::end_of_list);
  sq_transfer_32byte(ta_fifo_polygon_converter);
}

void aica_wait_write()
{
  while (ffst::aica_internal_write_buffer(system.FFST));
}

void aica_wait_read()
{
  uint32_t ffst = system.FFST;
  while ( ffst::holly_cpu_if_block_internal_write_buffer(ffst)
        | ffst::holly_g2_if_block_internal_write_buffer(ffst)
        | ffst::aica_internal_write_buffer(ffst)) {
    ffst = system.FFST;
  };
}

void aica_fill_chunk(volatile uint32_t * dest_chunk, const uint32_t * src_chunk, const uint32_t size)
{
  for (uint32_t i = 0; i < size / 4; i++) {
    if (i % 8 == 0) aica_wait_write();
    dest_chunk[i] = src_chunk[i];
  }
}

static volatile uint32_t (* chunk)[2][chunk_size / 4];

void aica_init(uint32_t& chunk_index, const uint32_t * src_chunk)
{
  const uint32_t * binary = reinterpret_cast<uint32_t *>(&_binary_start);
  const uint32_t binary_size = reinterpret_cast<uint32_t>(&_binary_size);

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(1);
  aica_wait_write(); aica_sound.common.dmea0_mrwinh = aica::dmea0_mrwinh::MRWINH(0);
  for (uint32_t i = 0; i < binary_size / 4; i++) {
    // copy
    while (aica_wave_memory[i] != binary[i]) {
      aica_wait_write();
      aica_wave_memory[i] = binary[i];
    }
  }

  chunk = reinterpret_cast<decltype (chunk)>(&aica_wave_memory[0x00100000 / 4]);

  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[0][0]));
  serial::integer<uint32_t>(reinterpret_cast<uint32_t>(&(*chunk)[1][0]));

  aica_fill_chunk(&(*chunk)[chunk_index][0],
                  src_chunk,
                  chunk_size);
  chunk_index = (chunk_index + 1) % 2;

  aica_wait_write(); aica_sound.common.vreg_armrst = aica::vreg_armrst::ARMRST(0);

  { // send arm interrupt
    aica_sound.common.scipd = scipd__arm_interrupt;
  }
}

static int32_t step_time = -1;
static int32_t step_end = -1;
static int32_t step_start = -1;

extern "C"
void * memset(void * dest, int c, size_t n)
{
  unsigned char * s = (unsigned char *)dest;
  for (; n; n--, s++) *s = c;
  return dest;
}

static fft::complex comp[samples_per_line * 8];

int render_column(int col, int x, const uint32_t * buf)
{
  uint32_t offset = texture_memory_alloc::texture.start + 0;
  auto texture = reinterpret_cast<volatile uint32_t *>(&texture_memory64[offset / 4]);

  const int16_t * src = &((int16_t *)(buf))[col * samples_per_line / 2];
  if ((src) >= (int16_t*)&buf[(chunk_size - samples_per_line * 2)/4])
    return 0; // FIXME: this is a hack

  fft::int16_to_complex(src, samples_per_line * 2, comp);
  fft::fft(comp, samples_per_line * 2);
  for (int32_t y = 0; y < samples_per_line; y++) {
    const fft::complex& cx = comp[y];
    float abs = sqrt(cx.real * cx.real + cx.imag * cx.imag);
    int v = abs / (65536 / 8);
    if (v > 255) v = 255;
    if (v < 0) v = 0;
    uint32_t ix = twiddle::from_xy(x, y, samples_per_line, samples_per_line);
    uint32_t value = texture[ix / 4];
    int shift = (ix % 4) * 8;
    value &= ~(0xff << shift);
    value |= v << shift;
    texture[ix / 4] = value;
  }

  return 1;
}

void render();
void texture_init();

void aica_step(uint32_t& chunk_index, const uint32_t gdrom_buf[2][chunk_size / 4 * 2])
{
  { // wait for interrupt from arm
    int col = 0;
    while ((system.ISTEXT & (1 << 1)) == 0) {
      if (step_time >= 0) {
	int32_t dt = step_start - (int32_t)sh7091.TMU.TCNT0;
	if (step_time * col / (lines_per_chunk * 4) < dt) {
	  int inc = render_column(col, __x, gdrom_buf[!chunk_index]);
	  col += inc;
	  __x += inc;
	  __x %= samples_per_line;
	  if (col % 2)
	    render();
	}
      }
    };
    aica_wait_write(); aica_sound.common.mcire = mcipd__sh4_interrupt;
  }
  step_end = sh7091.TMU.TCNT0;
  step_time = step_start - step_end;
  serial::string("aica step time: ");
  serial::integer<uint32_t>(step_time);
  step_start = sh7091.TMU.TCNT0;

  { // fill the requested chunk
    aica_fill_chunk(&(*chunk)[chunk_index][0],
                    gdrom_buf[chunk_index],
                    chunk_size);

    chunk_index = (chunk_index + 1) % 2;
  }

  { // send arm interrupt
    aica_sound.common.scipd = scipd__arm_interrupt;
  }
}

// gdrom

void gdrom_pio_data(const uint8_t * data)
{
  while ((gdrom::status::bsy(gdrom_if.status) | gdrom::status::drq(gdrom_if.status)) != 0);

  gdrom_if.features = gdrom::features::dma::disable;
  gdrom_if.drive_select = gdrom::drive_select::drive_select
                        | gdrom::drive_select::lun(0);

  gdrom_if.command = gdrom::command::code::packet_command;
  while (gdrom::status::drq(gdrom_if.status) == 0);

  const uint16_t * buf = reinterpret_cast<const uint16_t *>(&data[0]);
  for (int i = 0; i < 6; i++) {
    gdrom_if.data = buf[i];
  }

  while (gdrom::status::bsy(gdrom_if.status) != 0);
}

void gdrom_read_data(uint16_t * buf, const uint32_t length)
{
  //serial::string("read_data drq interrupt_reason: ");
  //serial::integer<uint8_t>(gdrom::status::drq(gdrom_if.status), ' ');
  //serial::integer<uint8_t>(gdrom_if.interrupt_reason);
  for (uint32_t i = 0; i < (length / 2); i++) {
    buf[i] = gdrom_if.data;
  }
}

uint32_t gdrom_toc__get_data_track_fad()
{
  auto packet = gdrom_command_packet_format::get_toc(0,     // single-density
                                                     0x0198 // maximum toc length
                                                     );
  serial::string("get_toc\n");
  gdrom_pio_data(packet._data());

  serial::string("byte_count: ");
  serial::integer<uint16_t>(gdrom_if.byte_count());
  uint16_t buf[gdrom_if.byte_count() / 2];
  gdrom_read_data(buf, gdrom_if.byte_count());

  serial::string("status: ");
  serial::integer<uint8_t>(gdrom_if.status);

  auto toc = reinterpret_cast<const struct gdrom_toc::toc *>(buf);
  for (int i = 0; i < 99; i++) {
    if (toc->track[i].fad() == 0xffffff)
      break;
    serial::string("track ");
    serial::integer<uint8_t>(i);
    serial::integer<uint32_t>(toc->track[i].fad());
  }

  // assume track 1 is the correct track
  return toc->track[1].fad();
}

uint32_t gdrom_cd_read2(uint16_t * buf,
                        const uint32_t starting_address,
                        const uint32_t transfer_length,
                        const uint32_t next_address)
{
  const uint8_t data_select = 0b0010; // data
  const uint8_t expected_data_type = 0b100; // XA mode 2 form 1
  const uint8_t parameter_type = 0b0; // FAD specified
  const uint8_t data = (data_select << 4) | (expected_data_type << 1) | (parameter_type << 0);

  auto packet = gdrom_command_packet_format::cd_read2(data,
                                                      starting_address,
                                                      transfer_length,
                                                      next_address);
  //serial::string("cd_read\n");
  //serial::string("starting_address: ");
  //serial::integer<uint32_t>(starting_address);
  //serial::string("transfer_length: ");
  //serial::integer<uint32_t>(transfer_length);
  //serial::string("next_address: ");
  //serial::integer<uint32_t>(next_address);
  gdrom_pio_data(packet._data());

  while ((gdrom::status::bsy(gdrom_if.status)) != 0); // wait for drive to become not-busy

  uint32_t length = 0;
  while ((gdrom::status::drq(gdrom_if.status)) != 0) {
    const uint32_t byte_count = gdrom_if.byte_count();
    length += byte_count;
    gdrom_read_data(buf, byte_count);

    while ((gdrom::status::bsy(gdrom_if.status)) != 0); // wait for drive to become not-busy
  }

  return length;
}

void gdrom_unlock()
{
  // gdrom unlock undocumented register
  g1_if.GDUNLOCK = 0x1fffff;

  // Without this read from system_boot_rom, the read value of
  // gdrom_if.status is always 0xff
  for(uint32_t i = 0; i < 0x200000 / 4; i++) {
    (void)system_boot_rom[i];
  }
}

bool str_equal(const uint8_t * a,
               const uint32_t a_len,
               const char * b,
               const uint32_t b_len)
{
  if (a_len != b_len)
    return false;

  uint32_t len = a_len;

  while (len != 0) {
    if (*a++ != *b++)
      return false;

    len--;
  }

  return true;
}

struct extent
{
  const uint32_t location;
  const uint32_t data_length;
};

struct extent gdrom_find_file()
{
  const uint32_t fad = gdrom_toc__get_data_track_fad();
  serial::character('\n');

  const uint32_t primary_volume_descriptor = fad + 16;
  uint16_t buf[2048 / 2];
  gdrom_cd_read2(buf,
                 primary_volume_descriptor,     // starting address
                 1,                             // one sector; 2048 bytes
                 primary_volume_descriptor + 1  // next address
                 );
  serial::character('\n');

  auto pvd = reinterpret_cast<const iso9660::primary_volume_descriptor *>(&buf[0]);
  auto root_dr = reinterpret_cast<const iso9660::directory_record *>(&pvd->directory_record_for_root_directory[0]);

  serial::string("primary volume descriptor:\n");
  serial::string("  standard_identifier: ");
  serial::string(pvd->standard_identifier, 5);
  serial::character('\n');
  serial::string("  root directory record:\n");
  serial::string("    location of extent: ");
  serial::integer<uint32_t>(root_dr->location_of_extent.get());
  serial::string("    data length: ");
  serial::integer<uint32_t>(root_dr->data_length.get());

  serial::character('\n');

  const uint32_t root_directory_extent = root_dr->location_of_extent.get();
  gdrom_cd_read2(buf,
                 root_directory_extent + 150, // 150?
                 1, // one sector; 2048 bytes
                 root_directory_extent + 151  // 150?
                 );
  serial::character('\n');

  auto buf8 = reinterpret_cast<const uint8_t *>(buf);
  uint32_t offset = 0;
  while (true) {
    serial::string("directory entry offset: ");
    serial::integer<uint32_t>(offset);

    auto dr = reinterpret_cast<const iso9660::directory_record *>(&buf8[offset]);
    if (dr->length_of_directory_record == 0)
      break;

    serial::string("  length_of_directory_record: ");
    serial::integer<uint8_t>(dr->length_of_directory_record);
    serial::string("  length_of_file_identifier: ");
    serial::integer<uint8_t>(dr->length_of_file_identifier);
    serial::string("  file_identifier: ");
    serial::string(dr->file_identifier, dr->length_of_file_identifier);
    serial::character('\n');

    const char filename[] = "RIDDLE.PCM;1";
    bool equal = str_equal(dr->file_identifier, dr->length_of_file_identifier,
                           filename, (sizeof (filename)) - 1);

    if (dr->file_flags == 0) {
      serial::string("  location_of_extent: ");
      serial::integer<uint32_t>(dr->location_of_extent.get());
      serial::string("  data_length: ");
      serial::integer<uint32_t>(dr->data_length.get());

      if (equal) {
        serial::string("FOUND\n");
        return {
          dr->location_of_extent.get(),
          dr->data_length.get()
        };
      }
    }

    offset += dr->length_of_directory_record;
  }

  return { 0 , 0 };
}

void gdrom_read_chunk(uint32_t * buf, const uint32_t extent, const uint32_t num_extents)
{
  const uint32_t gdrom_start = sh7091.TMU.TCNT0;

  gdrom_cd_read2(reinterpret_cast<uint16_t *>(buf),
                 extent + 150, // 150?
                 num_extents,  // one sector; 2048 bytes
                 extent + 150 + num_extents  // 150?
                 );

  const uint32_t gdrom_end = sh7091.TMU.TCNT0;
  const uint32_t gdrom_time = gdrom_start - gdrom_end;
  serial::string("gdrom time: ");
  serial::integer<uint32_t>(gdrom_time);
}

void next_segment(const struct extent& extent, uint32_t& segment_index)
{
  segment_index += sectors_per_chunk;
  if ((segment_index * 2048) > extent.data_length)
    segment_index = 0;
}

void palette_init()
{
  holly.PAL_RAM_CTRL = pal_ram_ctrl::pixel_format::argb8888;

  for (int i = 0; i < 256; i++) {
    holly.PALETTE_RAM[i] = color_format::argb8888(255, i, i, i);
  }
}

void texture_init()
{
  uint32_t offset = texture_memory_alloc::texture.start + 0;
  auto texture = reinterpret_cast<volatile uint32_t *>(&texture_memory64[offset / 4]);
  for (int i = 0; i < samples_per_line * samples_per_line / 4; i++) {
    texture[i] = 0x0;
  }
}

constexpr uint32_t ta_alloc =
    ta_alloc_ctrl::pt_opb::_8x4byte
  | ta_alloc_ctrl::tm_opb::no_list
  | ta_alloc_ctrl::t_opb::no_list
  | ta_alloc_ctrl::om_opb::no_list
  | ta_alloc_ctrl::o_opb::no_list
  ;

constexpr int render_passes = 1;
constexpr struct opb_size opb_size[render_passes] = {
  {
    .opaque = 8 * 4,
    .opaque_modifier = 0,
    .translucent = 0,
    .translucent_modifier = 0,
    .punch_through = 0
  }
};


constexpr int framebuffer_width = 640;
constexpr int framebuffer_height = 480;
constexpr int tile_width = framebuffer_width / 32;
constexpr int tile_height = framebuffer_height / 32;

void render()
{
  static int ta = -1;
  static int core = -2;
  if (core >= 0) {
    // core = 0  ; core = 1
    // ta = 1    ; ta = 0
    core_wait_end_of_render_video();
    //while (!spg_status::vsync(holly.SPG_STATUS));
    holly.FB_R_SOF1 = texture_memory_alloc::framebuffer[core].start;
  }

  // core = -2 ; core = 1 ; core = 0
  // ta = -1   ; ta = 0   ; ta = 1
  core += 1;
  ta += 1;
  if (core > 1) core = 0;
  if (ta > 1) ta = 0;

  if (core >= 0) {
    // core = 1 ; core = 0
    // ta = 0   ; ta = 1
    ta_wait_opaque_list();
    core_start_render2(texture_memory_alloc::region_array[core].start,
		       texture_memory_alloc::isp_tsp_parameters[core].start,
		       texture_memory_alloc::background[core].start,
		       texture_memory_alloc::framebuffer[core].start,
		       framebuffer_width);
  }


  // core = -1 ; core = 1 ; core = 0
  // ta = 0    ; ta = 0   ; ta = 1
  ta_polygon_converter_init2(texture_memory_alloc::isp_tsp_parameters[ta].start,
			     texture_memory_alloc::isp_tsp_parameters[ta].end,
			     texture_memory_alloc::object_list[ta].start,
			     texture_memory_alloc::object_list[ta].end,
			     opb_size[0].total(),
			     ta_alloc,
			     tile_width,
			     tile_height);
  transfer_scene();
}

void main()
{
  memset(&comp, 0, (sizeof (comp)));

  serial::init(0);

  holly.SOFTRESET = softreset::pipeline_soft_reset
		  | softreset::ta_soft_reset;
  holly.SOFTRESET = 0;

  core_init();

  video_output::set_mode_vga();

  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array[0].start,
			 texture_memory_alloc::object_list[0].start);
  region_array_multipass(tile_width,
			 tile_height,
			 opb_size,
			 render_passes,
			 texture_memory_alloc::region_array[1].start,
			 texture_memory_alloc::object_list[1].start);

  background_parameter2(texture_memory_alloc::background[0].start,
			0x00220033);
  background_parameter2(texture_memory_alloc::background[1].start,
			0x00220033);

  palette_init();
  texture_init();

  sh7091.TMU.TSTR = 0; // stop all timers
  sh7091.TMU.TOCR = tmu::tocr::tcoe::tclk_is_external_clock_or_input_capture;
  sh7091.TMU.TCR0 = tmu::tcr0::tpsc::p_phi_256; // 256 / 50MHz = 5.12 μs ; underflows in ~1 hour
  sh7091.TMU.TCOR0 = 0xffff'ffff;
  sh7091.TMU.TCNT0 = 0xffff'ffff;
  sh7091.TMU.TSTR = tmu::tstr::str0::counter_start;

  uint32_t chunk_index = 0;
  uint32_t segment_index = 0;

  gdrom_unlock();
  const auto extent = gdrom_find_file();
  uint32_t gdrom_buf[2][(chunk_size / 4) * 2] = {0};
  gdrom_read_chunk(gdrom_buf[chunk_index], extent.location + segment_index, sectors_per_chunk);
  next_segment(extent, segment_index);

  aica_init(chunk_index, gdrom_buf[chunk_index]);

  aica_sound.common.MCIEB(miceb__sh4_interrupt);

  //render();
  //render();
  //render();

  while (1) {
    gdrom_read_chunk(gdrom_buf[chunk_index], extent.location + segment_index, sectors_per_chunk);
    next_segment(extent, segment_index);

    aica_step(chunk_index, gdrom_buf);
  }

  while (1);
}
