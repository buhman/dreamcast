namespace ft8 {
  namespace data_transfer {
    namespace vset {
      constexpr uint32_t vn() { return 0b1111 << 4; }
      constexpr uint32_t vn(uint32_t reg) { return (reg >> 4) & 0b1111; }
      
      constexpr uint32_t vp() { return 0b11 << 2; }
      constexpr uint32_t vp(uint32_t reg) { return (reg >> 2) & 0b11; }
      
      constexpr uint32_t vd() { return 0b11 << 0; }
      constexpr uint32_t vd(uint32_t reg) { return (reg >> 0) & 0b11; }
      
      constexpr uint32_t pf() { return 0b1 << 15; }
      constexpr uint32_t pf(uint32_t reg) { return (reg >> 15) & 0b1; }
      
      constexpr uint32_t cv() { return 0b1 << 14; }
      constexpr uint32_t cv(uint32_t reg) { return (reg >> 14) & 0b1; }
      
      constexpr uint32_t pd() { return 0b1 << 13; }
      constexpr uint32_t pd(uint32_t reg) { return (reg >> 13) & 0b1; }
      
      constexpr uint32_t owf() { return 0b1 << 12; }
      constexpr uint32_t owf(uint32_t reg) { return (reg >> 12) & 0b1; }
      
      constexpr uint32_t va() { return 0b1111 << 8; }
      constexpr uint32_t va(uint32_t reg) { return (reg >> 8) & 0b1111; }
      
    }
    
    struct data_format {
      uint16_t vset;
      uint16_t fm;
    };
    static_assert((sizeof (struct data_format)) == 4);
  }
}

