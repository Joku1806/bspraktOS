#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Color"
#define LOG_COLORED_OUTPUT false // Das hier nicht ändern, sonst Rekursion!

#include <lib/assertions.h>
#include <lib/color.h>
#include <lib/debug.h>
#include <lib/fraction.h>
#include <lib/math.h>
#include <stdint.h>

// Angepasster Code von https://gist.github.com/mjackson/5311256
fraction hue_to_rgb(fraction *p, fraction *q, fraction *t) {
  fraction c0 = fraction_from_number(0);
  fraction c1 = fraction_from_number(1);
  fraction c1_6 = fraction_create(1, 6);
  fraction c1_2 = fraction_create(1, 2);
  fraction c2_3 = fraction_create(2, 3);
  fraction c6 = fraction_from_number(6);

  if (fraction_lt(t, &c0)) {
    // Aufpassen, das wird t außerhalb dieser Funktion überschreiben.
    *t = fraction_add(t, &c1);
  }

  if (fraction_lt(&c1, t)) {
    *t = fraction_sub(t, &c1);
  }

  if (fraction_lt(t, &c1_6)) {
    fraction q_m_p = fraction_sub(q, p);
    fraction q_m_p_m_6 = fraction_multiply(&q_m_p, &c6);
    fraction q_m_p_m_6_m_t = fraction_multiply(&q_m_p_m_6, t);
    return fraction_add(p, &q_m_p_m_6_m_t);
  }

  if (fraction_lt(t, &c1_2)) {
    return *q;
  }

  if (fraction_lt(t, &c2_3)) {
    fraction q_m_p = fraction_sub(q, p);
    fraction tt_m_t = fraction_sub(&c2_3, t);
    fraction q_m_p_m_tt_m_t = fraction_multiply(&q_m_p, &tt_m_t);
    fraction q_m_p_m_tt_m_t_m_6 = fraction_multiply(&q_m_p_m_tt_m_t, &c6);
    return fraction_add(p, &q_m_p_m_tt_m_t_m_6);
  }

  return *p;
}

rgb_color hsl_to_rgb(hsl_color *hsl) {
  VERIFY(hsl->hue < 360);
  VERIFY(hsl->saturation <= 100);
  VERIFY(hsl->lightness <= 100);

  dbgln("Coverting hsl(%u°, %u%%, %u%%) to rgb.", hsl->hue, hsl->saturation, hsl->lightness);

  fraction c0 = fraction_from_number(0);
  fraction c1_3 = fraction_create(1, 3);
  fraction c1_2 = fraction_create(1, 2);
  fraction c1 = fraction_from_number(1);
  fraction c2 = fraction_from_number(2);
  fraction c255 = fraction_from_number(255);

  fraction r, g, b;
  fraction h = fraction_create(hsl->hue, 360);
  fraction s = fraction_create(hsl->saturation, 100);
  fraction l = fraction_create(hsl->lightness, 100);

  if (fraction_eq(&s, &c0)) {
    r = g = b = l;
  } else {
    fraction q;
    if (fraction_lt(&l, &c1_2)) {
      fraction s_p_1 = fraction_add(&s, &c1);
      q = fraction_multiply(&l, &s_p_1);
    } else {
      fraction l_p_s = fraction_add(&l, &s);
      fraction l_m_s = fraction_multiply(&l, &s);
      q = fraction_sub(&l_p_s, &l_m_s);
    }

    fraction l_m_2 = fraction_multiply(&l, &c2);
    fraction p = fraction_sub(&l_m_2, &q);

    fraction h_p_ot = fraction_add(&h, &c1_3);
    fraction h_m_ot = fraction_sub(&h, &c1_3);

    r = hue_to_rgb(&p, &q, &h_p_ot);
    g = hue_to_rgb(&p, &q, &h);
    b = hue_to_rgb(&p, &q, &h_m_ot);
  }

  r = fraction_multiply(&r, &c255);
  g = fraction_multiply(&g, &c255);
  b = fraction_multiply(&b, &c255);

  r = fraction_round(&r);
  g = fraction_round(&g);
  b = fraction_round(&b);

  rgb_color rgb = {
      .red = fraction_to_whole_number(&r),
      .green = fraction_to_whole_number(&g),
      .blue = fraction_to_whole_number(&b),
  };

  dbgln("Converted to rgb(%u, %u, %u).", rgb.red, rgb.green, rgb.blue);
  return rgb;
}