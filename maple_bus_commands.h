#include <stdint.h>
#include <stddef.h>

#include "type.h"

namespace device_request {
  constexpr uint32_t command_code = 0x1;
}

namespace all_status_request {
  constexpr uint32_t command_code = 0x2;
}

namespace device_reset {
  constexpr uint32_t command_code = 0x3;
}

namespace device_kill {
  constexpr uint32_t command_code = 0x4;
}

namespace device_status {
  constexpr uint32_t command_code = 0x5;
  
  struct data_fields {
    uint8_t device_id[16];
    uint8_t destination_code[1];
    uint8_t connection_direction[1];
    uint8_t product_name[30];
    uint8_t license[60];
    uint8_t low_consumption_standby_current[2];
    uint8_t maximum_current_consumption[2];
  };
  
  static_assert((sizeof (struct data_fields)) == 112);
}

namespace device_all_status {
  constexpr uint32_t command_code = 0x6;
  
  template <int N>
  struct data_fields {
    uint8_t device_id[16];
    uint8_t destination_code[1];
    uint8_t connection_direction[1];
    uint8_t product_name[30];
    uint8_t license[60];
    uint8_t low_consumption_standby_current[2];
    uint8_t maximum_current_consumption[2];
    uint8_t free_device_status[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 112);
}

namespace device_reply {
  constexpr uint32_t command_code = 0x7;
}

namespace data_transfer {
  constexpr uint32_t command_code = 0x8;
  
  template <int N>
  struct data_fields {
    uint8_t data[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 0);
}

namespace get_condition {
  constexpr uint32_t command_code = 0x9;
  
  struct data_fields {
    uint8_t function_type[4];
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
}

namespace get_media_info {
  constexpr uint32_t command_code = 0xa;
  
  struct data_fields {
    uint8_t function_type[4];
    uint8_t pt[4];
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
}

namespace block_read {
  constexpr uint32_t command_code = 0xb;
  
  struct data_fields {
    uint8_t function_type[4];
    uint8_t pt[1];
    uint8_t phase[1];
    uint8_t block_no[2];
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
}

namespace block_write {
  constexpr uint32_t command_code = 0xc;
  
  template <int N>
  struct data_fields {
    uint8_t function_type[4];
    uint8_t pt[1];
    uint8_t phase[1];
    uint8_t block_no[2];
    uint8_t written_data[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 8);
}

namespace get_last_error {
  constexpr uint32_t command_code = 0xd;
  
  struct data_fields {
    uint8_t function_type[4];
    uint8_t pt[1];
    uint8_t phase[1];
    uint8_t block_no[2];
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
}

namespace set_condition {
  constexpr uint32_t command_code = 0xe;
  
  template <int N>
  struct data_fields {
    uint8_t function_type[4];
    uint8_t write_in_data[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 4);
}

namespace ft4_control {
  constexpr uint32_t command_code = 0xf;
  
  template <int N>
  struct data_fields {
    uint8_t function_type[4];
    uint8_t ft4_data[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 4);
}

namespace ar_control {
  constexpr uint32_t command_code = 0x10;
  
  template <int N>
  struct data_fields {
    uint8_t function_type[4];
    uint8_t data[N];
  };
  
  static_assert((sizeof (struct data_fields<0>)) == 4);
}

namespace function_type_unknown {
  constexpr uint32_t command_code = 0xfe;
}

namespace command_unknown {
  constexpr uint32_t command_code = 0xfd;
}

namespace transmit_again {
  constexpr uint32_t command_code = 0xfc;
}

namespace file_error {
  constexpr uint32_t command_code = 0xfb;
  
  struct data_fields {
    uint8_t function_error_code[4];
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
}

namespace lcd_error {
  constexpr uint32_t command_code = 0xfa;
  
  struct data_fields {
    uint8_t function_error_code[4];
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
}

namespace ar_error {
  constexpr uint32_t command_code = 0xf9;
  
  struct data_fields {
    uint8_t function_error_code[4];
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
}

