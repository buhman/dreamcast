#include <stdint.h>
#include <stddef.h>

#include "type.hpp"

struct aica_common {
  union {
    reg32 reg_2800;
    reg32 mono_mem8mb_dac18b_ver_mvol;
  };
  union {
    reg32 reg_2804;
    reg32 rbl_rbp;
  };
  union {
    reg32 reg_2808;
    reg32 moful_moemp_miovf_miful_miemp_mibuf;
  };
  union {
    reg32 reg_280c;
    reg32 afsel_mslc_mobuf;
  };
  union {
    reg32 reg_2810;
    reg32 lp_sgc_eg;
  };
  union {
    reg32 reg_2814;
    reg32 ca;
  };
  const reg32 _pad0[26];
  union {
    reg32 reg_2880;
    reg32 dmea0_mrwinh;
  };
  union {
    reg32 reg_2884;
    reg32 dmea1;
  };
  union {
    reg32 reg_2888;
    reg32 dgate_drga;
  };
  union {
    reg32 reg_288c;
    reg32 ddir_dlg_dexe;
  };
  union {
    reg32 reg_2890;
    reg32 tactl_tima;
  };
  union {
    reg32 reg_2894;
    reg32 tbctl_timb;
  };
  union {
    reg32 reg_2898;
    reg32 tcctl_timc;
  };
  union {
    reg32 reg_289c;
    reg32 scieb;
  };
  union {
    reg32 reg_28a0;
    reg32 scipd;
  };
  union {
    reg32 reg_28a4;
    reg32 scire;
  };
  union {
    reg32 reg_28a8;
    reg32 scilv0;
  };
  union {
    reg32 reg_28ac;
    reg32 scilv1;
  };
  union {
    reg32 reg_28b0;
    reg32 scilv2;
  };
  union {
    reg32 reg_28b4;
    reg32 mcieb;
  };
  union {
    reg32 reg_28b8;
    reg32 mcipd;
  };
  union {
    reg32 reg_28bc;
    reg32 mcire;
  };
  const reg32 _pad1[208];
  union {
    reg32 reg_2c00;
    reg32 vreg_armrst;
  };
  const reg32 _pad2[63];
  union {
    reg32 reg_2d00;
    reg32 l7_l6_l5_l4_l3_l2_l1_l0;
  };
  union {
    reg32 reg_2d04;
    reg32 rp_m7_m6_m5_m4_m3_m2_m1_m0;
  };

  uint32_t MONO() const
  {
    return (static_cast<uint32_t>((reg_2800 >> 15) & 0x1) << 0);
  }
  void MONO(const uint32_t v)
  {
    reg_2800 = (((v >> 0) & 0x1) << 15) | (reg_2800 & 0x7fff);
  }

  uint32_t MEM8MB() const
  {
    return (static_cast<uint32_t>((reg_2800 >> 9) & 0x1) << 0);
  }
  void MEM8MB(const uint32_t v)
  {
    reg_2800 = (((v >> 0) & 0x1) << 9) | (reg_2800 & 0xfdff);
  }

  uint32_t DAC18B() const
  {
    return (static_cast<uint32_t>((reg_2800 >> 8) & 0x1) << 0);
  }
  void DAC18B(const uint32_t v)
  {
    reg_2800 = (((v >> 0) & 0x1) << 8) | (reg_2800 & 0xfeff);
  }

  uint32_t VER() const
  {
    return (static_cast<uint32_t>((reg_2800 >> 4) & 0xf) << 0);
  }
  void VER(const uint32_t v)
  {
    reg_2800 = (((v >> 0) & 0xf) << 4) | (reg_2800 & 0xff0f);
  }

  uint32_t MVOL() const
  {
    return (static_cast<uint32_t>((reg_2800 >> 0) & 0xf) << 0);
  }
  void MVOL(const uint32_t v)
  {
    reg_2800 = (((v >> 0) & 0xf) << 0) | (reg_2800 & 0xfff0);
  }

  uint32_t RBL() const
  {
    return (static_cast<uint32_t>((reg_2804 >> 13) & 0x3) << 0);
  }
  void RBL(const uint32_t v)
  {
    reg_2804 = (((v >> 0) & 0x3) << 13) | (reg_2804 & 0x9fff);
  }

  uint32_t RBP() const
  {
    return (static_cast<uint32_t>((reg_2804 >> 0) & 0xfff) << 11);
  }
  void RBP(const uint32_t v)
  {
    reg_2804 = (((v >> 11) & 0xfff) << 0) | (reg_2804 & 0xf000);
  }

