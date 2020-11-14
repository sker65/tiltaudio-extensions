#include <Wire.h>

#define I2C_MSG_IN_SIZE    2
#define I2C_MSG_OUT_SIZE   4
#define I2C_ADDRESS 0x61
#define PWM_PIN 5

void setup() {
  // init i2c bus
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  // not used Wire.onRequest(requestEvent);
  // init PWN output
  pinMode(PWM_PIN, OUTPUT);
}

int currentSpeed = 1;
int maxSpeed = 20;

void off() {
   analogWrite( PWM_PIN, 0 );
}

void on() {
   analogWrite( PWM_PIN, currentSpeed );
}

void setSpeed( int speed ) {
  if( speed > 0 && speed <= maxSpeed ) {
    currentSpeed = speed;
  }
  on();
}

void receiveEvent(int count)
{
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte value = Wire.read();
    switch( cmd ) {
      case 0x01:
        off();
        break;
      case 0x02:
        off();
        break;
      case 0x03:
        setSpeed(value);
        break;
      default:
        // unknown command
    }
  }
}