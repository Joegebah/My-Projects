# STM32 Embedded Systems — von Bare-Metal zu HAL

Selbstgesteuertes Lernprojekt zur hardwarenahen Programmierung auf einem
STM32 Nucleo-F411RE. Alle Treiber sind zunächst **auf Registerebene** von Grund
auf implementiert — ohne HAL, ohne generierten Code, ohne Bibliotheken außer
`<stdint.h>`. Anschließend wurde dieselbe Aufgabe mit **STM32CubeMX und der HAL**
umgesetzt, um beide Abstraktionsebenen zu vergleichen.

---

## Warum zuerst ohne HAL

Ein Aufruf wie `HAL_I2C_Mem_Read()` liefert schnell ein Ergebnis, erklärt aber
nicht, was darunter passiert. Ziel dieses Projekts war, die Hardware tatsächlich
zu verstehen: was ein Register ist, wie eine Peripherie adressiert wird und warum
jeder einzelne Konfigurationsschritt notwendig ist.

Dafür habe ich die Registerstrukturen des Controllers anhand des
Referenzhandbuchs (RM0383) selbst als C-Structs nachgebildet — inklusive
reservierter Lücken, damit jedes Register an seinem korrekten Offset liegt.
Dadurch wird sichtbar, dass ein Zugriff wie `I2C1->CR1` nichts anderes ist als
*Basisadresse plus Feld-Offset*.

---

## Projekte

### 1 · GPIO (Bare-Metal)
Ansteuerung der Onboard-LED über direkte Registerzugriffe, anschließend
Erweiterung um den Taster als Eingang.

**Konzepte:** Memory-Mapped I/O · `volatile` · Bitmasken (Setzen, Löschen,
Toggeln, Testen) · Takt-Gating über RCC · Pull-up-Logik am Taster

### 2 · UART (Bare-Metal)
Sendetreiber von Grund auf: Takt freischalten, Pin in den Alternate-Function-
Modus versetzen, Baudrate über Taktteiler einstellen, Statusflag abfragen.

**Konzepte:** Alternate-Function-Multiplexing · AF-Nummer aus dem Datenblatt ·
Baudraten-Teiler (`BRR = Takt / Baudrate`) · Polling des TXE-Flags ·
Umwandlung von Zahlen in darstellbare Zeichen

### 3 · I²C & IMU — Digitale Wasserwaage (Bare-Metal)
Vollständiger I²C-Treiber (Lesen und Schreiben) zur Anbindung einer
MPU6050-IMU. Der Sensor wird aus dem Sleep-Modus geweckt, die
Beschleunigungswerte ausgelesen und über Trigonometrie in Neigungswinkel
umgerechnet, die live über UART ausgegeben werden.

**Konzepte:** Open-Drain und Pull-ups als Voraussetzung für einen geteilten Bus ·
START / Adressierung / ACK-NACK / Repeated START / STOP · Statusflag-Polling ·
16-Bit-Werte aus High- und Low-Byte zusammensetzen · vorzeichenbehaftete
Sensordaten · Neigungsberechnung über `atan2`

### 4 · Portierung auf HAL & Bewegungsmelder
Dieselbe Sensoranbindung, umgesetzt mit STM32CubeMX und der HAL — zum direkten
Vergleich beider Ebenen. Darauf aufbauend ein Bewegungsmelder, der aufeinander
folgende Messwerte vergleicht und bei Überschreiten eines Schwellwerts auslöst.

**Konzepte:** grafische Peripherie-Konfiguration (`.ioc`) · Code-Generierung und
`USER CODE`-Marker · HAL-Handles · Mehrbyte-Lesezugriff in einem Aufruf ·
Auswertung von Rückgabewerten (`HAL_StatusTypeDef`) · Schwellwertlogik

---

## Was der Vergleich zeigt

| | Bare-Metal | HAL |
|---|---|---|
| I²C-Register lesen | ~25 Zeilen Transaktionslogik | ein Funktionsaufruf |
| Peripherie-Setup | manuell aus dem Referenzhandbuch | grafisch konfiguriert |
| Fehlerbehandlung | eigene Timeouts nötig | Rückgabewert und Timeout integriert |
| Verständnis | vollständig sichtbar | abstrahiert |

Beide Ansätze haben ihre Berechtigung. Entscheidend ist, die untere Ebene zu
kennen — dann ist die HAL eine Arbeitserleichterung und keine Blackbox.

---

## Nennenswerte Fehlersuchen

**Falscher Sensorwert, korrekter Code.** Das WHO_AM_I-Register lieferte
konstant einen unerwarteten Wert. Statt den Code umzuschreiben habe ich
schrittweise verifiziert: Adressbestätigung geprüft, per Breakpoint kontrolliert,
dass die richtige Registernummer im Datenregister liegt, und festgestellt, dass
der Wert über alle Durchläufe exakt reproduzierbar war. Ein Timing-Problem
erzeugt schwankende Werte — ein konstanter Wert bedeutet, dass der Chip korrekt
antwortet. Auf dem Breakout-Board saß ein kompatibler Nachbau mit abweichender
Kennung.

**Hard Fault bei der ersten Fließkomma-Operation.** Die FPU des Cortex-M4 ist
nach dem Reset deaktiviert. Ohne explizite Freischaltung über das CPACR-Register
führt die erste Berechnung direkt in einen Hard Fault — das Projekt kompiliert
dabei fehlerfrei.

**I²C meldet dauerhaft HAL_BUSY.** Eingefrorene, über alle Durchläufe identische
Messwerte wiesen darauf hin, dass der Puffer nie beschrieben wurde. Die Auswertung
des Rückgabewerts bestätigte: die Transaktion wurde gar nicht erst gestartet. Der
Bus hing in einem blockierten Zustand — behoben durch vollständiges Trennen der
Spannungsversorgung.

---

## Hardware & Werkzeuge

| | |
|---|---|
| Board | STM32 Nucleo-F411RE (Cortex-M4) |
| Sensor | GY-521 Breakout mit MPU6050-kompatibler IMU |
| Bus | I²C1, Standard Mode 100 kHz, PB8 / PB9 |
| Ausgabe | USART2 über ST-Link Virtual COM Port, 115200 Baud |
| Referenzen | RM0383 · Datenblatt STM32F411 · MPU6050 Register Map |
| Toolchain | STM32CubeIDE · STM32CubeMX · VS Code · ARM GCC |

---

## Nächste Schritte

- Interrupts statt Polling (EXTI, Data-Ready-Interrupt des Sensors)
- SPI als zweites Bus-Protokoll
- Timer und PWM zur Motoransteuerung