  uint32_t MOFUL() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 12) & 0x1) << 0);
  }
  void MOFUL(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0x1) << 12) | (reg_2808 & 0xefff);
  }

  uint32_t MOEMP() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 11) & 0x1) << 0);
  }
  void MOEMP(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0x1) << 11) | (reg_2808 & 0xf7ff);
  }

  uint32_t MIOVF() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 10) & 0x1) << 0);
  }
  void MIOVF(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0x1) << 10) | (reg_2808 & 0xfbff);
  }

  uint32_t MIFUL() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 9) & 0x1) << 0);
  }
  void MIFUL(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0x1) << 9) | (reg_2808 & 0xfdff);
  }

  uint32_t MIEMP() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 8) & 0x1) << 0);
  }
  void MIEMP(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0x1) << 8) | (reg_2808 & 0xfeff);
  }

  uint32_t MIBUF() const
  {
    return (static_cast<uint32_t>((reg_2808 >> 0) & 0xff) << 0);
  }
  void MIBUF(const uint32_t v)
  {
    reg_2808 = (((v >> 0) & 0xff) << 0) | (reg_2808 & 0xff00);
  }

  uint32_t AFSEL() const
  {
    return (static_cast<uint32_t>((reg_280c >> 14) & 0x1) << 0);
  }
  void AFSEL(const uint32_t v)
  {
    reg_280c = (((v >> 0) & 0x1) << 14) | (reg_280c & 0xbfff);
  }

  uint32_t MSLC() const
  {
    return (static_cast<uint32_t>((reg_280c >> 8) & 0x3f) << 0);
  }
  void MSLC(const uint32_t v)
  {
    reg_280c = (((v >> 0) & 0x3f) << 8) | (reg_280c & 0xc0ff);
  }

  uint32_t MOBUF() const
  {
    return (static_cast<uint32_t>((reg_280c >> 0) & 0xff) << 0);
  }
  void MOBUF(const uint32_t v)
  {
    reg_280c = (((v >> 0) & 0xff) << 0) | (reg_280c & 0xff00);
  }

  uint32_t LP() const
  {
    return (static_cast<uint32_t>((reg_2810 >> 15) & 0x1) << 0);
  }
  void LP(const uint32_t v)
  {
    reg_2810 = (((v >> 0) & 0x1) << 15) | (reg_2810 & 0x7fff);
  }

  uint32_t SGC() const
  {
    return (static_cast<uint32_t>((reg_2810 >> 13) & 0x3) << 0);
  }
  void SGC(const uint32_t v)
  {
    reg_2810 = (((v >> 0) & 0x3) << 13) | (reg_2810 & 0x9fff);
  }

  uint32_t EG() const
  {
    return (static_cast<uint32_t>((reg_2810 >> 0) & 0x1fff) << 0);
  }
  void EG(const uint32_t v)
  {
    reg_2810 = (((v >> 0) & 0x1fff) << 0) | (reg_2810 & 0xe000);
  }

  uint32_t CA() const
  {
    return (static_cast<uint32_t>((reg_2814 >> 0) & 0xffff) << 0);
  }
  void CA(const uint32_t v)
  {
    reg_2814 = (((v >> 0) & 0xffff) << 0);
  }

  uint32_t DMEA() const
  {
    return (static_cast<uint32_t>((reg_2880 >> 9) & 0x7f) << 16) | (static_cast<uint32_t>((reg_2884 >> 2) & 0x3fff) << 2);
  }
  void DMEA(const uint32_t v)
  {
    reg_2880 = (((v >> 16) & 0x7f) << 9) | (reg_2880 & 0x1ff);
    reg_2884 = (((v >> 2) & 0x3fff) << 2);
  }

  uint32_t MRWINH() const
  {
    return (static_cast<uint32_t>((reg_2880 >> 0) & 0xf) << 0);
  }
  void MRWINH(const uint32_t v)
  {
    reg_2880 = (((v >> 0) & 0xf) << 0) | (reg_2880 & 0xfff0);
  }

  uint32_t DGATE() const
  {
    return (static_cast<uint32_t>((reg_2888 >> 15) & 0x1) << 0);
  }
  void DGATE(const uint32_t v)
  {
    reg_2888 = (((v >> 0) & 0x1) << 15) | (reg_2888 & 0x7fff);
  }

  uint32_t DRGA() const
  {
    return (static_cast<uint32_t>((reg_2888 >> 2) & 0x1fff) << 2);
  }
  void DRGA(const uint32_t v)
  {
    reg_2888 = (((v >> 2) & 0x1fff) << 2) | (reg_2888 & 0x8003);
  }

  uint32_t DDIR() const
  {
    return (static_cast<uint32_t>((reg_288c >> 15) & 0x1) << 0);
  }
  void DDIR(const uint32_t v)
  {
    reg_288c = (((v >> 0) & 0x1) << 15) | (reg_288c & 0x7fff);
  }

  uint32_t DLG() const
  {
    return (static_cast<uint32_t>((reg_288c >> 2) & 0x1fff) << 2);
  }
  void DLG(const uint32_t v)
  {
    reg_288c = (((v >> 2) & 0x1fff) << 2) | (reg_288c & 0x8003);
  }

  uint32_t DEXE() const
  {
    return (static_cast<uint32_t>((reg_288c >> 0) & 0x1) << 0);
  }
  void DEXE(const uint32_t v)
  {
    reg_288c = (((v >> 0) & 0x1) << 0) | (reg_288c & 0xfffe);
  }

  uint32_t TACTL() const
  {
    return (static_cast<uint32_t>((reg_2890 >> 8) & 0x7) << 0);
  }
  void TACTL(const uint32_t v)
  {
    reg_2890 = (((v >> 0) & 0x7) << 8) | (reg_2890 & 0xf8ff);
  }

  uint32_t TIMA() const
  {
    return (static_cast<uint32_t>((reg_2890 >> 0) & 0xff) << 0);
  }
  void TIMA(const uint32_t v)
  {
    reg_2890 = (((v >> 0) & 0xff) << 0) | (reg_2890 & 0xff00);
  }

  uint32_t TBCTL() const
  {
    return (static_cast<uint32_t>((reg_2894 >> 8) & 0x7) << 0);
  }
  void TBCTL(const uint32_t v)
  {
    reg_2894 = (((v >> 0) & 0x7) << 8) | (reg_2894 & 0xf8ff);
  }

  uint32_t TIMB() const
  {
    return (static_cast<uint32_t>((reg_2894 >> 0) & 0xff) << 0);
  }
  void TIMB(const uint32_t v)
  {
    reg_2894 = (((v >> 0) & 0xff) << 0) | (reg_2894 & 0xff00);
  }

  uint32_t TCCTL() const
  {
    return (static_cast<uint32_t>((reg_2898 >> 8) & 0x7) << 0);
  }
  void TCCTL(const uint32_t v)
  {
    reg_2898 = (((v >> 0) & 0x7) << 8) | (reg_2898 & 0xf8ff);
  }

  uint32_t TIMC() const
  {
    return (static_cast<uint32_t>((reg_2898 >> 0) & 0xff) << 0);
  }
  void TIMC(const uint32_t v)
  {
    reg_2898 = (((v >> 0) & 0xff) << 0) | (reg_2898 & 0xff00);
  }

  uint32_t SCIEB() const
  {
    return (static_cast<uint32_t>((reg_289c >> 0) & 0x7ff) << 0);
  }
  void SCIEB(const uint32_t v)
  {
    reg_289c = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t SCIPD() const
  {
    return (static_cast<uint32_t>((reg_28a0 >> 0) & 0x7ff) << 0);
  }
  void SCIPD(const uint32_t v)
  {
    reg_28a0 = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t SCIRE() const
  {
    return (static_cast<uint32_t>((reg_28a4 >> 0) & 0x7ff) << 0);
  }
  void SCIRE(const uint32_t v)
  {
    reg_28a4 = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t SCILV0() const
  {
    return (static_cast<uint32_t>((reg_28a8 >> 0) & 0xff) << 0);
  }
  void SCILV0(const uint32_t v)
  {
    reg_28a8 = (((v >> 0) & 0xff) << 0);
  }

  uint32_t SCILV1() const
  {
    return (static_cast<uint32_t>((reg_28ac >> 0) & 0xff) << 0);
  }
  void SCILV1(const uint32_t v)
  {
    reg_28ac = (((v >> 0) & 0xff) << 0);
  }

  uint32_t SCILV2() const
  {
    return (static_cast<uint32_t>((reg_28b0 >> 0) & 0xff) << 0);
  }
  void SCILV2(const uint32_t v)
  {
    reg_28b0 = (((v >> 0) & 0xff) << 0);
  }

  uint32_t MCIEB() const
  {
    return (static_cast<uint32_t>((reg_28b4 >> 0) & 0x7ff) << 0);
  }
  void MCIEB(const uint32_t v)
  {
    reg_28b4 = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t MCIPD() const
  {
    return (static_cast<uint32_t>((reg_28b8 >> 0) & 0x7ff) << 0);
  }
  void MCIPD(const uint32_t v)
  {
    reg_28b8 = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t MCIRE() const
  {
    return (static_cast<uint32_t>((reg_28bc >> 0) & 0x7ff) << 0);
  }
  void MCIRE(const uint32_t v)
  {
    reg_28bc = (((v >> 0) & 0x7ff) << 0);
  }

  uint32_t VREG() const
  {
    return (static_cast<uint32_t>((reg_2c00 >> 8) & 0x3) << 0);
  }
  void VREG(const uint32_t v)
  {
    reg_2c00 = (((v >> 0) & 0x3) << 8) | (reg_2c00 & 0xfcff);
  }

  uint32_t ARMRST() const
  {
    return (static_cast<uint32_t>((reg_2c00 >> 0) & 0x1) << 0);
  }
  void ARMRST(const uint32_t v)
  {
    reg_2c00 = (((v >> 0) & 0x1) << 0) | (reg_2c00 & 0xfffe);
  }

  uint32_t L7() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 7) & 0x1) << 0);
  }
  void L7(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 7) | (reg_2d00 & 0xff7f);
  }

  uint32_t L6() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 6) & 0x1) << 0);
  }
  void L6(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 6) | (reg_2d00 & 0xffbf);
  }

  uint32_t L5() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 5) & 0x1) << 0);
  }
  void L5(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 5) | (reg_2d00 & 0xffdf);
  }

  uint32_t L4() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 4) & 0x1) << 0);
  }
  void L4(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 4) | (reg_2d00 & 0xffef);
  }

  uint32_t L3() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 3) & 0x1) << 0);
  }
  void L3(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 3) | (reg_2d00 & 0xfff7);
  }

  uint32_t L2() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 2) & 0x1) << 0);
  }
  void L2(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 2) | (reg_2d00 & 0xfffb);
  }

  uint32_t L1() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 1) & 0x1) << 0);
  }
  void L1(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 1) | (reg_2d00 & 0xfffd);
  }

  uint32_t L0() const
  {
    return (static_cast<uint32_t>((reg_2d00 >> 0) & 0x1) << 0);
  }
  void L0(const uint32_t v)
  {
    reg_2d00 = (((v >> 0) & 0x1) << 0) | (reg_2d00 & 0xfffe);
  }

  uint32_t RP() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 8) & 0x1) << 0);
  }
  void RP(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 8) | (reg_2d04 & 0xfeff);
  }

  uint32_t M7() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 7) & 0x1) << 0);
  }
  void M7(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 7) | (reg_2d04 & 0xff7f);
  }

  uint32_t M6() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 6) & 0x1) << 0);
  }
  void M6(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 6) | (reg_2d04 & 0xffbf);
  }

  uint32_t M5() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 5) & 0x1) << 0);
  }
  void M5(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 5) | (reg_2d04 & 0xffdf);
  }

  uint32_t M4() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 4) & 0x1) << 0);
  }
  void M4(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 4) | (reg_2d04 & 0xffef);
  }

  uint32_t M3() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 3) & 0x1) << 0);
  }
  void M3(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 3) | (reg_2d04 & 0xfff7);
  }

  uint32_t M2() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 2) & 0x1) << 0);
  }
  void M2(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 2) | (reg_2d04 & 0xfffb);
  }

  uint32_t M1() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 1) & 0x1) << 0);
  }
  void M1(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 1) | (reg_2d04 & 0xfffd);
  }

  uint32_t M0() const
  {
    return (static_cast<uint32_t>((reg_2d04 >> 0) & 0x1) << 0);
  }
  void M0(const uint32_t v)
  {
    reg_2d04 = (((v >> 0) & 0x1) << 0) | (reg_2d04 & 0xfffe);
  }

};

