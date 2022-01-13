#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} k_rgb_color;

typedef struct {
  uint16_t hue;
  uint8_t saturation;
  uint8_t lightness;
} k_hsl_color;

k_rgb_color k_hsl_to_rgb(k_hsl_color *hsl);

#endif