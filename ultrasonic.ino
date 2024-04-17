//pins for ultrasonic sensor
#define echoPin A9
#define trigPin A10

//ultrasonic variable
double duration; // variable for the duration of sound wave travel
float obs_dist; // variable for the distance measurement

void setupUltra() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

uint32_t getDistance()
{
  // clears trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH);
  unsigned long obs_dist = duration * 0.034 / 2;
  //Serial.print(obs_dist);
  //Serial.println(" cm");
  return obs_dist;
}

void sendDistance() {

  uint32_t ultrasonicDist = getDistance();

  TPacket distancePacket;
  distancePacket.packetType = PACKET_TYPE_RESPONSE;
  distancePacket.command = RESP_DIST;
  distancePacket.params[0] = ultrasonicDist;
  sendResponse(&distancePacket);

  //sendOK();
}