#pragma once

#include <cstdint>
#include <cstddef>

#include "command_packet_format_byte_order.hpp"

namespace gdrom_command_packet_format {
  struct test_unit {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;
    uint8_t _res9;
    uint8_t _res10;

    test_unit()
      : command_code(0x0)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
      , _res9(0)
      , _res10(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (test_unit)) == 12);
  static_assert((offsetof (struct test_unit, command_code)) == 0x00);
  static_assert((offsetof (struct test_unit, _res0)) == 0x01);
  static_assert((offsetof (struct test_unit, _res1)) == 0x02);
  static_assert((offsetof (struct test_unit, _res2)) == 0x03);
  static_assert((offsetof (struct test_unit, _res3)) == 0x04);
  static_assert((offsetof (struct test_unit, _res4)) == 0x05);
  static_assert((offsetof (struct test_unit, _res5)) == 0x06);
  static_assert((offsetof (struct test_unit, _res6)) == 0x07);
  static_assert((offsetof (struct test_unit, _res7)) == 0x08);
  static_assert((offsetof (struct test_unit, _res8)) == 0x09);
  static_assert((offsetof (struct test_unit, _res9)) == 0x0a);
  static_assert((offsetof (struct test_unit, _res10)) == 0x0b);

  struct req_stat {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t starting_address;
    uint8_t _res1;
    uint8_t allocation_length;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;

