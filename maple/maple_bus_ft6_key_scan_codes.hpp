#pragma once

#include <cstdint>

namespace ft6 {
  namespace scan_code {
    constexpr uint32_t no_operation = 0x0;
    constexpr uint32_t rollover_error = 0x1;
    constexpr uint32_t post_fail = 0x2;
    constexpr uint32_t undefined_error = 0x3;
    constexpr uint32_t a_A = 0x4;
    constexpr uint32_t b_B = 0x5;
    constexpr uint32_t c_C = 0x6;
    constexpr uint32_t d_D = 0x7;
    constexpr uint32_t e_E = 0x8;
    constexpr uint32_t f_F = 0x9;
    constexpr uint32_t g_G = 0xa;
    constexpr uint32_t h_H = 0xb;
    constexpr uint32_t i_I = 0xc;
    constexpr uint32_t j_J = 0xd;
    constexpr uint32_t k_K = 0xe;
    constexpr uint32_t l_L = 0xf;
    constexpr uint32_t m_M = 0x10;
    constexpr uint32_t n_N = 0x11;
    constexpr uint32_t o_O = 0x12;
    constexpr uint32_t p_P = 0x13;
    constexpr uint32_t q_Q = 0x14;
    constexpr uint32_t r_R = 0x15;
    constexpr uint32_t s_S = 0x16;
    constexpr uint32_t t_T = 0x17;
    constexpr uint32_t u_U = 0x18;
    constexpr uint32_t v_V = 0x19;
    constexpr uint32_t w_W = 0x1a;
    constexpr uint32_t x_X = 0x1b;
    constexpr uint32_t y_Y = 0x1c;
    constexpr uint32_t z_Z = 0x1d;
    constexpr uint32_t _1_exclam = 0x1e;
    constexpr uint32_t _2_at = 0x1f;
    constexpr uint32_t _3_numbersign = 0x20;
    constexpr uint32_t _4_dollar = 0x21;
    constexpr uint32_t _5_percent = 0x22;
    constexpr uint32_t _6_asciicircum = 0x23;
    constexpr uint32_t _7_ampersand = 0x24;
    constexpr uint32_t _8_asterisk = 0x25;
    constexpr uint32_t _9_parenleft = 0x26;
    constexpr uint32_t _0_parenright = 0x27;
    constexpr uint32_t _return = 0x28;
    constexpr uint32_t esc = 0x29;
    constexpr uint32_t backspace = 0x2a;
    constexpr uint32_t tab = 0x2b;
    constexpr uint32_t spacebar = 0x2c;
    constexpr uint32_t minus_underscore = 0x2d;
    constexpr uint32_t equal_plus = 0x2e;
    constexpr uint32_t bracketleft_braceleft = 0x2f;
    constexpr uint32_t bracketright_braceright = 0x30;
    constexpr uint32_t backslash_bar = 0x31;
    constexpr uint32_t iso_numbersign_tilde = 0x32;
    constexpr uint32_t semicolon_colon = 0x33;
    constexpr uint32_t apostrophe_quotedbl = 0x34;
    constexpr uint32_t grave_asciitilde = 0x35;
    constexpr uint32_t comma_less = 0x36;
    constexpr uint32_t period_greater = 0x37;
    constexpr uint32_t slash_question = 0x38;
    constexpr uint32_t caps_lock = 0x39;
    constexpr uint32_t F1 = 0x3a;
    constexpr uint32_t F2 = 0x3b;
    constexpr uint32_t F3 = 0x3c;
    constexpr uint32_t F4 = 0x3d;
    constexpr uint32_t F5 = 0x3e;
    constexpr uint32_t F6 = 0x3f;
    constexpr uint32_t F7 = 0x40;
    constexpr uint32_t F8 = 0x41;
    constexpr uint32_t F9 = 0x42;
    constexpr uint32_t F10 = 0x43;
    constexpr uint32_t F11 = 0x44;
    constexpr uint32_t F12 = 0x45;
    constexpr uint32_t print_screen = 0x46;
    constexpr uint32_t scroll_lock = 0x47;
    constexpr uint32_t pause = 0x48;
    constexpr uint32_t insert = 0x49;
    constexpr uint32_t home = 0x4a;
    constexpr uint32_t page_up = 0x4b;
    constexpr uint32_t _delete = 0x4c;
    constexpr uint32_t end = 0x4d;
    constexpr uint32_t page_down = 0x4e;
    constexpr uint32_t right_arrow = 0x4f;
    constexpr uint32_t left_arrow = 0x50;
    constexpr uint32_t down_arrow = 0x51;
    constexpr uint32_t up_arrow = 0x52;
  }
}
namespace ft6 {
  namespace scan_code {
    constexpr uint32_t first_printable = 0x4;
    constexpr uint32_t last_printable = 0x38;