static_assert((sizeof (aica_common)) == 0x2d08 - 0x2800);
static_assert((offsetof (aica_common, reg_2800)) == 0x2800 - 0x2800);
static_assert((offsetof (aica_common, reg_2804)) == 0x2804 - 0x2800);
static_assert((offsetof (aica_common, reg_2808)) == 0x2808 - 0x2800);
static_assert((offsetof (aica_common, reg_280c)) == 0x280c - 0x2800);
static_assert((offsetof (aica_common, reg_2810)) == 0x2810 - 0x2800);
static_assert((offsetof (aica_common, reg_2814)) == 0x2814 - 0x2800);
static_assert((offsetof (aica_common, reg_2880)) == 0x2880 - 0x2800);
static_assert((offsetof (aica_common, reg_2884)) == 0x2884 - 0x2800);
static_assert((offsetof (aica_common, reg_2888)) == 0x2888 - 0x2800);
static_assert((offsetof (aica_common, reg_288c)) == 0x288c - 0x2800);
static_assert((offsetof (aica_common, reg_2890)) == 0x2890 - 0x2800);
static_assert((offsetof (aica_common, reg_2894)) == 0x2894 - 0x2800);
static_assert((offsetof (aica_common, reg_2898)) == 0x2898 - 0x2800);
static_assert((offsetof (aica_common, reg_289c)) == 0x289c - 0x2800);
static_assert((offsetof (aica_common, reg_28a0)) == 0x28a0 - 0x2800);
static_assert((offsetof (aica_common, reg_28a4)) == 0x28a4 - 0x2800);
static_assert((offsetof (aica_common, reg_28a8)) == 0x28a8 - 0x2800);
static_assert((offsetof (aica_common, reg_28ac)) == 0x28ac - 0x2800);
static_assert((offsetof (aica_common, reg_28b0)) == 0x28b0 - 0x2800);
static_assert((offsetof (aica_common, reg_28b4)) == 0x28b4 - 0x2800);
static_assert((offsetof (aica_common, reg_28b8)) == 0x28b8 - 0x2800);
static_assert((offsetof (aica_common, reg_28bc)) == 0x28bc - 0x2800);
static_assert((offsetof (aica_common, reg_2c00)) == 0x2c00 - 0x2800);
static_assert((offsetof (aica_common, reg_2d00)) == 0x2d00 - 0x2800);
static_assert((offsetof (aica_common, reg_2d04)) == 0x2d04 - 0x2800);