    req_stat(const uint8_t starting_address,
             const uint8_t allocation_length
             )
      : command_code(0x10)
      , _res0(0)
      , starting_address(starting_address)
      , _res1(0)
      , allocation_length(allocation_length)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (req_stat)) == 12);
  static_assert((offsetof (struct req_stat, command_code)) == 0x00);
  static_assert((offsetof (struct req_stat, _res0)) == 0x01);
  static_assert((offsetof (struct req_stat, starting_address)) == 0x02);
  static_assert((offsetof (struct req_stat, _res1)) == 0x03);
  static_assert((offsetof (struct req_stat, allocation_length)) == 0x04);
  static_assert((offsetof (struct req_stat, _res2)) == 0x05);
  static_assert((offsetof (struct req_stat, _res3)) == 0x06);
  static_assert((offsetof (struct req_stat, _res4)) == 0x07);
  static_assert((offsetof (struct req_stat, _res5)) == 0x08);
  static_assert((offsetof (struct req_stat, _res6)) == 0x09);
  static_assert((offsetof (struct req_stat, _res7)) == 0x0a);
  static_assert((offsetof (struct req_stat, _res8)) == 0x0b);

  struct req_mode {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t starting_address;
    uint8_t _res1;
    uint8_t allocation_length;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;

    req_mode(const uint8_t starting_address,
             const uint8_t allocation_length
             )
      : command_code(0x11)
      , _res0(0)
      , starting_address(starting_address)
      , _res1(0)
      , allocation_length(allocation_length)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (req_mode)) == 12);
  static_assert((offsetof (struct req_mode, command_code)) == 0x00);
  static_assert((offsetof (struct req_mode, _res0)) == 0x01);
  static_assert((offsetof (struct req_mode, starting_address)) == 0x02);
  static_assert((offsetof (struct req_mode, _res1)) == 0x03);
  static_assert((offsetof (struct req_mode, allocation_length)) == 0x04);
  static_assert((offsetof (struct req_mode, _res2)) == 0x05);
  static_assert((offsetof (struct req_mode, _res3)) == 0x06);
  static_assert((offsetof (struct req_mode, _res4)) == 0x07);
  static_assert((offsetof (struct req_mode, _res5)) == 0x08);
  static_assert((offsetof (struct req_mode, _res6)) == 0x09);
  static_assert((offsetof (struct req_mode, _res7)) == 0x0a);
  static_assert((offsetof (struct req_mode, _res8)) == 0x0b);

  struct set_mode {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t starting_address;
    uint8_t _res1;
    uint8_t allocation_length;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;

    set_mode(const uint8_t starting_address,
             const uint8_t allocation_length
             )
      : command_code(0x12)
      , _res0(0)
      , starting_address(starting_address)
      , _res1(0)
      , allocation_length(allocation_length)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (set_mode)) == 12);
  static_assert((offsetof (struct set_mode, command_code)) == 0x00);
  static_assert((offsetof (struct set_mode, _res0)) == 0x01);
  static_assert((offsetof (struct set_mode, starting_address)) == 0x02);
  static_assert((offsetof (struct set_mode, _res1)) == 0x03);
  static_assert((offsetof (struct set_mode, allocation_length)) == 0x04);
  static_assert((offsetof (struct set_mode, _res2)) == 0x05);
  static_assert((offsetof (struct set_mode, _res3)) == 0x06);
  static_assert((offsetof (struct set_mode, _res4)) == 0x07);
  static_assert((offsetof (struct set_mode, _res5)) == 0x08);
  static_assert((offsetof (struct set_mode, _res6)) == 0x09);
  static_assert((offsetof (struct set_mode, _res7)) == 0x0a);
  static_assert((offsetof (struct set_mode, _res8)) == 0x0b);

  struct req_error {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t allocation_length;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;
    uint8_t _res9;

    req_error(const uint8_t allocation_length
              )
      : command_code(0x13)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , allocation_length(allocation_length)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
      , _res9(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (req_error)) == 12);
  static_assert((offsetof (struct req_error, command_code)) == 0x00);
  static_assert((offsetof (struct req_error, _res0)) == 0x01);
  static_assert((offsetof (struct req_error, _res1)) == 0x02);
  static_assert((offsetof (struct req_error, _res2)) == 0x03);
  static_assert((offsetof (struct req_error, allocation_length)) == 0x04);
  static_assert((offsetof (struct req_error, _res3)) == 0x05);
  static_assert((offsetof (struct req_error, _res4)) == 0x06);
  static_assert((offsetof (struct req_error, _res5)) == 0x07);
  static_assert((offsetof (struct req_error, _res6)) == 0x08);
  static_assert((offsetof (struct req_error, _res7)) == 0x09);
  static_assert((offsetof (struct req_error, _res8)) == 0x0a);
  static_assert((offsetof (struct req_error, _res9)) == 0x0b);

  struct get_toc {
    uint8_t command_code;
    uint8_t select;
    uint8_t _res0;
    uint8_t allocation_length[2];
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;

    get_toc(const uint8_t select,
            const uint32_t allocation_length
            )
      : command_code(0x14)
      , select(select)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
    {
      byte_order<2>(&this->allocation_length[0], allocation_length);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (get_toc)) == 12);
  static_assert((offsetof (struct get_toc, command_code)) == 0x00);
  static_assert((offsetof (struct get_toc, select)) == 0x01);
  static_assert((offsetof (struct get_toc, _res0)) == 0x02);
  static_assert((offsetof (struct get_toc, allocation_length)) == 0x03);
  static_assert((offsetof (struct get_toc, _res1)) == 0x05);
  static_assert((offsetof (struct get_toc, _res2)) == 0x06);
  static_assert((offsetof (struct get_toc, _res3)) == 0x07);
  static_assert((offsetof (struct get_toc, _res4)) == 0x08);
  static_assert((offsetof (struct get_toc, _res5)) == 0x09);
  static_assert((offsetof (struct get_toc, _res6)) == 0x0a);
  static_assert((offsetof (struct get_toc, _res7)) == 0x0b);

  struct req_ses {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t session_number;
    uint8_t _res1;
    uint8_t allocation_length;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;

    req_ses(const uint8_t session_number,
            const uint8_t allocation_length
            )
      : command_code(0x15)
      , _res0(0)
      , session_number(session_number)
      , _res1(0)
      , allocation_length(allocation_length)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (req_ses)) == 12);
  static_assert((offsetof (struct req_ses, command_code)) == 0x00);
  static_assert((offsetof (struct req_ses, _res0)) == 0x01);
  static_assert((offsetof (struct req_ses, session_number)) == 0x02);
  static_assert((offsetof (struct req_ses, _res1)) == 0x03);
  static_assert((offsetof (struct req_ses, allocation_length)) == 0x04);
  static_assert((offsetof (struct req_ses, _res2)) == 0x05);
  static_assert((offsetof (struct req_ses, _res3)) == 0x06);
  static_assert((offsetof (struct req_ses, _res4)) == 0x07);
  static_assert((offsetof (struct req_ses, _res5)) == 0x08);
  static_assert((offsetof (struct req_ses, _res6)) == 0x09);
  static_assert((offsetof (struct req_ses, _res7)) == 0x0a);
  static_assert((offsetof (struct req_ses, _res8)) == 0x0b);

  struct cd_open {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;
    uint8_t _res9;
    uint8_t _res10;

    cd_open()
      : command_code(0x16)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
      , _res9(0)
      , _res10(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_open)) == 12);
  static_assert((offsetof (struct cd_open, command_code)) == 0x00);
  static_assert((offsetof (struct cd_open, _res0)) == 0x01);
  static_assert((offsetof (struct cd_open, _res1)) == 0x02);
  static_assert((offsetof (struct cd_open, _res2)) == 0x03);
  static_assert((offsetof (struct cd_open, _res3)) == 0x04);
  static_assert((offsetof (struct cd_open, _res4)) == 0x05);
  static_assert((offsetof (struct cd_open, _res5)) == 0x06);
  static_assert((offsetof (struct cd_open, _res6)) == 0x07);
  static_assert((offsetof (struct cd_open, _res7)) == 0x08);
  static_assert((offsetof (struct cd_open, _res8)) == 0x09);
  static_assert((offsetof (struct cd_open, _res9)) == 0x0a);
  static_assert((offsetof (struct cd_open, _res10)) == 0x0b);

  struct cd_play {
    uint8_t command_code;
    uint8_t parameter_type;
    uint8_t starting_point[3];
    uint8_t _res0;
    uint8_t repeat_times;
    uint8_t _res1;
    uint8_t end_point[3];
    uint8_t _res2;

    cd_play(const uint8_t parameter_type,
            const uint32_t starting_point,
            const uint8_t repeat_times,
            const uint32_t end_point
            )
      : command_code(0x20)
      , parameter_type(parameter_type)
      , _res0(0)
      , repeat_times(repeat_times)
      , _res1(0)
      , _res2(0)
    {
      byte_order<3>(&this->starting_point[0], starting_point);
      byte_order<3>(&this->end_point[0], end_point);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_play)) == 12);
  static_assert((offsetof (struct cd_play, command_code)) == 0x00);
  static_assert((offsetof (struct cd_play, parameter_type)) == 0x01);
  static_assert((offsetof (struct cd_play, starting_point)) == 0x02);
  static_assert((offsetof (struct cd_play, _res0)) == 0x05);
  static_assert((offsetof (struct cd_play, repeat_times)) == 0x06);
  static_assert((offsetof (struct cd_play, _res1)) == 0x07);
  static_assert((offsetof (struct cd_play, end_point)) == 0x08);
  static_assert((offsetof (struct cd_play, _res2)) == 0x0b);

  struct cd_seek {
    uint8_t command_code;
    uint8_t parameter_type;
    uint8_t seek_point[3];
    uint8_t _res0;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;

    cd_seek(const uint8_t parameter_type,
            const uint32_t seek_point
            )
      : command_code(0x21)
      , parameter_type(parameter_type)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
    {
      byte_order<3>(&this->seek_point[0], seek_point);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_seek)) == 12);
  static_assert((offsetof (struct cd_seek, command_code)) == 0x00);
  static_assert((offsetof (struct cd_seek, parameter_type)) == 0x01);
  static_assert((offsetof (struct cd_seek, seek_point)) == 0x02);
  static_assert((offsetof (struct cd_seek, _res0)) == 0x05);
  static_assert((offsetof (struct cd_seek, _res1)) == 0x06);
  static_assert((offsetof (struct cd_seek, _res2)) == 0x07);
  static_assert((offsetof (struct cd_seek, _res3)) == 0x08);
  static_assert((offsetof (struct cd_seek, _res4)) == 0x09);
  static_assert((offsetof (struct cd_seek, _res5)) == 0x0a);
  static_assert((offsetof (struct cd_seek, _res6)) == 0x0b);

  struct cd_scan {
    uint8_t command_code;
    uint8_t _res0;
    uint8_t direction;
    uint8_t speed;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;
    uint8_t _res8;

    cd_scan(const uint8_t direction,
            const uint8_t speed
            )
      : command_code(0x22)
      , _res0(0)
      , direction(direction)
      , speed(speed)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
      , _res8(0)
    { }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_scan)) == 12);
  static_assert((offsetof (struct cd_scan, command_code)) == 0x00);
  static_assert((offsetof (struct cd_scan, _res0)) == 0x01);
  static_assert((offsetof (struct cd_scan, direction)) == 0x02);
  static_assert((offsetof (struct cd_scan, speed)) == 0x03);
  static_assert((offsetof (struct cd_scan, _res1)) == 0x04);
  static_assert((offsetof (struct cd_scan, _res2)) == 0x05);
  static_assert((offsetof (struct cd_scan, _res3)) == 0x06);
  static_assert((offsetof (struct cd_scan, _res4)) == 0x07);
  static_assert((offsetof (struct cd_scan, _res5)) == 0x08);
  static_assert((offsetof (struct cd_scan, _res6)) == 0x09);
  static_assert((offsetof (struct cd_scan, _res7)) == 0x0a);
  static_assert((offsetof (struct cd_scan, _res8)) == 0x0b);

  struct cd_read {
    uint8_t command_code;
    uint8_t data;
    uint8_t starting_address[3];
    uint8_t _res0;
    uint8_t _res1;
    uint8_t _res2;
    uint8_t transfer_length[3];
    uint8_t _res3;

    cd_read(const uint8_t data,
            const uint32_t starting_address,
            const uint32_t transfer_length
            )
      : command_code(0x30)
      , data(data)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
    {
      byte_order<3>(&this->starting_address[0], starting_address);
      byte_order<3>(&this->transfer_length[0], transfer_length);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_read)) == 12);
  static_assert((offsetof (struct cd_read, command_code)) == 0x00);
  static_assert((offsetof (struct cd_read, data)) == 0x01);
  static_assert((offsetof (struct cd_read, starting_address)) == 0x02);
  static_assert((offsetof (struct cd_read, _res0)) == 0x05);
  static_assert((offsetof (struct cd_read, _res1)) == 0x06);
  static_assert((offsetof (struct cd_read, _res2)) == 0x07);
  static_assert((offsetof (struct cd_read, transfer_length)) == 0x08);
  static_assert((offsetof (struct cd_read, _res3)) == 0x0b);

  struct cd_read2 {
    uint8_t command_code;
    uint8_t data;
    uint8_t starting_address[3];
    uint8_t _res0;
    uint8_t transfer_length[2];
    uint8_t next_address[3];
    uint8_t _res1;

    cd_read2(const uint8_t data,
             const uint32_t starting_address,
             const uint32_t transfer_length,
             const uint32_t next_address
             )
      : command_code(0x31)
      , data(data)
      , _res0(0)
      , _res1(0)
    {
      byte_order<3>(&this->starting_address[0], starting_address);
      byte_order<2>(&this->transfer_length[0], transfer_length);
      byte_order<3>(&this->next_address[0], next_address);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_read2)) == 12);
  static_assert((offsetof (struct cd_read2, command_code)) == 0x00);
  static_assert((offsetof (struct cd_read2, data)) == 0x01);
  static_assert((offsetof (struct cd_read2, starting_address)) == 0x02);
  static_assert((offsetof (struct cd_read2, _res0)) == 0x05);
  static_assert((offsetof (struct cd_read2, transfer_length)) == 0x06);
  static_assert((offsetof (struct cd_read2, next_address)) == 0x08);
  static_assert((offsetof (struct cd_read2, _res1)) == 0x0b);

  struct cd_scd {
    uint8_t command_code;
    uint8_t data_format;
    uint8_t _res0;
    uint8_t allocation_length[2];
    uint8_t _res1;
    uint8_t _res2;
    uint8_t _res3;
    uint8_t _res4;
    uint8_t _res5;
    uint8_t _res6;
    uint8_t _res7;

    cd_scd(const uint8_t data_format,
           const uint32_t allocation_length
           )
      : command_code(0x40)
      , data_format(data_format)
      , _res0(0)
      , _res1(0)
      , _res2(0)
      , _res3(0)
      , _res4(0)
      , _res5(0)
      , _res6(0)
      , _res7(0)
    {
      byte_order<2>(&this->allocation_length[0], allocation_length);
    }

    const uint8_t * _data()
    {
      return reinterpret_cast<const uint8_t *>(this);
    }
  };
  static_assert((sizeof (cd_scd)) == 12);
  static_assert((offsetof (struct cd_scd, command_code)) == 0x00);
  static_assert((offsetof (struct cd_scd, data_format)) == 0x01);
  static_assert((offsetof (struct cd_scd, _res0)) == 0x02);
  static_assert((offsetof (struct cd_scd, allocation_length)) == 0x03);
  static_assert((offsetof (struct cd_scd, _res1)) == 0x05);
  static_assert((offsetof (struct cd_scd, _res2)) == 0x06);
  static_assert((offsetof (struct cd_scd, _res3)) == 0x07);
  static_assert((offsetof (struct cd_scd, _res4)) == 0x08);
  static_assert((offsetof (struct cd_scd, _res5)) == 0x09);
  static_assert((offsetof (struct cd_scd, _res6)) == 0x0a);
  static_assert((offsetof (struct cd_scd, _res7)) == 0x0b);

}

