#ifndef _CONFIG_H_
#define _CONFIG_H_

/* Falls auf den Boards gearbeitet wird am besten
 * die nächste Zeile auskommentieren
 */
#define __QEMU__

/**
 * \file config.h
 *
 * Enthält defines und static Funktionen zum testen der
 * Implementierung. Wir tauschen diese Werte/Funktionen beim
 * Korrigieren zum Testen aus. Bitte fügt hier KEINE weiteren
 * defines oÄ ein. Ihr könnt diese Werte zum Testen natürlich
 * auch gerne selbst verändern.
 */

/* Include-Pfad bitte gegenfalls anpassen */
#include <kernel/kprintf.h>
static inline void test_kprintf(void) {
  /* Nur für Aufgabenblatt 1
   * Hier Test Funktion für kprintf schreiben und
   * nach dem vollständigen initialisieren aufrufen
   */
  kprintf("beef == %x\n", 0xbeef);
  kprintf("0xff == %p\n", 0xff);
  kprintf("1234 == %u\n", 1234);
  kprintf("-123 == %i\n", -123);
  kprintf("tree == %c%c%c%c\n", 't', 'r', 'e', 'e');
  kprintf("complete %s\n", "this sentence!");
  kprintf("I’ll have two number %us, a number %u large, a number %u with extra dip, a number %i, two number %u%c, one with cheese, and a large %s.\n", 9, 9, 6, 7, 45, 's', "soda");
  kprintf("00001234 == %08u\n", 1234);
  kprintf("-0000123 == %08i\n", -123);
  kprintf("    -123 == %8i\n", -123);
  kprintf("      1234567890 == %16u\n", 1234567890);
  kprintf("1234567890 == %8u\n", 1234567890);
  int length = kprintf("Dieser Satz hat %u Buchstaben.\n", 31);
  kprintf("Der letzte Satz hatte %u Buchstaben.\n", length);
}

/**
 * Erst ab Aufgabenblatt 2 relevant
 */

#ifdef __QEMU__
/* Werte zum testen unter QEMU */
#define BUSY_WAIT_COUNTER 3000000
#else
/* Werte zum testen auf der Hardware */
#define BUSY_WAIT_COUNTER 30000
#endif // __QEMU__

// Wir testen nur mit Werten die durch 2^n darstellbar sind
#define UART_INPUT_BUFFER_SIZE 128

// Timer Interrupt Interval zum testen in Mikrosekunden
// Systimer taktet mit 1MHz
// 1000000 -> 1 Sekunde
#define TIMER_INTERVAL 1000000

#endif // _CONFIG_H_
