
#include <serialize.h>
#include "packet.h"
#include "constants.h"
#include <stdarg.h>
#include <math.h>
#include <avr/sleep.h>
volatile TDirection dir;

/*
   Alex's configuration constants
*/

// Number of ticks per revolution from the wheel encoder.

#define COUNTS_PER_REV      4

// Wheel circumference in cm.
// We will use this to calculate forward/backward distance traveled by taking revs * WHEEL_CIRC

#define WHEEL_CIRC          21
#define PI 3.141592654
#define ALEX_LENGTH  24
#define ALEX_BREADTH  15
float alexDiagonal = 0.0;
float alexCirc = 0.0;

/*
      Alex's State Variables
*/
unsigned long computeDeltaTicks(float ang) {
  unsigned long ticks = (unsigned long)((ang * alexCirc * COUNTS_PER_REV) / (360 * WHEEL_CIRC));
  return ticks;
}

// Store the ticks from Alex's left and right encoders.
volatile unsigned long leftForwardTicks;
volatile unsigned long rightForwardTicks;
volatile unsigned long leftReverseTicks;
volatile unsigned long rightReverseTicks;

volatile unsigned long leftForwardTicksTurns;
volatile unsigned long rightForwardTicksTurns;
volatile unsigned long leftReverseTicksTurns;
volatile unsigned long rightReverseTicksTurns;

// Store the revolutions on Alex's left and right wheels
volatile unsigned long leftRevs;
volatile unsigned long rightRevs;

// Forward and backward distance traveled
volatile unsigned long forwardDist;
volatile unsigned long reverseDist;

//variable to keep track of distance 
unsigned long deltaDist;
unsigned long newDist;

//variable to keep track of angle 
unsigned long deltaTicks;
unsigned long targetTicks;

//Setup and start codes for external interrupts and pullup resistors.
// Enable pull up resistors on pins 18 and 19
void enablePullups()
{
  DDRD &= 0b11110011; //setting PD2 and PD3 as inputs
  PORTD |= 0b00001100; //drive PD2 and PD3 HIGH

}

// Functions to be called by INT2 and INT3 ISRs.
void leftISR()
{
  if (dir == FORWARD)
    // update leftForwardTicks and forwardDist
  {
    leftForwardTicks++;
    forwardDist = (unsigned long) ((float) leftForwardTicks / COUNTS_PER_REV * WHEEL_CIRC);
  }
  else if (dir == BACKWARD)
    // update leftReverseTicks and reverseDist
  {
    leftReverseTicks++;
    reverseDist = (unsigned long) ((float) leftReverseTicks / COUNTS_PER_REV * WHEEL_CIRC);
  }
  else if (dir == LEFT)
    // left wheel spins backwards for left turn, so update leftReverseTicksTurns
  {
    leftReverseTicksTurns++;
  }
  else if (dir == RIGHT)
    // left wheel spins forwards for right turn, so update leftForwardTicksTurns
  {
    leftForwardTicksTurns++;
  }
}
void rightISR()
{
  if (dir == FORWARD)
    // update rightForwardTicks
  {
    rightForwardTicks++;
  }
  else if (dir == BACKWARD)
    // update rightReverseTicks
  {
    rightReverseTicks++;

  }
  else if (dir == LEFT)
    // right wheel spins forwards for left turn, so update rightForwardTicksTurns
  {
    rightForwardTicksTurns++;
  }
  else if (dir == RIGHT)
    // right wheel spins backwards for right turn, so update rightReverseTicksTurns
  {
    rightReverseTicksTurns++;
  }
}

// Set up the external interrupt pins INT2 and INT3 for falling edge triggered. Use bare-metal.
void setupEINT()
{
  // Use bare-metal to configure pins 18 and 19 to be
  // falling edge triggered. Remember to enable
  // the INT2 and INT3 interrupts.
  // Hint: Check pages 110 and 111 in the ATmega2560 Datasheet.
  cli();
  EICRA |= 0b10100000; //set INT2 and INT3 to falling edge
  EIMSK |= 0b00001100; //enable INT2 and INT3
  sei();
}

