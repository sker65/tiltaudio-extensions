// Controlling a servo position using i2c commands.
//
// This file is to be loaded onto an Arduino Pro Mini so it will act as a servo controller.
// Communication between tiltaudio and Arduino is using the I2C bus, so only two wires needed.
// It best to run the Pro Mini on 5V and 16MHz.
// That way, you can skip levelconverters on I2C.
// Arduino Mini Pro uses A4 and A5 for I2C bus.
// i2c address in this demo sketch is set to 0x5F

#include <Wire.h>
#include <Servo.h>

#define I2C_MSG_IN_SIZE    2
#define I2C_MSG_OUT_SIZE   4
#define I2C_ADDRESS 0x5F

Servo servo1;  // create servo object to control a servo

#define MAX_SERVOS 1
Servo* servos[MAX_SERVOS];

#define SERVO_SET 1

void setup() {
  // init i2c bus
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  int i = 0;
  // init servo object
  servo1.attach(9);  // attaches the servo on pin 9 to the servo object
  servos[i++] = &servo1;
}

void loop() {
}

int rangeLimit(int v, int min, int max ) {
  return v<min?min: ( v>max? max : v );
}

void receiveEvent(int count)
{
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();  
    byte servo = Wire.read();
    int value = Wire.read();
    if( servoNumber >= 0 && servoNumber < MAX_SERVOS ) {
      switch (cmd)
      {
      case SERVO_SET:
        servos[servo]->write(rangeLimit(value,0,180));
        break;
      
      default:
        break;
      }
    }
  }
}

void clearSendBuffer()
{
  for(byte x=0; x < sizeof(sendBuffer); x++)
    sendBuffer[x]=0;
}

void requestEvent()
{
  Wire.write((const uint8_t*)sendBuffer,sizeof(sendBuffer));
}