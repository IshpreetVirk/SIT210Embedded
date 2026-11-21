
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

// HARDWARE PINS 

#define DHTPIN 3
#define DHTTYPE DHT22

#define VIBRATION_PIN 4
#define RED_LED_PIN 7

// Change to HIGH if your vibration module
// produces HIGH when vibration is detected.
#define VIBRATION_ACTIVE_STATE LOW

//  WIFI SETTINGS

const char WIFI_SSID[] = "YOUR_WIFI_NAME";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

//   MQTT SETTINGS 

const char MQTT_BROKER[] =
  "r167aa16.ala.asia-southeast1.emqxsl.com";

const int MQTT_PORT = 8883;

const char MQTT_USERNAME[] = "YOUR_EMQX_USERNAME";
const char MQTT_PASSWORD[] = "YOUR_EMQX_PASSWORD";

//  MQTT TOPICS 

const char TEMP_TOPIC[] = "smartflow/temperature";
const char HUMIDITY_TOPIC[] = "smartflow/humidity";
const char LIGHT_TOPIC[] = "smartflow/light";

const char VIBRATION_TOPIC[] =
  "smartflow/vibration_pulses";

const char LED_COMMAND_TOPIC[] =
  "smartflow/red_led";

//  SENSOR OBJECTS 

WiFiSSLClient wifiClient;
PubSubClient mqtt(wifiClient);

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

//  TIMING 

const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long VIBRATION_WINDOW = 2000;
const unsigned long DEBOUNCE_TIME = 50;
const unsigned long LED_DURATION = 5000;

const unsigned long WIFI_RETRY_INTERVAL = 5000;
const unsigned long MQTT_RETRY_INTERVAL = 3000;

//  VARIABLES 

unsigned long lastSensorTime = 0;
unsigned long vibrationWindowStart = 0;
unsigned long lastVibrationTime = 0;
unsigned long ledStartTime = 0;

unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;

int vibrationPulseCount = 0;
int previousVibrationState = HIGH;

bool redLedOn = false;

//  RED LED CONTROL 

void setRedLed(bool status) {

  redLedOn = status;

  digitalWrite(
    RED_LED_PIN,
    status ? HIGH : LOW
  );

  if (status) {
    ledStartTime = millis();
    Serial.println("RED LED ON");
  } else {
    Serial.println("RED LED OFF");
  }
}

// MQTT CALLBACK 
// Receives commands from the Raspberry Pi to switch the red LED on or off.
// The LED also has a five-second safety timeout in the main loop.

void mqttCallback(
  char* topic,
  byte* payload,
  unsigned int length
) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  if (String(topic) == LED_COMMAND_TOPIC) {

    if (message == "ON") {
      setRedLed(true);
    }

    else if (message == "OFF") {
      setRedLed(false);
    }
  }
}

//       WIFI CONNECTION 
// Checks the Wi-Fi connection and attempts to reconnect if it is lost.
// This allows the Arduino to continue working without restarting manually.


void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if (millis() - lastWiFiAttempt <
      WIFI_RETRY_INTERVAL) {
    return;
  }

  lastWiFiAttempt = millis();

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

//  MQTT CONNECTION 

void connectMQTT() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqtt.connected()) {
    return;
  }

  if (millis() - lastMQTTAttempt <
      MQTT_RETRY_INTERVAL) {
    return;
  }

  lastMQTTAttempt = millis();

  Serial.println("Connecting to EMQX...");

  if (
    mqtt.connect(
      "NetHouseArduino",
      MQTT_USERNAME,
      MQTT_PASSWORD
    )
  ) {

    Serial.println("MQTT connected!");

    mqtt.subscribe(LED_COMMAND_TOPIC);

    Serial.println("Subscribed to red LED commands.");

  } else {

    Serial.print("MQTT connection failed. State: ");
    Serial.println(mqtt.state());
  }
}

//   MQTT PUBLISH 

void publishData(
  const char* topic,
  String value
) {

  if (mqtt.connected()) {

    mqtt.publish(
      topic,
      value.c_str()
    );
  }
}

// SENSOR READINGS 
// Reads the temperature, humidity and light intensity from the sensors.
// The readings are then published separately so the Raspberry Pi can process them.


void readSensors() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (
    !isnan(temperature) &&
    !isnan(humidity)
  ) {

    publishData(
      TEMP_TOPIC,
      String(temperature, 1)
    );

    publishData(
      HUMIDITY_TOPIC,
      String(humidity, 1)
    );

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

  } else {

    Serial.println("DHT22 reading failed.");
  }

  float lux = lightMeter.readLightLevel();

  if (lux >= 0) {

    publishData(
      LIGHT_TOPIC,
      String(lux, 1)
    );

    Serial.print("Light intensity: ");
    Serial.print(lux);
    Serial.println(" lux");

  } else {

    Serial.println("BH1750 reading failed.");
  }
}

//  VIBRATION DETECTION 


void countVibration() {

  int currentState = digitalRead(VIBRATION_PIN);

  unsigned long currentTime = millis();

  // Count a transition into the active state.
  // Debouncing prevents rapid electrical changes
  // from being counted as separate pulses.

  if (
    currentState == VIBRATION_ACTIVE_STATE &&
    previousVibrationState != VIBRATION_ACTIVE_STATE &&
    currentTime - lastVibrationTime >= DEBOUNCE_TIME
  ) {

    vibrationPulseCount++;

    lastVibrationTime = currentTime;
  }

  previousVibrationState = currentState;

  // Publish pulse count every 2 seconds.

  if (
    currentTime - vibrationWindowStart >=
    VIBRATION_WINDOW
  ) {

    publishData(
      VIBRATION_TOPIC,
      String(vibrationPulseCount)
    );

    Serial.print("Vibration pulses in 2 seconds: ");
    Serial.println(vibrationPulseCount);

    vibrationPulseCount = 0;

    vibrationWindowStart = currentTime;
  }
}

//SETUP

void setup() {

  Serial.begin(9600);

  pinMode(
    VIBRATION_PIN,
    INPUT_PULLUP
  );

  pinMode(
    RED_LED_PIN,
    OUTPUT
  );

  setRedLed(false);

  dht.begin();

  Wire.begin();

  lightMeter.begin(
    BH1750::CONTINUOUS_HIGH_RES_MODE
  );

  previousVibrationState =
    digitalRead(VIBRATION_PIN);

  vibrationWindowStart = millis();

  mqtt.setServer(
    MQTT_BROKER,
    MQTT_PORT
  );

  mqtt.setCallback(mqttCallback);

  mqtt.setKeepAlive(30);

  Serial.println("==============================");
  Serial.println("NET HOUSE MONITORING SYSTEM");
  Serial.println("==============================");
}

//  MAIN LOOP

void loop() {

  connectWiFi();

  connectMQTT();

  if (mqtt.connected()) {
    mqtt.loop();
  }

  unsigned long currentTime = millis();

  // Read vibration sensor continuously.
  countVibration();

  // Publish environmental readings every 2 seconds.
  if (
    currentTime - lastSensorTime >=
    SENSOR_INTERVAL
  ) {

    lastSensorTime = currentTime;

    readSensors();
  }

  // Safety timeout for red LED.
  if (
    redLedOn &&
    currentTime - ledStartTime >= LED_DURATION
  ) {

    setRedLed(false);
  }
}
