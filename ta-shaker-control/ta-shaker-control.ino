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

#define END 0x00
#define ON 0x01
#define OFF 0x02
#define RAMP 0x03

int sequences[] = { ON, 5, 500, OFF, 0, 500, ON, 5, 500, END };
int* seqPtr = NULL;

unsigned long nextAction = 0; // timer marker for next action
float rampInc;                // speed incs for ramps in steps
int rampNo = 0;               // counts number of step in ramp phase

void loop() {
  unsigned long now = millis();
  if( now > nextAction ) {

    // ramp active
    if( rampNo > 0 ) {
      nextAction = now + 50;
      setSpeed( currentSpeed + rampInc );
      rampNo--;
    } else {
      // read next command from sequence
      int cmd = *seqPtr++;
      if( cmd == END ) {
        seqPtr = NULL; // end of sequence
        nextAction = 0;
      } else {
        int val = *seqPtr++;
        int delay = *seqPtr++;
        nextAction = now + delay;
        switch(cmd) {
          case ON:  setSpeed(val); break;
          case OFF: off(); break;
          case RAMP:
            rampNo = delay / 50; // inc every 50ms
            nextAction = now + 50;
            rampInc = (val - currentSpeed) / (float) rampNo; 
        }
      }
    }
  }
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