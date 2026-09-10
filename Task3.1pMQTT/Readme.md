# Sunlight Trigger & Notification System

This project uses an Arduino Nano 33 IoT with a BH1750 light sensor to detect when a terrarium is exposed to sunlight, and sends an email notification whenever this state changes.

## How it works
The Arduino reads lux values from the BH1750 sensor over I2C and publishes them via MQTT to a HiveMQ Cloud broker on the `sensor/light` topic. A Node-RED flow subscribes to this topic, and a function node compares each reading against a 500 lux threshold to determine whether the terrarium is currently in sunlight. The threshold value was chosen by testing sensor readings near and far from a light source.

To avoid repeated alerts, the function node only generates a message when the light state actually changes (sunlight starting or stopping), using Node-RED's context storage to track the previous state. This message is then passed to an Email node, which sends a notification with the current lux reading.

## Files
-  Task3.1pMQTT.ino – Arduino sketch (sensor reading + MQTT publish)
-  hello.h - the file with id, pass, wifi pass and other confidential information so in it, it will show confidential information.
-  all the screenshots – debug output, Node-RED function node logic and email notification proof
