//Setting up the pins
const int porchLight = 2;    
const int hallwayLight = 8; 
const int buttonPin = 6;           

void setup() 
{
  //Initalizing the pins
  pinMode(porchLight, OUTPUT);
  pinMode(hallwayLight, OUTPUT);
  pinMode(buttonPin, INPUT);

}

void loop() 
{
  //Initializing the button
  int buttonState = digitalRead(buttonPin);


  if (buttonState == HIGH) 
  {
    lightsOn(); 
  }
}

// Function to control the lights as per linda's requirements
void lightsOn() 
{
  //Both the lights turned on
  digitalWrite(porchLight, HIGH);
  digitalWrite(hallwayLight, HIGH);

  // First wait for 30 sec
  delay(30000); 
  
  // Then turn off the porch light
  digitalWrite(porchLight, LOW);

  //Second wait for 30 sec
  delay(30000); 
  
  // Then turn off the hallway light
  digitalWrite(hallwayLight, LOW);
}
