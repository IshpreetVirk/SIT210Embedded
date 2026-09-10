#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Wire.h>


// ===============================
// Wi-Fi Details
// ===============================

#define WIFI_SSID "1234578"
#define WIFI_PASSWORD "12345678"


// ===============================
// HiveMQ Details
// ===============================

#define MQTT_SERVER "6fe51bf7a6cb4327b97ade299d32a64a.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_TOPIC "sensor/int"

#define MQTT_USER "mjain"
#define MQTT_PASSWORD "12345678"


// ===============================
// Objects
// ===============================

BH1750 lightMeter;

WiFiSSLClient wifiClient;
PubSubClient mqttClient(wifiClient);


// ===============================
// Connect to Wi-Fi
// ===============================

void connectWiFi() {

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 60) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

  } else {

    Serial.println("Wi-Fi connection FAILED!");

    Serial.print("Wi-Fi status: ");
    Serial.println(WiFi.status());
  }
}


// ===============================
// Connect to HiveMQ
// ===============================

void connectMQTT() {

  while (!mqttClient.connected()) {

    Serial.println();
    Serial.println("Connecting to HiveMQ Cloud...");

    if (mqttClient.connect(
          "ArduinoNano33IoT",
          MQTT_USER,
          MQTT_PASSWORD)) {

      Serial.println("HiveMQ connected!");

    } else {

      Serial.print("MQTT connection failed. State: ");
      Serial.println(mqttClient.state());

      delay(5000);
    }
  }
}


// ===============================
// Setup
// ===============================

void setup() {

  Serial.begin(9600);

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("Arduino Nano 33 IoT");
  Serial.println("BH1750 + HiveMQ MQTT");
  Serial.println("==============================");


  // =============================
  // Initialise I2C
  // =============================

  Serial.println("Starting I2C...");

  Wire.begin();

  Serial.println("I2C started.");


  // =============================
  // Initialise BH1750
  // =============================

  Serial.println("Starting BH1750...");

  if (lightMeter.begin()) {

    Serial.println("BH1750 initialized successfully!");

  } else {

    Serial.println("BH1750 initialization FAILED!");

    while (1);
  }


  // =============================
  // Connect Wi-Fi
  // =============================

  connectWiFi();


  // =============================
  // Configure MQTT
  // =============================

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);


  // =============================
  // Connect HiveMQ
  // =============================

  if (WiFi.status() == WL_CONNECTED) {

    connectMQTT();

  } else {

    Serial.println(
      "HiveMQ connection skipped because Wi-Fi failed."
    );
  }


  Serial.println();
  Serial.println("==============================");
  Serial.println("SYSTEM READY");
  Serial.println("==============================");
}


// ===============================
// Main Loop
// ===============================

void loop() {

  // =============================
  // Check Wi-Fi
  // =============================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println();
    Serial.println("Wi-Fi connection lost!");

    connectWiFi();
  }


  // =============================
  // Check MQTT
  // =============================

  if (WiFi.status() == WL_CONNECTED) {

    if (!mqttClient.connected()) {

      connectMQTT();
    }

    mqttClient.loop();
  }


  // =============================
  // Read BH1750
  // =============================

  float lux = lightMeter.readLightLevel();


  // =============================
  // Display Lux
  // =============================

  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");


  // =============================
  // Convert Lux to String
  // =============================

  String message = String(lux, 2);


  // =============================
  // Publish to HiveMQ
  // =============================

  if (mqttClient.connected()) {

    if (mqttClient.publish(
          MQTT_TOPIC,
          message.c_str())) {

      Serial.print("Published to ");
      Serial.print(MQTT_TOPIC);
      Serial.print(": ");
      Serial.println(message);

    } else {

      Serial.println("MQTT publish FAILED!");
    }

  } else {

    Serial.println("MQTT is NOT connected.");
  }


  // =============================
  // Wait 1 second
  // =============================

  delay(1000);
}