#pragma once
#include <lvgl.h>
#include "LarniaPoppinsFonts.h"

static inline lv_color_t C_WHITE()  { return lv_color_hex(0xFFFFFF); }
static inline lv_color_t C_BLUE()   { return lv_color_hex(0x125B90); }
static inline lv_color_t C_BLUE2()  { return lv_color_hex(0x4C89AF); }
static inline lv_color_t C_CYAN()   { return lv_color_hex(0x82CEDC); }
static inline lv_color_t C_CYAN2()  { return lv_color_hex(0xA4F0FC); }
static inline lv_color_t C_EMPTY()  { return lv_color_hex(0xEBF9FA); }
static inline lv_color_t C_ERROR()  { return lv_color_hex(0xCC3E41); }

// Final typography: Poppins converted to LVGL.
// Required generated files in the Arduino sketch folder:
// lv_font_poppins_13.c, 16.c, 20.c, 24.c, 34.c, 36.c, 40.c
#define FONT_SMALL      (&lv_font_poppins_13)
#define FONT_BODY       (&lv_font_poppins_16)
#define FONT_BODY_BOLD  (&lv_font_poppins_16)
#define FONT_H2         (&lv_font_poppins_24)
#define FONT_H1         (&lv_font_poppins_34)
#define FONT_TITLE      (&lv_font_poppins_34)
#define FONT_AI         (&lv_font_poppins_36)
#define FONT_BIG        (&lv_font_poppins_40)
