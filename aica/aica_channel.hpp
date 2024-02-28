#include <cstdint>
#include <cstddef>

#include "type.hpp"

struct aica_channel {
  reg16 reg_0000;
  const reg16 _pad0[1];
  reg16 reg_0004;
  const reg16 _pad1[1];
  reg16 reg_0008;
  const reg16 _pad2[1];
  reg16 reg_000c;
  const reg16 _pad3[1];
  reg16 reg_0010;
  const reg16 _pad4[1];
  reg16 reg_0014;
  const reg16 _pad5[1];
  reg16 reg_0018;
  const reg16 _pad6[1];
  reg16 reg_001c;
  const reg16 _pad7[1];
  reg16 reg_0020;
  const reg16 _pad8[1];
  reg16 reg_0024;
  const reg16 _pad9[1];
  reg16 reg_0028;
  const reg16 _pad10[1];
  reg16 reg_002c;
  const reg16 _pad11[1];
  reg16 reg_0030;
  const reg16 _pad12[1];
  reg16 reg_0034;
  const reg16 _pad13[1];
  reg16 reg_0038;
  const reg16 _pad14[1];
  reg16 reg_003c;
  const reg16 _pad15[1];
  reg16 reg_0040;
  const reg16 _pad16[1];
  reg16 reg_0044;
  const reg16 _pad17[1];
  const reg16 _pad18[28];

