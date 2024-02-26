#include <cstdint>

namespace gdrom_toc {

struct track {
  const uint8_t _control_adr;
  const uint8_t _fad[3];

  uint32_t fad() const
  {
    return (_fad[0] << 16) | (_fad[1] << 8) | (_fad[2] << 0);
  }

  uint8_t control() const
  {
    return (_control_adr >> 4) & 0xf;
  }

  uint8_t adr() const
  {
    return (_control_adr >> 0) & 0xf;
  }
};
static_assert((sizeof (track)) == 4);

struct start_track {
  const uint8_t _control_adr;
  const uint8_t start_track_number;
  const uint8_t _zero[2];

  uint8_t control() const
  {
    return (_control_adr >> 4) & 0xf;
  }

  uint8_t adr() const
  {
    return (_control_adr >> 0) & 0xf;
  }
};
static_assert((sizeof (start_track)) == 4);

struct end_track {
  const uint8_t _control_adr;
  const uint8_t end_track_number;
  const uint8_t _zero[2];

  uint8_t control() const
  {
    return (_control_adr >> 4) & 0xf;
  }

  uint8_t adr() const
  {
    return (_control_adr >> 0) & 0xf;
  }
};
static_assert((sizeof (end_track)) == 4);

struct lead_out {
  const uint8_t _control_adr;
  const uint8_t _fad[3];

  uint32_t fad() const
  {
    return (_fad[0] << 16) | (_fad[1] << 8) | (_fad[2] << 0);
  }

  uint8_t control() const
  {
    return (_control_adr >> 4) & 0xf;
  }

  uint8_t adr() const
  {
    return (_control_adr >> 0) & 0xf;
  }
};
static_assert((sizeof (lead_out)) == 4);

struct toc {
  struct track track[99];
  struct start_track start_track;
  struct end_track end_track;
  struct lead_out lead_out;
};
static_assert((sizeof (toc)) == 408);

}
