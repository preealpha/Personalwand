# Personalwand
Arduino Sketch für die Steurerung der Personalwand.
Benötigte Bibliothek: http://fastled.io/

## Konstruktion
Die LEDs sind vom Typ WS2812 und werden über Fast LED über Pin 12 angesteuert.

Die Position der Setcard wird mittels eines Hall-Effekt Sensors überwacht. Ändert sich die Position, ändert sich auch das Magnetfeld. Die Daten des Sensors sind Analog, da der Arduino nicht über genügend Analoge eingänge verfügt, Sind die Sensoren an einen Analogmultiplexer angeschlossen. Die verbauten Multiplexer haben 8 Kanäle. über eine 3 Bit Adresse kann der Kanal gewählt werden. Um genügen analoge Signale zu haben. sind die Multiplexer in Stufen geschaltet. Der erste Multiplexer (A) hat an jedem Kanal einen weiteren Multiplexer (B), so dass theroetisch 8x8 Analoge Signale durchgeschaltet werden können. Daher hat die Adresse für jeden Sensor 6 Bit. die ersten 3 Bit für den ersten Multiplexer (A) und die zweiten 3 Bit für den Multiplexer (B) dahinter. 

![Schema](https://github.com/preealpha/Personalwand/raw/master/Multiplex.png)