  uint32_t KYONEX() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 15) & 0x1) << 0);
  }
  void KYONEX(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x1) << 15) | (reg_0000 & 0x7fff);
  }

  uint32_t KYONB() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 14) & 0x1) << 0);
  }
  void KYONB(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x1) << 14) | (reg_0000 & 0xbfff);
  }

  uint32_t SSCTL() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 10) & 0x1) << 0);
  }
  void SSCTL(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x1) << 10) | (reg_0000 & 0xfbff);
  }

  uint32_t LPCTL() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 9) & 0x1) << 0);
  }
  void LPCTL(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x1) << 9) | (reg_0000 & 0xfdff);
  }

  uint32_t PCMS() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 7) & 0x3) << 0);
  }
  void PCMS(const uint32_t v)
  {
    reg_0000 = (((v >> 0) & 0x3) << 7) | (reg_0000 & 0xfe7f);
  }

  uint32_t SA() const
  {
    return (static_cast<uint32_t>((reg_0000 >> 0) & 0x7f) << 16) | (static_cast<uint32_t>((reg_0004 >> 0) & 0xffff) << 0);
  }
  void SA(const uint32_t v)
  {
    reg_0000 = (((v >> 16) & 0x7f) << 0) | (reg_0000 & 0xff80);
    reg_0004 = (((v >> 0) & 0xffff) << 0);
  }

  uint32_t LSA() const
  {
    return (static_cast<uint32_t>((reg_0008 >> 0) & 0xffff) << 0);
  }
  void LSA(const uint32_t v)
  {
    reg_0008 = (((v >> 0) & 0xffff) << 0);
  }

  uint32_t LEA() const
  {
    return (static_cast<uint32_t>((reg_000c >> 0) & 0xffff) << 0);
  }
  void LEA(const uint32_t v)
  {
    reg_000c = (((v >> 0) & 0xffff) << 0);
  }

  uint32_t D2R() const
  {
    return (static_cast<uint32_t>((reg_0010 >> 11) & 0x1f) << 0);
  }
  void D2R(const uint32_t v)
  {
    reg_0010 = (((v >> 0) & 0x1f) << 11) | (reg_0010 & 0x7ff);
  }

  uint32_t D1R() const
  {
    return (static_cast<uint32_t>((reg_0010 >> 6) & 0x1f) << 0);
  }
  void D1R(const uint32_t v)
  {
    reg_0010 = (((v >> 0) & 0x1f) << 6) | (reg_0010 & 0xf83f);
  }

  uint32_t AR() const
  {
    return (static_cast<uint32_t>((reg_0010 >> 0) & 0x1f) << 0);
  }
  void AR(const uint32_t v)
  {
    reg_0010 = (((v >> 0) & 0x1f) << 0) | (reg_0010 & 0xffe0);
  }

  uint32_t LPSLNK() const
  {
    return (static_cast<uint32_t>((reg_0014 >> 14) & 0x1) << 0);
  }
  void LPSLNK(const uint32_t v)
  {
    reg_0014 = (((v >> 0) & 0x1) << 14) | (reg_0014 & 0xbfff);
  }

  uint32_t KRS() const
  {
    return (static_cast<uint32_t>((reg_0014 >> 10) & 0xf) << 0);
  }
  void KRS(const uint32_t v)
  {
    reg_0014 = (((v >> 0) & 0xf) << 10) | (reg_0014 & 0xc3ff);
  }

  uint32_t DL() const
  {
    return (static_cast<uint32_t>((reg_0014 >> 5) & 0x1f) << 0);
  }
  void DL(const uint32_t v)
  {
    reg_0014 = (((v >> 0) & 0x1f) << 5) | (reg_0014 & 0xfc1f);
  }

  uint32_t RR() const
  {
    return (static_cast<uint32_t>((reg_0014 >> 0) & 0x1f) << 0);
  }
  void RR(const uint32_t v)
  {
    reg_0014 = (((v >> 0) & 0x1f) << 0) | (reg_0014 & 0xffe0);
  }

  uint32_t OCT() const
  {
    return (static_cast<uint32_t>((reg_0018 >> 11) & 0xf) << 0);
  }
  void OCT(const uint32_t v)
  {
    reg_0018 = (((v >> 0) & 0xf) << 11) | (reg_0018 & 0x87ff);
  }

  uint32_t FNS() const
  {
    return (static_cast<uint32_t>((reg_0018 >> 0) & 0x3ff) << 0);
  }
  void FNS(const uint32_t v)
  {
    reg_0018 = (((v >> 0) & 0x3ff) << 0) | (reg_0018 & 0xfc00);
  }

  uint32_t LFORE() const
  {
    return (static_cast<uint32_t>((reg_001c >> 15) & 0x1) << 0);
  }
  void LFORE(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x1) << 15) | (reg_001c & 0x7fff);
  }

  uint32_t LFOF() const
  {
    return (static_cast<uint32_t>((reg_001c >> 10) & 0x1f) << 0);
  }
  void LFOF(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x1f) << 10) | (reg_001c & 0x83ff);
  }

  uint32_t PLFOWS() const
  {
    return (static_cast<uint32_t>((reg_001c >> 8) & 0x3) << 0);
  }
  void PLFOWS(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x3) << 8) | (reg_001c & 0xfcff);
  }

  uint32_t PLFOS() const
  {
    return (static_cast<uint32_t>((reg_001c >> 5) & 0x7) << 0);
  }
  void PLFOS(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x7) << 5) | (reg_001c & 0xff1f);
  }

  uint32_t ALFOWS() const
  {
    return (static_cast<uint32_t>((reg_001c >> 3) & 0x3) << 0);
  }
  void ALFOWS(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x3) << 3) | (reg_001c & 0xffe7);
  }

  uint32_t ALFOS() const
  {
    return (static_cast<uint32_t>((reg_001c >> 0) & 0x7) << 0);
  }
  void ALFOS(const uint32_t v)
  {
    reg_001c = (((v >> 0) & 0x7) << 0) | (reg_001c & 0xfff8);
  }

  uint32_t IMXL() const
  {
    return (static_cast<uint32_t>((reg_0020 >> 4) & 0xf) << 0);
  }
  void IMXL(const uint32_t v)
  {
    reg_0020 = (((v >> 0) & 0xf) << 4) | (reg_0020 & 0xff0f);
  }

  uint32_t ISEL() const
  {
    return (static_cast<uint32_t>((reg_0020 >> 0) & 0xf) << 0);
  }
  void ISEL(const uint32_t v)
  {
    reg_0020 = (((v >> 0) & 0xf) << 0) | (reg_0020 & 0xfff0);
  }

  uint32_t DISDL() const
  {
    return (static_cast<uint32_t>((reg_0024 >> 8) & 0xf) << 0);
  }
  void DISDL(const uint32_t v)
  {
    reg_0024 = (((v >> 0) & 0xf) << 8) | (reg_0024 & 0xf0ff);
  }

  uint32_t DIPAN() const
  {
    return (static_cast<uint32_t>((reg_0024 >> 0) & 0x1f) << 0);
  }
  void DIPAN(const uint32_t v)
  {
    reg_0024 = (((v >> 0) & 0x1f) << 0) | (reg_0024 & 0xffe0);
  }

  uint32_t TL() const
  {
    return (static_cast<uint32_t>((reg_0028 >> 8) & 0xff) << 0);
  }
  void TL(const uint32_t v)
  {
    reg_0028 = (((v >> 0) & 0xff) << 8) | (reg_0028 & 0xff);
  }

  uint32_t VOFF() const
  {
    return (static_cast<uint32_t>((reg_0028 >> 6) & 0x1) << 0);
  }
  void VOFF(const uint32_t v)
  {
    reg_0028 = (((v >> 0) & 0x1) << 6) | (reg_0028 & 0xffbf);
  }

  uint32_t LPOFF() const
  {
    return (static_cast<uint32_t>((reg_0028 >> 5) & 0x1) << 0);
  }
  void LPOFF(const uint32_t v)
  {
    reg_0028 = (((v >> 0) & 0x1) << 5) | (reg_0028 & 0xffdf);
  }

  uint32_t Q() const
  {
    return (static_cast<uint32_t>((reg_0028 >> 0) & 0x1f) << 0);
  }
  void Q(const uint32_t v)
  {
    reg_0028 = (((v >> 0) & 0x1f) << 0) | (reg_0028 & 0xffe0);
  }

  uint32_t FLV0() const
  {
    return (static_cast<uint32_t>((reg_002c >> 0) & 0x1fff) << 0);
  }
  void FLV0(const uint32_t v)
  {
    reg_002c = (((v >> 0) & 0x1fff) << 0);
  }

  uint32_t FLV1() const
  {
    return (static_cast<uint32_t>((reg_0030 >> 0) & 0x1fff) << 0);
  }
  void FLV1(const uint32_t v)
  {
    reg_0030 = (((v >> 0) & 0x1fff) << 0);
  }

  uint32_t FLV2() const
  {
    return (static_cast<uint32_t>((reg_0034 >> 0) & 0x1fff) << 0);
  }
  void FLV2(const uint32_t v)
  {
    reg_0034 = (((v >> 0) & 0x1fff) << 0);
  }

  uint32_t FLV3() const
  {
    return (static_cast<uint32_t>((reg_0038 >> 0) & 0x1fff) << 0);
  }
  void FLV3(const uint32_t v)
  {
    reg_0038 = (((v >> 0) & 0x1fff) << 0);
  }

  uint32_t FLV4() const
  {
    return (static_cast<uint32_t>((reg_003c >> 0) & 0x1fff) << 0);
  }
  void FLV4(const uint32_t v)
  {
    reg_003c = (((v >> 0) & 0x1fff) << 0);
  }

  uint32_t FAR() const
  {
    return (static_cast<uint32_t>((reg_0040 >> 8) & 0x1f) << 0);
  }
  void FAR(const uint32_t v)
  {
    reg_0040 = (((v >> 0) & 0x1f) << 8) | (reg_0040 & 0xe0ff);
  }

  uint32_t FD1R() const
  {
    return (static_cast<uint32_t>((reg_0040 >> 0) & 0x1f) << 0);
  }
  void FD1R(const uint32_t v)
  {
    reg_0040 = (((v >> 0) & 0x1f) << 0) | (reg_0040 & 0xffe0);
  }

  uint32_t FD2R() const
  {
    return (static_cast<uint32_t>((reg_0044 >> 8) & 0x1f) << 0);
  }
  void FD2R(const uint32_t v)
  {
    reg_0044 = (((v >> 0) & 0x1f) << 8) | (reg_0044 & 0xe0ff);
  }

  uint32_t FRR() const
  {
    return (static_cast<uint32_t>((reg_0044 >> 0) & 0x1f) << 0);
  }
  void FRR(const uint32_t v)
  {
    reg_0044 = (((v >> 0) & 0x1f) << 0) | (reg_0044 & 0xffe0);
  }

};