// Implement the external interrupt ISRs below.
// INT3 ISR should call leftISR while INT2 ISR should call rightISR.
ISR(INT2_vect) {
  rightISR();
}

ISR(INT3_vect) {
  leftISR();
}

/*
   Alex's setup and run codes

*/

void initializeState()
{
  clearCounters();
}

void handleCommand(TPacket *command)
{
  switch (command->command)
  {
    // For movement commands, param[0] = distance, param[1] = speed.
    case COMMAND_FORWARD:
      sendOK();
      forward((double) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_REVERSE:
      sendOK();
      backward((double) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_TURN_LEFT:
      sendOK();
      left((double) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_TURN_RIGHT:
      sendOK();
      right((double) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_STOP:
      sendOK();
      stop();
      break;

    case COMMAND_GET_STATS:
      sendStatus();
      break;

    case COMMAND_GET_DIST:
      sendOK();
      sendDistance();
      break;

    case COMMAND_GET_COLOUR:
      sendOK();
      getColour();
      determineColour();
      sendColour();
      break;

    case COMMAND_CLEAR_STATS:
      sendOK();
      clearOneCounter(command->params[0]);
      break;

    default:
      sendBadCommand();
  }
}

void waitForHello()
{
  int exit = 0;

  while (!exit)
  {
    TPacket hello;
    TResult result;

    do
    {
      result = readPacket(&hello);
    } while (result == PACKET_INCOMPLETE);

    if (result == PACKET_OK)
    {
      if (hello.packetType == PACKET_TYPE_HELLO)
      {
        sendOK();
        exit = 1;
      }
      else
        sendBadResponse();
    }
    else if (result == PACKET_BAD)
    {
      sendBadPacket();
    }
    else if (result == PACKET_CHECKSUM_BAD)
      sendBadChecksum();
  } // !exit
}

void setup() {
  // put your setup code here, to run once:
  alexDiagonal = sqrt((ALEX_LENGTH * ALEX_LENGTH) + (ALEX_BREADTH * ALEX_BREADTH));
  alexCirc = PI * alexDiagonal;

  cli();
  setupEINT();
  setupSerial();
  startSerial();
  setupColour();
  setupUltra();
  enablePullups();
  initializeState();
  sei();

}

void handlePacket(TPacket *packet)
{
  switch (packet->packetType)
  {
    case PACKET_TYPE_COMMAND:
      handleCommand(packet);
      break;

    case PACKET_TYPE_RESPONSE:
      break;

    case PACKET_TYPE_ERROR:
      break;

    case PACKET_TYPE_MESSAGE:
      break;

    case PACKET_TYPE_HELLO:
      break;
  }
}

void loop() {
  TPacket recvPacket; // This holds commands from the Pi

  TResult result = readPacket(&recvPacket);

  if (result == PACKET_OK)
    handlePacket(&recvPacket);
  else if (result == PACKET_BAD)
  {
    sendBadPacket();
  }
  else if (result == PACKET_CHECKSUM_BAD)
  {
    sendBadChecksum();
  }

  if (deltaDist > 0)
  {
    if (dir == FORWARD)
    {
      if (forwardDist > newDist)
      {
        deltaDist = 0;
        newDist = 0;
        stop();
        delay(200);
      }
    }
    else if (dir == BACKWARD)
    {
      if (reverseDist > newDist)
      {
        deltaDist = 0;
        newDist = 0;
        stop();
        delay(200);
      }
    }
    else if (dir == STOP)
    {
      deltaDist = 0;
      newDist = 0;
      stop();
      delay(200);
    }
  }

  if (deltaTicks > 0)
  {
    if (dir == LEFT)
    {
      if (leftReverseTicksTurns >= targetTicks)
      {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
      }
    }
    else if (dir == RIGHT)
    {
      if (rightReverseTicksTurns >= targetTicks)
      {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
//        delay(100);
      }
    }
    else if (dir == STOP)
    {
      deltaTicks = 0;
      targetTicks = 0;
      stop();
//      delay(100);
    }
  }
}
