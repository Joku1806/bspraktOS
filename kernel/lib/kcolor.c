#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Color"
#define LOG_COLORED_OUTPUT false // Das hier nicht ändern, sonst Rekursion!

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kcolor.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kfraction.h>
#include <kernel/lib/kmath.h>
#include <stdint.h>

// Angepasster Code von https://gist.github.com/mjackson/5311256
k_fraction k_hue_to_rgb(k_fraction *p, k_fraction *q, k_fraction *t) {
  k_fraction c0 = k_fraction_from_number(0);
  k_fraction c1 = k_fraction_from_number(1);
  k_fraction c1_6 = k_fraction_create(1, 6);
  k_fraction c1_2 = k_fraction_create(1, 2);
  k_fraction c2_3 = k_fraction_create(2, 3);
  k_fraction c6 = k_fraction_from_number(6);

  if (k_fraction_lt(t, &c0)) {
    // Aufpassen, das wird t außerhalb dieser Funktion überschreiben.
    *t = k_fraction_add(t, &c1);
  }

  if (k_fraction_lt(&c1, t)) {
    *t = k_fraction_sub(t, &c1);
  }

  if (k_fraction_lt(t, &c1_6)) {
    k_fraction q_m_p = k_fraction_sub(q, p);
    k_fraction q_m_p_m_6 = k_fraction_multiply(&q_m_p, &c6);
    k_fraction q_m_p_m_6_m_t = k_fraction_multiply(&q_m_p_m_6, t);
    return k_fraction_add(p, &q_m_p_m_6_m_t);
  }

  if (k_fraction_lt(t, &c1_2)) {
    return *q;
  }

  if (k_fraction_lt(t, &c2_3)) {
    k_fraction q_m_p = k_fraction_sub(q, p);
    k_fraction tt_m_t = k_fraction_sub(&c2_3, t);
    k_fraction q_m_p_m_tt_m_t = k_fraction_multiply(&q_m_p, &tt_m_t);
    k_fraction q_m_p_m_tt_m_t_m_6 = k_fraction_multiply(&q_m_p_m_tt_m_t, &c6);
    return k_fraction_add(p, &q_m_p_m_tt_m_t_m_6);
  }

  return *p;
}

k_rgb_color k_hsl_to_rgb(k_hsl_color *hsl) {
  VERIFY(hsl->hue < 360);
  VERIFY(hsl->saturation <= 100);
  VERIFY(hsl->lightness <= 100);

  kdbgln("Coverting hsl(%u°, %u%%, %u%%) to rgb.", hsl->hue, hsl->saturation, hsl->lightness);

  k_fraction c0 = k_fraction_from_number(0);
  k_fraction c1_3 = k_fraction_create(1, 3);
  k_fraction c1_2 = k_fraction_create(1, 2);
  k_fraction c1 = k_fraction_from_number(1);
  k_fraction c2 = k_fraction_from_number(2);
  k_fraction c255 = k_fraction_from_number(255);

  k_fraction r, g, b;
  k_fraction h = k_fraction_create(hsl->hue, 360);
  k_fraction s = k_fraction_create(hsl->saturation, 100);
  k_fraction l = k_fraction_create(hsl->lightness, 100);

  if (k_fraction_eq(&s, &c0)) {
    r = g = b = l;
  } else {
    k_fraction q;
    if (k_fraction_lt(&l, &c1_2)) {
      k_fraction s_p_1 = k_fraction_add(&s, &c1);
      q = k_fraction_multiply(&l, &s_p_1);
    } else {
      k_fraction l_p_s = k_fraction_add(&l, &s);
      k_fraction l_m_s = k_fraction_multiply(&l, &s);
      q = k_fraction_sub(&l_p_s, &l_m_s);
    }

    k_fraction l_m_2 = k_fraction_multiply(&l, &c2);
    k_fraction p = k_fraction_sub(&l_m_2, &q);

    k_fraction h_p_ot = k_fraction_add(&h, &c1_3);
    k_fraction h_m_ot = k_fraction_sub(&h, &c1_3);

    r = k_hue_to_rgb(&p, &q, &h_p_ot);
    g = k_hue_to_rgb(&p, &q, &h);
    b = k_hue_to_rgb(&p, &q, &h_m_ot);
  }

  r = k_fraction_multiply(&r, &c255);
  g = k_fraction_multiply(&g, &c255);
  b = k_fraction_multiply(&b, &c255);

  r = k_fraction_round(&r);
  g = k_fraction_round(&g);
  b = k_fraction_round(&b);

  k_rgb_color rgb = {
      .red = k_fraction_to_whole_number(&r),
      .green = k_fraction_to_whole_number(&g),
      .blue = k_fraction_to_whole_number(&b),
  };

  kdbgln("Converted to rgb(%u, %u, %u).", rgb.red, rgb.green, rgb.blue);
  return rgb;
}