// Set the pins for the lights and button
const int porchLight_yellow = 2;
const int hallwayLight_red = 3;
const int buttonPin = 4;


void setup() {
  // Set light pins as output
  pinMode(porchLight_yellow, OUTPUT);
  pinMode(hallwayLight_red, OUTPUT);

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
  digitalWrite(porchLight_yellow, HIGH);
  digitalWrite(hallwayLight_red, HIGH);
  Serial.println("Both lights ON.");

  // Wait for 30 seconds
  delay(30000);

  // Turn the yellow porch light off
  digitalWrite(porchLight_yellow, LOW);
  Serial.println("Porch yellow light OFF.");

  // Wait for another 30 seconds
  delay(30000);

  // Turn the red hallway light off
  digitalWrite(hallwayLight_red, LOW);
  Serial.println("Hallway red light OFF. Done.");
}