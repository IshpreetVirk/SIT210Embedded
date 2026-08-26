#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Wire.h>

#include "hello.h"

BH1750 lightMeter;

WiFiSSLClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectWiFi() {

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 60) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

  } else {

    Serial.println("WiFi connection failed!");

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
  }
}


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

      Serial.print("HiveMQ connection failed. MQTT state: ");
      Serial.println(mqttClient.state());

      delay(5000);
    }
  }
}


void setup() {

  Serial.begin(9600);

  delay(1000);

  Serial.println();
  Serial.println(" Arduino Nano 33 IoT");
  Serial.println(" BH1750 + HiveMQ MQTT");
 
  // Initializing I2C
  Wire.begin();

  // Initializing BH1750
  if (lightMeter.begin()) {

    Serial.println("BH1750 initialized successfully!");

  } else {

    Serial.println("BH1750 initialization failed!");

    while (1);
  }

  // Connecting to Wi-Fi
  connectWiFi();

  // Configuring MQTT server
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  // Connecting to HiveMQ if Wi-Fi connected
  if (WiFi.status() == WL_CONNECTED) {

    connectMQTT();

  } else {

    Serial.println("HiveMQ connection skipped because Wi-Fi failed.");
  }

  Serial.println();
  Serial.println("System ready!");
}


void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println();
    Serial.println("Wi-Fi connection lost!");

    connectWiFi();
  }

  // Checking MQTT connection
  if (WiFi.status() == WL_CONNECTED) {

    if (!mqttClient.connected()) {

      connectMQTT();
    }

    mqttClient.loop();
  }

  // Reading light level from BH1750
  float lux = lightMeter.readLightLevel();

  // Display light level
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  // Converting light value to String
  String message = String(lux, 2);

  // Publishing light value to HiveMQ
  if (mqttClient.connected()) {

    if (mqttClient.publish(MQTT_TOPIC, message.c_str())) {

      Serial.print("Published to ");
      Serial.print(MQTT_TOPIC);
      Serial.print(": ");
      Serial.println(message);

    } else {

      Serial.println("MQTT publish failed!");
    }
  }

  delay(1000);
}