    const uint8_t code_point[last_printable + 1][2] = {
      [scan_code::no_operation] = { 0, 0 },
      [scan_code::rollover_error] = { 0, 0 },
      [scan_code::post_fail] = { 0, 0 },
      [scan_code::undefined_error] = { 0, 0 },
      [scan_code::a_A] = { 'a', 'A' },
      [scan_code::b_B] = { 'b', 'B' },
      [scan_code::c_C] = { 'c', 'C' },
      [scan_code::d_D] = { 'd', 'D' },
      [scan_code::e_E] = { 'e', 'E' },
      [scan_code::f_F] = { 'f', 'F' },
      [scan_code::g_G] = { 'g', 'G' },
      [scan_code::h_H] = { 'h', 'H' },
      [scan_code::i_I] = { 'i', 'I' },
      [scan_code::j_J] = { 'j', 'J' },
      [scan_code::k_K] = { 'k', 'K' },
      [scan_code::l_L] = { 'l', 'L' },
      [scan_code::m_M] = { 'm', 'M' },
      [scan_code::n_N] = { 'n', 'N' },
      [scan_code::o_O] = { 'o', 'O' },
      [scan_code::p_P] = { 'p', 'P' },
      [scan_code::q_Q] = { 'q', 'Q' },
      [scan_code::r_R] = { 'r', 'R' },
      [scan_code::s_S] = { 's', 'S' },
      [scan_code::t_T] = { 't', 'T' },
      [scan_code::u_U] = { 'u', 'U' },
      [scan_code::v_V] = { 'v', 'V' },
      [scan_code::w_W] = { 'w', 'W' },
      [scan_code::x_X] = { 'x', 'X' },
      [scan_code::y_Y] = { 'y', 'Y' },
      [scan_code::z_Z] = { 'z', 'Z' },
      [scan_code::_1_exclam] = { '1', '!' },
      [scan_code::_2_at] = { '2', '@' },
      [scan_code::_3_numbersign] = { '3', '#' },
      [scan_code::_4_dollar] = { '4', '$' },
      [scan_code::_5_percent] = { '5', '%' },
      [scan_code::_6_asciicircum] = { '6', '^' },
      [scan_code::_7_ampersand] = { '7', '&' },
      [scan_code::_8_asterisk] = { '8', '*' },
      [scan_code::_9_parenleft] = { '9', '(' },
      [scan_code::_0_parenright] = { '0', ')' },
      [scan_code::_return] = { 0, 0 },
      [scan_code::esc] = { 0, 0 },
      [scan_code::backspace] = { 0, 0 },
      [scan_code::tab] = { 0, 0 },
      [scan_code::spacebar] = { 0, 0 },
      [scan_code::minus_underscore] = { '-', '_' },
      [scan_code::equal_plus] = { '=', '+' },
      [scan_code::bracketleft_braceleft] = { '[', '{' },
      [scan_code::bracketright_braceright] = { ']', '}' },
      [scan_code::backslash_bar] = { '\\', '|' },
      [scan_code::iso_numbersign_tilde] = { '#', '~' },
      [scan_code::semicolon_colon] = { ';', ':' },
      [scan_code::apostrophe_quotedbl] = { '\'', '"' },
      [scan_code::grave_asciitilde] = { '\'', '~' },
      [scan_code::comma_less] = { ',', '<' },
      [scan_code::period_greater] = { '.', '>' },
      [scan_code::slash_question] = { '/', '?' },
    };
  }
}
