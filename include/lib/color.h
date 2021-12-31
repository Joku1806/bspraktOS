#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} rgb_color;

typedef struct {
  uint16_t hue;
  uint8_t saturation;
  uint8_t lightness;
} hsl_color;

rgb_color hsl_to_rgb(hsl_color *hsl);

#endif