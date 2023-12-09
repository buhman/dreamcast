#include <cstdint>

namespace device_request {
  constexpr uint32_t command_code = 0x1;
  
  struct data_fields {
  };
}

namespace all_status_request {
  constexpr uint32_t command_code = 0x2;
  
  struct data_fields {
  };
}

namespace device_reset {
  constexpr uint32_t command_code = 0x3;
  
  struct data_fields {
  };
}

namespace device_kill {
  constexpr uint32_t command_code = 0x4;
  
  struct data_fields {
  };
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
  
  template <typename T>
  struct data_fields {
    uint8_t device_id[16];
    uint8_t destination_code[1];
    uint8_t connection_direction[1];
    uint8_t product_name[30];
    uint8_t license[60];
    uint8_t low_consumption_standby_current[2];
    uint8_t maximum_current_consumption[2];
    T free_device_status;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 112);
}

namespace device_reply {
  constexpr uint32_t command_code = 0x7;
  
  struct data_fields {
  };
}

namespace data_transfer {
  constexpr uint32_t command_code = 0x8;
  
  template <typename T>
  struct data_fields {
    uint8_t function_type[4];
    T data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
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
  
  template <typename T>
  struct data_fields {
    uint8_t function_type[4];
    uint8_t pt[1];
    uint8_t phase[1];
    uint8_t block_no[2];
    T written_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 8);
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
  
  template <typename T>
  struct data_fields {
    uint8_t function_type[4];
    T write_in_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
}

namespace ft4_control {
  constexpr uint32_t command_code = 0xf;
  
  template <typename T>
  struct data_fields {
    uint8_t function_type[4];
    T ft4_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
}

namespace ar_control {
  constexpr uint32_t command_code = 0x10;
  
  template <typename T>
  struct data_fields {
    uint8_t function_type[4];
    T data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
}

namespace function_type_unknown {
  constexpr uint32_t command_code = 0xfe;
  
  struct data_fields {
  };
}

namespace command_unknown {
  constexpr uint32_t command_code = 0xfd;
  
  struct data_fields {
  };
}

namespace transmit_again {
  constexpr uint32_t command_code = 0xfc;
  
  struct data_fields {
  };
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

