Task 2.1P - Sending Temperature and Light Data to the Web

What this does

This is for a task where I had to read temperature, humidity, and light from an Arduino and send the readings to ThingSpeak so they can be seen on a web page. Data updates every 30 seconds.

What I used: 
Arduino Nano 33 IoT;
DHT22 sensor (temperature + humidity);
BH1750 sensor (light);
ThingSpeak, WiFiNINA, DHT, and BH1750 libraries

How it works: 
connectWiFi() connects the Arduino to WiFi;
readRoomSensors() reads the temp, humidity, and light values;
uploadRoomData() sends the values to ThingSpeak using ThingSpeak.writeFields();
loop() runs all of this every 30 seconds

Files
Task2.1webhooks.ino - the code;
circuitdiagram.png - circuit made in Wokwi;
thingspeak.png - chart showing the readings over 5 mins;
untold.h 

Note

My WiFi and ThingSpeak API key are in a separate untold.h file which I didn't show since it has private info.
