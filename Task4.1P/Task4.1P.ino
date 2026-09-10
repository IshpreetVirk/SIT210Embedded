#include <Wire.h>
#include <BH1750.h>

BH1750 lightSensor;

// Using these pins for the sensors, button, and LEDs
const int PIR_PIN = 2;
const int BUTTON_PIN = 3;
const int LED1_PIN = 4;
const int LED2_PIN = 5;

// Setting the light level below which the system is considering it dark
const float DARK_LEVEL = 50.0;

// Setting the time for which the lights are staying on after being triggered
const unsigned long LIGHT_TIME = 30000;

// Using these flags for recording when an interrupt is being triggered
volatile bool pirTriggered = false;
volatile bool buttonTriggered = false;

// Using these variables for keeping track of the lights and their timer
unsigned long lightStartTime = 0;
bool lightsOn = false;

// Running when motion is being detected by the PIR sensor
void pirInterrupt()
{
  pirTriggered = true;
}

// Running when the button is being pressed
void buttonInterrupt()
{
  buttonTriggered = true;
}

void setup()
{
  // Starting the Serial Monitor for displaying system messages
  Serial.begin(9600);

  // Setting the sensors as inputs and the LEDs as outputs
  pinMode(PIR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Making sure both lights are starting in the OFF state
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  // Starting communication with the BH1750 light sensor
  Wire.begin();

  // Checking whether the BH1750 sensor is being connected correctly
  if (lightSensor.begin())
  {
    Serial.println("BH1750 ready.");
  }
  else
  {
    Serial.println("BH1750 not detected.");
  }

  // Setting the PIR interrupt to trigger when its signal is changing from LOW to HIGH
  attachInterrupt(
    digitalPinToInterrupt(PIR_PIN),
    pirInterrupt,
    RISING
  );

  // Setting the button interrupt to trigger when the button is changing from HIGH to LOW
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonInterrupt,
    FALLING
  );

  // Confirming that the system is finishing its setup
  Serial.println("System ready.");
}

void loop()
{
  // Checking if the PIR interrupt has detected motion
  if (pirTriggered)
  {
    // Resetting the flag so the same event is not being processed repeatedly
    pirTriggered = false;

    // Reading the current light level from the BH1750
    float lux = lightSensor.readLightLevel();

    // Displaying the detected motion and current light level
    Serial.print("Motion detected. Light: ");
    Serial.print(lux);
    Serial.println(" lux");

    // Turning the lights on automatically when the area is being detected as dark
    if (lux < DARK_LEVEL)
    {
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);

      // Recording that the lights are being turned on and starting the 30-second timer
      lightsOn = true;
      lightStartTime = millis();

      Serial.println("Dark - Lights ON for 30 seconds");
    }
    else
    {
      // Keeping the lights off when enough light is already being detected
      Serial.println("Bright - Lights remain OFF");
    }
  }

  // Checking if the button interrupt has been triggered
  if (buttonTriggered)
  {
    // Resetting the flag after the button press is being detected
    buttonTriggered = false;

    // Using the button as a manual backup for turning both lights on
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);

    // Starting the 30-second timer for the manually activated lights
    lightsOn = true;
    lightStartTime = millis();

    Serial.println("Button pressed - Lights ON for 30 seconds");
  }

  // Checking whether the lights have been staying on for 30 seconds
  if (lightsOn &&
      millis() - lightStartTime >= LIGHT_TIME)
  {
    // Turning both LEDs off when the timer is finishing
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);

    // Updating the light state so the timer is stopping
    lightsOn = false;

    Serial.println("30 seconds completed - Lights OFF");
  }

  // Adding a small delay before the system is checking again
  delay(50);
}