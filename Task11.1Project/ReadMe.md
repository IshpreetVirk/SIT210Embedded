
# Net House Monitoring and Security System

## Project Overview

The Net House Monitoring and Security System is an embedded system developed using an Arduino Nano 33 IoT and Raspberry Pi 4.

The system monitors temperature, humidity, light intensity and vibration. Sensor readings are transmitted to the Raspberry Pi using MQTT through an EMQX Cloud broker.

The Raspberry Pi displays the readings on a dashboard and activates alerts when the vibration threshold is reached.

## Hardware Components

- Arduino Nano 33 IoT
- Raspberry Pi 4
- DHT22 temperature and humidity sensor
- BH1750 light sensor
- Spring vibration sensor
- Red LED
- Buzzer
- Resistors and jumper wires

## Technologies Used

- Arduino C++
- Python
- MQTT
- EMQX Cloud
- Tkinter
- Gmail SMTP

## System Features

- Real-time temperature monitoring
- Humidity monitoring
- Light intensity monitoring
- Vibration detection
- Adjustable alert thresholds
- Five-second buzzer alert
- Five-second red LED alert
- Email notifications


## MQTT Topics

- smartflow/temperature
- smartflow/humidity
- smartflow/light
- smartflow/vibration_pulses
- smartflow/red_led

## Hardware and Demonstration

Project circuit diagrams, hardware photographs and demonstration materials are included in the Documentation folder.