namespace aica {
  namespace mono_mem8mb_dac18b_ver_mvol {
    constexpr uint32_t MONO(const uint32_t v) { return (((v >> 0) & 0x1) << 15); }
    constexpr uint32_t MEM8MB(const uint32_t v) { return (((v >> 0) & 0x1) << 9); }
    constexpr uint32_t DAC18B(const uint32_t v) { return (((v >> 0) & 0x1) << 8); }
    constexpr uint32_t VER(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 4) & 0xf) << 0); }
    constexpr uint32_t MVOL(const uint32_t v) { return (((v >> 0) & 0xf) << 0); }
  }
  namespace rbl_rbp {
    constexpr uint32_t RBL(const uint32_t v) { return (((v >> 0) & 0x3) << 13); }
    constexpr uint32_t RBP(const uint32_t v) { return (((v >> 11) & 0xfff) << 0); }
  }
  namespace moful_moemp_miovf_miful_miemp_mibuf {
    constexpr uint32_t MOFUL(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 12) & 0x1) << 0); }
    constexpr uint32_t MOEMP(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 11) & 0x1) << 0); }
    constexpr uint32_t MIOVF(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 10) & 0x1) << 0); }
    constexpr uint32_t MIFUL(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 9) & 0x1) << 0); }
    constexpr uint32_t MIEMP(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 8) & 0x1) << 0); }
    constexpr uint32_t MIBUF(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 0) & 0xff) << 0); }
  }
  namespace afsel_mslc_mobuf {
    constexpr uint32_t AFSEL(const uint32_t v) { return (((v >> 0) & 0x1) << 14); }
    constexpr uint32_t MSLC(const uint32_t v) { return (((v >> 0) & 0x3f) << 8); }
    constexpr uint32_t MOBUF(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace lp_sgc_eg {
    constexpr uint32_t LP(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 15) & 0x1) << 0); }
    constexpr uint32_t SGC(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 13) & 0x3) << 0); }
    constexpr uint32_t EG(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 0) & 0x1fff) << 0); }
  }
  namespace ca {
    constexpr uint32_t CA(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 0) & 0xffff) << 0); }
  }
  namespace dmea0_mrwinh {
    constexpr uint32_t DMEA(const uint32_t v) { return (((v >> 16) & 0x7f) << 9); }
    constexpr uint32_t MRWINH(const uint32_t v) { return (((v >> 0) & 0xf) << 0); }
  }
  namespace dmea1 {
    constexpr uint32_t DMEA(const uint32_t v) { return (((v >> 2) & 0x3fff) << 2); }
  }
  namespace dgate_drga {
    constexpr uint32_t DGATE(const uint32_t v) { return (((v >> 0) & 0x1) << 15); }
    constexpr uint32_t DRGA(const uint32_t v) { return (((v >> 2) & 0x1fff) << 2); }
  }
  namespace ddir_dlg_dexe {
    constexpr uint32_t DDIR(const uint32_t v) { return (((v >> 0) & 0x1) << 15); }
    constexpr uint32_t DLG(const uint32_t v) { return (((v >> 2) & 0x1fff) << 2); }
    constexpr uint32_t DEXE(const uint32_t v) { return (((v >> 0) & 0x1) << 0); }
  }
  namespace tactl_tima {
    constexpr uint32_t TACTL(const uint32_t v) { return (((v >> 0) & 0x7) << 8); }
    constexpr uint32_t TIMA(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace tbctl_timb {
    constexpr uint32_t TBCTL(const uint32_t v) { return (((v >> 0) & 0x7) << 8); }
    constexpr uint32_t TIMB(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace tcctl_timc {
    constexpr uint32_t TCCTL(const uint32_t v) { return (((v >> 0) & 0x7) << 8); }
    constexpr uint32_t TIMC(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace scieb {
    constexpr uint32_t SCIEB(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace scipd {
    constexpr uint32_t SCIPD(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace scire {
    constexpr uint32_t SCIRE(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace scilv0 {
    constexpr uint32_t SCILV0(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace scilv1 {
    constexpr uint32_t SCILV1(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace scilv2 {
    constexpr uint32_t SCILV2(const uint32_t v) { return (((v >> 0) & 0xff) << 0); }
  }
  namespace mcieb {
    constexpr uint32_t MCIEB(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace mcipd {
    constexpr uint32_t MCIPD(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace mcire {
    constexpr uint32_t MCIRE(const uint32_t v) { return (((v >> 0) & 0x7ff) << 0); }
  }
  namespace vreg_armrst {
    constexpr uint32_t VREG(const uint32_t v) { return (((v >> 0) & 0x3) << 8); }
    constexpr uint32_t ARMRST(const uint32_t v) { return (((v >> 0) & 0x1) << 0); }
  }
  namespace l7_l6_l5_l4_l3_l2_l1_l0 {
    constexpr uint32_t L7(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 7) & 0x1) << 0); }
    constexpr uint32_t L6(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 6) & 0x1) << 0); }
    constexpr uint32_t L5(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 5) & 0x1) << 0); }
    constexpr uint32_t L4(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 4) & 0x1) << 0); }
    constexpr uint32_t L3(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 3) & 0x1) << 0); }
    constexpr uint32_t L2(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 2) & 0x1) << 0); }
    constexpr uint32_t L1(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 1) & 0x1) << 0); }
    constexpr uint32_t L0(const uint32_t reg) { return (static_cast<uint32_t>((reg >> 0) & 0x1) << 0); }
  }
  namespace rp_m7_m6_m5_m4_m3_m2_m1_m0 {
    constexpr uint32_t RP(const uint32_t v) { return (((v >> 0) & 0x1) << 8); }
    constexpr uint32_t M7(const uint32_t v) { return (((v >> 0) & 0x1) << 7); }
    constexpr uint32_t M6(const uint32_t v) { return (((v >> 0) & 0x1) << 6); }
    constexpr uint32_t M5(const uint32_t v) { return (((v >> 0) & 0x1) << 5); }
    constexpr uint32_t M4(const uint32_t v) { return (((v >> 0) & 0x1) << 4); }
    constexpr uint32_t M3(const uint32_t v) { return (((v >> 0) & 0x1) << 3); }
    constexpr uint32_t M2(const uint32_t v) { return (((v >> 0) & 0x1) << 2); }
    constexpr uint32_t M1(const uint32_t v) { return (((v >> 0) & 0x1) << 1); }
    constexpr uint32_t M0(const uint32_t v) { return (((v >> 0) & 0x1) << 0); }
  }
}
