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
  kprintf("00001234 == %08u\n", 1234);
  kprintf("-0000123 == %08i\n", -123);
  kprintf("    -123 == %8i\n", -123);
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