#define BLYNK_TEMPLATE_ID "TMPL34Rp5McoT"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN "JYXON9DdJgKpdhuc1_I4bGmZ2RUnyMNt"

#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>

// Wi-Fi details
char ssid[] = "1234578";
char pass[] = "12345678";

// LED pins
#define LIVING_ROOM 2
#define BATHROOM 3
#define CLOSET 4

// Control the selected room
void controlRoom(String room, int state)
{
  if (room == "living room")
  {
    digitalWrite(LIVING_ROOM, state);
  }
  else if (room == "bathroom")
  {
    digitalWrite(BATHROOM, state);
  }
  else if (room == "closet")
  {
    digitalWrite(CLOSET, state);
  }
}

// Living Room - V0
BLYNK_WRITE(V0)
{
  int state = param.asInt();

  controlRoom("living room", state);
}

// Bathroom - V1
BLYNK_WRITE(V1)
{
  int state = param.asInt();

  controlRoom("bathroom", state);
}

// Closet - V2
BLYNK_WRITE(V2)
{
  int state = param.asInt();

  controlRoom("closet", state);
}

void setup()
{
  Serial.begin(115200);

  // LED pins
  pinMode(LIVING_ROOM, OUTPUT);
  pinMode(BATHROOM, OUTPUT);
  pinMode(CLOSET, OUTPUT);

  // Start with LEDs OFF
  digitalWrite(LIVING_ROOM, LOW);
  digitalWrite(BATHROOM, LOW);
  digitalWrite(CLOSET, LOW);

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Smart Home connected!");
}

void loop()
{
  Blynk.run();
}