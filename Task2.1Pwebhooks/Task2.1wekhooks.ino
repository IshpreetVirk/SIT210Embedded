#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>

#include "untold.h"

//defining the DHT sensor
#define DHT_DATA_PIN 2
#define DHT_SENSOR_TYPE DHT22

DHT roomSensor(DHT_DATA_PIN, DHT_SENSOR_TYPE);
BH1750 lightSensor;

WiFiClient webClient;

//thingSpeak channel details
unsigned long myChannelNumber = CH_ID;
const char* myWriteAPIKey = WRITE_APIKEY;

// Sensor readings and initially 0
float roomTemperature = 0;
float roomHumidity = 0;
float lightIntensity = 0;


//connecting Arduino to Wi-Fi
void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(SSID);

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(SSID, PASS);
        Serial.print(".");
        delay(3000);
    }

    Serial.println();
    Serial.println("Wi-Fi connected.");
}


//reading the values from both sensors
void readRoomSensors()
{
    roomTemperature = roomSensor.readTemperature();
    roomHumidity = roomSensor.readHumidity();
    lightIntensity = lightSensor.readLightLevel();

    Serial.println();
    Serial.println("Current room conditions:");

    Serial.print("Temperature: ");
    Serial.print(roomTemperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(roomHumidity);
    Serial.println(" %");

    Serial.print("Light Intensity: ");
    Serial.print(lightIntensity);
    Serial.println(" lux");
}


//send the three readings to ThingSpeak
void uploadRoomData()
{
    ThingSpeak.setField(1, roomTemperature);
    ThingSpeak.setField(2, roomHumidity);
    ThingSpeak.setField(3, lightIntensity);

    int uploadResult = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (uploadResult == 200)
    {
        Serial.println("ThingSpeak update successful.");
    }
    else
    {
        Serial.print("ThingSpeak update failed. HTTP error code: ");
        Serial.println(uploadResult);
    }
}


void setup()
{
    Serial.begin(115200);

    while (!Serial)
    {
        ;
    }

    //starting the sensors
    roomSensor.begin();
    Wire.begin();

    if (lightSensor.begin())
    {
        Serial.println("BH1750 light sensor ready.");
    }
    else
    {
        Serial.println("BH1750 light sensor not detected.");
    }

    //check Wi-Fi hardware
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Wi-Fi module not detected.");
        while (true)
        {
            ;
        }
    }

    ThingSpeak.begin(webClient);

    Serial.println("Smart room monitoring system started.");
}


void loop()
{
    connectWiFi();

    readRoomSensors();

    //checking sensor reading 
    if (isnan(roomTemperature) ||
        isnan(roomHumidity) ||
        isnan(lightIntensity))
    {
        Serial.println("Error: Unable to read sensor data.");
    }
    else
    {
        uploadRoomData();
    }

    //updating the value every 30seconds
    Serial.println("Waiting 30 seconds for the next update...");
    delay(30000);
}