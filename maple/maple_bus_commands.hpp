#pragma once

#include <cstdint>

struct device_id {
  uint32_t ft;
  uint32_t fd[3];
};

static_assert((sizeof (struct device_id)) == 16);
struct device_request {
  static constexpr uint32_t command_code = 0x1;
  
  struct data_fields {
  };
};


struct all_status_request {
  static constexpr uint32_t command_code = 0x2;
  
  struct data_fields {
  };
};


struct device_reset {
  static constexpr uint32_t command_code = 0x3;
  
  struct data_fields {
  };
};


struct device_kill {
  static constexpr uint32_t command_code = 0x4;
  
  struct data_fields {
  };
};


struct device_status {
  static constexpr uint32_t command_code = 0x5;
  
  struct data_fields {
    struct device_id device_id;
    uint8_t destination_code;
    uint8_t connection_direction;
    uint8_t product_name[30];
    uint8_t license[60];
    uint16_t low_consumption_standby_current;
    uint16_t maximum_current_consumption;
  };
  
  static_assert((sizeof (struct data_fields)) == 112);
};


struct device_all_status {
  static constexpr uint32_t command_code = 0x6;
  
  template <typename T>
  struct data_fields {
    struct device_id device_id;
    uint8_t destination_code;
    uint8_t connection_direction;
    uint8_t product_name[30];
    uint8_t license[60];
    uint16_t low_consumption_standby_current;
    uint16_t maximum_current_consumption;
    T free_device_status;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 112);
};


struct device_reply {
  static constexpr uint32_t command_code = 0x7;
  
  struct data_fields {
  };
};

template <typename T>
struct data_transfer {
  static constexpr uint32_t command_code = 0x8;

  struct data_fields {
    uint32_t function_type;
    T data;
  };
};
static_assert((sizeof (struct data_transfer<uint8_t[0]>::data_fields)) == 4);

struct get_condition {
  static constexpr uint32_t command_code = 0x9;
  
  struct data_fields {
    uint32_t function_type;
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
};


struct get_media_info {
  static constexpr uint32_t command_code = 0xa;
  
  struct data_fields {
    uint32_t function_type;
    uint32_t pt;
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
};


struct block_read {
  static constexpr uint32_t command_code = 0xb;
  
  struct data_fields {
    uint32_t function_type;
    uint8_t pt;
    uint8_t phase;
    uint16_t block_no;
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
};


struct block_write {
  static constexpr uint32_t command_code = 0xc;
  
  template <typename T>
  struct data_fields {
    uint32_t function_type;
    uint8_t pt;
    uint8_t phase;
    uint16_t block_no;
    T written_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 8);
};


struct get_last_error {
  static constexpr uint32_t command_code = 0xd;
  
  struct data_fields {
    uint32_t function_type;
    uint8_t pt;
    uint8_t phase;
    uint16_t block_no;
  };
  
  static_assert((sizeof (struct data_fields)) == 8);
};


struct set_condition {
  static constexpr uint32_t command_code = 0xe;
  
  template <typename T>
  struct data_fields {
    uint32_t function_type;
    T write_in_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
};


struct ft4_control {
  static constexpr uint32_t command_code = 0xf;
  
  template <typename T>
  struct data_fields {
    uint32_t function_type;
    T ft4_data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
};


struct ar_control {
  static constexpr uint32_t command_code = 0x10;
  
  template <typename T>
  struct data_fields {
    uint32_t function_type;
    T data;
  };
  
  static_assert((sizeof (struct data_fields<char[0]>)) == 4);
};


struct function_type_unknown {
  static constexpr uint32_t command_code = 0xfe;
  
  struct data_fields {
  };
};


struct command_unknown {
  static constexpr uint32_t command_code = 0xfd;
  
  struct data_fields {
  };
};


struct transmit_again {
  static constexpr uint32_t command_code = 0xfc;
  
  struct data_fields {
  };
};


struct file_error {
  static constexpr uint32_t command_code = 0xfb;
  
  struct data_fields {
    uint32_t function_error_code;
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
};


struct lcd_error {
  static constexpr uint32_t command_code = 0xfa;
  
  struct data_fields {
    uint32_t function_error_code;
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
};


struct ar_error {
  static constexpr uint32_t command_code = 0xf9;
  
  struct data_fields {
    uint32_t function_error_code;
  };
  
  static_assert((sizeof (struct data_fields)) == 4);
};


