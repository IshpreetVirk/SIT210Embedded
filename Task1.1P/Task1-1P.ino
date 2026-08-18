// Set the pins for the lights and button
const int porchLight = 2;
const int hallwayLight = 8;
const int buttonPin = 6;


void setup() {
  // Set light pins as output
  pinMode(porchLight, OUTPUT);
  pinMode(hallwayLight, OUTPUT);

  // Set button pin as input
  pinMode(buttonPin, INPUT);

  Serial.begin(9600);
}


void loop() {
  // Check if the button is pressed
  int buttonState = digitalRead(buttonPin);

  // Start the lights when button is pressed
  if (buttonState == HIGH) {
    Serial.println("Linda has arrived! Activating Smart Lights...");
    runLightingSequence();
  }
}


// This function controls the lights
void runLightingSequence() {

  // Turn both lights on
  digitalWrite(porchLight, HIGH);
  digitalWrite(hallwayLight, HIGH);
  Serial.println("Both lights ON.");

  // Wait for 30 seconds
  delay(30000);

  // Turn the yellow porch light off
  digitalWrite(porchLight, LOW);
  Serial.println("Porch yellow light OFF.");

  // Wait for another 30 seconds
  delay(30000);

  // Turn the red hallway light off
  digitalWrite(hallwayLight, LOW);
  Serial.println("Hallway red light OFF. Done.");
}
