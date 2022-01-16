# BSprakt

## Logging Dokumentation

Dieses System hat eine Logging Implementierung mit überraschend vielen Stellschrauben, die wir hier einzeln vorstellen.

### Allgemeines

Die Ausgabe von Logs erfolgt durch die Macros `(k)dbgln(...)`, `(k)warnln(...)` und `(k)panicln(...)` definiert in `(k)debug.h`, wobei das `(k)` für die Kernel-Variante steht.  

Warnungen und Panics geben beide vor dem Text ein `[WARNING]` bzw. `[PANIC]` Label aus, um diese Nachrichten einfach von unwichtigeren unterscheiden zu können.  

Der einzige Unterschied zwischen der Kernel- und der User-Variante liegt darin, dass mit einem `panicln(...)` der aktuelle Thread beendet wird und mit einem `kpanicln(...)` der gesamte Kernel.

### Modularität

Da alles in einer Header-Datei definiert ist, ist es möglich, verschiedene Einstellungen für verschiedene Subsysteme/Dateien zu haben. Das gilt für alle änderbaren Einstellungen, die wir gleich vorstellen werden.  

Es ist nur wichtig, diese Änderungen vor dem `#include <.../lib/(k)debug.h>` zu machen, weil sonst automatisch die Standardeinstellungen verwendet werden. Für globale Änderungen können die Standardeinstellungen in der Header Datei angepasst werden.

### Logstufen

Es gibt drei Logstufen, nach Wichtigkeit sortiert sind das

1. `PANIC_LEVEL`
2. `WARNING_LEVEL`
3. `DEBUG_LEVEL`

Sie bestimmen, ab welcher Wichtigkeitsstufe Nachrichten ausgegeben werden sollen. Mit `WARNING_LEVEL` werden zum Beispiel `(k)warnln(...)` und `(k)panicln(...)` ausgegeben, aber nicht `(k)dbgln(...)`. Die Logstufe kann mit `#define LOG_LEVEL ...` geändert werden. Der Default Wert ist `DEBUG_LEVEL`.

### Subsystem-Labels

Mit `LOG_LABEL "string"` kann ein Name für ein Subsystem festgelegt werden, der vor allen Nachrichten ausgegeben wird. Das macht es einfacher, Output von verschiedenen Subsystemen zu unterscheiden. Falls kein eigenes Label definiert ist, wird `"Unknown"` benutzt.

### Automatische Subsystem-Farben

Alle Labels eines bestimmten Subsystems werden per Default anhand einer berechneten Farbe basierend auf dem `LOG_LABEL` gefärbt. Die Farben werden im HSL-System bestimmt, sie sollten also alle mehr oder weniger lesbar sein.  

Falls ihr keine Farbausgabe wollt oder euer Terminal kein `truecolor` versteht, könnt ihr diese Funktionalität mit `#define LOG_COLORED_OUTPUT false` ausschalten.

## Syscall Dokumentation

### Zeichen einlesen (nicht-blockierend)

```C
char sys$read_character();
```
Verschiebt den aufrufenden Thread in eine Warteliste, die nicht vom Scheduler erfasst wird. Alte unverarbeitete und neu eingegebe Zeichen werden der Reihe nach Threads in dieser Liste zugewiesen.

Sobald ein Thread ein Zeichen zugewiesen bekommen hat, wird er wieder in die vom Scheduler erfasste Ready Liste verschoben.

### Zeichen ausgeben

```C
void sys$output_character(char ch);
```
Gibt `ch` mithilfe des UART im Terminal aus.

### Threads erstellen

```C
int sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size);
```
Erstellt einen neuen Thread, der vom Scheduler erfasst wird.

### Argumente

1. `func`: Funktion, die vom neuen Thread ausgeführt wird.
2. `args`: Argumente für die ausgeführte Funktion.
3. `args_size`: Größe von `args` in Bytes.

### Rückgabewerte

1. Gibt `-EINVAL` zurück, wenn `args_size` > `STACK_SIZE` ist.
2. Gibt `-EBUSY` zurück, wenn zurzeit keine Threads verfügbar sind.

### Zeiterfassung

```C
uint32_t sys$get_time();
```
Gibt die unteren 32bit des Systimers zurück und wird im Moment verwendet, um blockierendes Warten zu implementieren.

### Warten (nicht-blockierend)

```C
int sys$stall_thread(unsigned ms);
```
Verschiebt den aufrufenden Thread für `ms` Millisekunden in eine Warteliste, die nicht vom Scheduler erfasst wird. Für kleine Werte von `ms`, für die kein Interrupt möglich ist, wird der aufrufende Thread stattdessen direkt an das Ende der Ready Liste angehängt.

### Rückgabewerte
1. Gibt `-EINVAL` zurück, wenn `ms` zu groß ist, um ohne Overflow zur Hz-Frequenz des Systimers konvertiert zu werden.

### Threads beenden

```C
_Noreturn void sys$exit_thread();
```
Beendet den aufrufenden Thread und setzt seinen Thread Control Block auf den Initialzustand zurück. Werte in den General Purpose Registern `r0-r12` sowie gespeicherte Daten auf dem Stack werden nicht überschrieben.