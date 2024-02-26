#include <cstdint>

namespace gdrom {

namespace status {

constexpr uint8_t bsy = (1 << 7);
constexpr uint8_t drdy = (1 << 6);
constexpr uint8_t df = (1 << 5);
constexpr uint8_t dsc = (1 << 4);
constexpr uint8_t drq = (1 << 3);
constexpr uint8_t corr = (1 << 2);
constexpr uint8_t check = (1 << 0);

}

namespace interrupt_reason {

constexpr uint8_t io = (1 << 1);
constexpr uint8_t cod = (1 << 0);

}

namespace command {

constexpr uint8_t test_unit = 0x00;
constexpr uint8_t req_stat = 0x10;
constexpr uint8_t req_mode = 0x11;
constexpr uint8_t set_mode = 0x12;
constexpr uint8_t req_error = 0x13;
constexpr uint8_t get_toc = 0x14;
constexpr uint8_t req_ses = 0x15;
constexpr uint8_t cd_open = 0x16;
constexpr uint8_t cd_play = 0x20;
constexpr uint8_t cd_seek = 0x21;
constexpr uint8_t cd_scan = 0x22;
constexpr uint8_t cd_read = 0x30;
constexpr uint8_t cd_read2 = 0x31;
constexpr uint8_t get_scd = 0x40;

}

}