static_assert((sizeof (aica_channel)) == 0x80 - 0x0);
static_assert((offsetof (aica_channel, reg_0000)) == 0x0 - 0x0);
static_assert((offsetof (aica_channel, reg_0004)) == 0x4 - 0x0);
static_assert((offsetof (aica_channel, reg_0008)) == 0x8 - 0x0);
static_assert((offsetof (aica_channel, reg_000c)) == 0xc - 0x0);
static_assert((offsetof (aica_channel, reg_0010)) == 0x10 - 0x0);
static_assert((offsetof (aica_channel, reg_0014)) == 0x14 - 0x0);
static_assert((offsetof (aica_channel, reg_0018)) == 0x18 - 0x0);
static_assert((offsetof (aica_channel, reg_001c)) == 0x1c - 0x0);
static_assert((offsetof (aica_channel, reg_0020)) == 0x20 - 0x0);
static_assert((offsetof (aica_channel, reg_0024)) == 0x24 - 0x0);
static_assert((offsetof (aica_channel, reg_0028)) == 0x28 - 0x0);
static_assert((offsetof (aica_channel, reg_002c)) == 0x2c - 0x0);
static_assert((offsetof (aica_channel, reg_0030)) == 0x30 - 0x0);
static_assert((offsetof (aica_channel, reg_0034)) == 0x34 - 0x0);
static_assert((offsetof (aica_channel, reg_0038)) == 0x38 - 0x0);
static_assert((offsetof (aica_channel, reg_003c)) == 0x3c - 0x0);
static_assert((offsetof (aica_channel, reg_0040)) == 0x40 - 0x0);
static_assert((offsetof (aica_channel, reg_0044)) == 0x44 - 0x0);
