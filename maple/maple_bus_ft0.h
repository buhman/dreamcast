namespace ft0 {
  namespace data_transfer {
    namespace digital_button {
      constexpr uint32_t ra = 1 << 7;
      constexpr uint32_t la = 1 << 6;
      constexpr uint32_t da = 1 << 5;
      constexpr uint32_t ua = 1 << 4;
      constexpr uint32_t start = 1 << 3;
      constexpr uint32_t a = 1 << 2;
      constexpr uint32_t b = 1 << 1;
      constexpr uint32_t c = 1 << 0;
      constexpr uint32_t rb = 1 << 15;
      constexpr uint32_t lb = 1 << 14;
      constexpr uint32_t db = 1 << 13;
      constexpr uint32_t ub = 1 << 12;
      constexpr uint32_t d = 1 << 11;
      constexpr uint32_t x = 1 << 10;
      constexpr uint32_t y = 1 << 9;
      constexpr uint32_t z = 1 << 8;
    }
    
    struct data_format {
      uint16_t digital_button;
      uint8_t analog_axis_1;
      uint8_t analog_axis_2;
      uint8_t analog_axis_3;
      uint8_t analog_axis_4;
      uint8_t analog_axis_5;
      uint8_t analog_axis_6;
    };
    static_assert((sizeof (struct data_format)) == 8);
  }
}

