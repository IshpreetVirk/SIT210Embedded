Task 1.1P - Switching ON Lights by Ishpreet Singh (2510994809)

This project uses an Arduino Nano, two LEDs, two resistors, and a push button to control the porch and hallway lights.
When the button is pressed, both lights turn ON. The yellow porch light stays ON for 30 seconds, while the red hallway light stays ON for 60 seconds.

Modular Programming
The program uses a separate function called `runLightingSequence()` to control the lighting process.

* `setup()` sets the porch, hallway and button pins and prepares the Arduino.
* `loop()` checks whether the button has been pressed.
* `runLightingSequence()` turns both lights ON and controls when each light is switched OFF.

This makes the program easier to understand because the main loop only checks the button, while the lighting sequence is handled separately.

Circuit
* D8 → Green Porch LED
* D2 → Red Hallway LED
* D6 → Push Button
* LEDs → Connected through the one resistor
* GND → Common ground

Result
When the button is pressed, both lights turn ON. After 30 seconds, the porch light turns OFF, and after another 30 seconds, the hallway light also turns OFF.
