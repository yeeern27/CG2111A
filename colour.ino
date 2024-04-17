/* Alex's Colour Sensor */

#define S0 22
#define S1 23
#define S2 24
#define S3 25
#define colourOut A8

unsigned long redFreq;
unsigned long greenFreq;
unsigned long blueFreq;
int colour = 0;

void setupColour() {
  //Serial.begin(9600);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(colourOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

// take 5 readings and return average frequency of Alex's colour sensor
int avgFreq() {
  int reading = 0;
  int total = 0;
  for (int i = 0; i < 5; i++) {
    reading = pulseIn(colourOut, LOW);
    total += reading;
    delay(100);
  }

  return total / 5;
}

// Find the colour of the paper
// Determination of the exact colour will be handled on alex-pi.cpp
void getColour() {
  // RED
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(100);
  redFreq = avgFreq();
  delay(100);

  // GREEN
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(100);
  greenFreq = avgFreq();
  delay(100);

  // BLUE
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(100);
  blueFreq = avgFreq();
  delay(100);


  //      Serial.print("Red: ");
  //      Serial.println(redFreq);
  //      Serial.print("Green: ");
  //      Serial.println(greenFreq);
  //      Serial.print("Blue: ");
  //      Serial.println(blueFreq);

}

/*
  returns:
  0 - white
  1 - red
  2 - green
  else
  3 - error
*/

int determineColour() {
  int redColour = redFreq;
  int greenColour = greenFreq;
  int blueColour = blueFreq;
  
  if (redColour > greenColour && greenColour > blueColour && blueColour > 270) { //green detected
    //Serial.println("GREEN");
    colour = 2;
    return colour;
  }
  if (redColour > greenColour && greenColour > blueColour) {
    //Serial.println("WHITE");
    colour = 0;
    return colour;
  }

  if (blueColour < greenColour && redColour < blueColour) { //red detected
    //Serial.println("RED");
    colour = 1;
    return colour;
  }
  colour = 3;
  return colour;
}

void sendColour()
{
  TPacket colourPacket;
  colourPacket.packetType = PACKET_TYPE_RESPONSE;
  colourPacket.command = RESP_COLOUR;

  colourPacket.params[0] = redFreq;
  colourPacket.params[1] = greenFreq;
  colourPacket.params[2] = blueFreq;
  colourPacket.params[3] = determineColour(); // 0, 1, 2, 3
  sendResponse(&colourPacket);

}

