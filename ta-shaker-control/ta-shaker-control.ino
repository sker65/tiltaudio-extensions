#include <Wire.h>

#define I2C_MSG_IN_SIZE    2
#define I2C_MSG_OUT_SIZE   4
#define I2C_ADDRESS 0x61
#define PWM_PIN 5
#define LED 13

int ledBlinks = 0;
int ledState = 0;
unsigned long nextBlink = 0;

void setup() {
  // init i2c bus
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
//  Wire.onRequest(requestEvent);
  // init PWN output
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED,OUTPUT);
  ledBlinks = 10;
  analogWrite(PWM_PIN,0); 
}

int currentSpeed = 0;
int maxSpeed = 255;

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
#define SETSPEED 0x04
#define PLAYSEQ 0x05

volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

int sequences[] = { 
  RAMP, 90, 5000, RAMP, 00, 5000, RAMP, 160, 5000, RAMP, 00, 5000, OFF, 0, 100, END, 
  // 16
  ON, 25, 1000, OFF, 0, 1000, ON, 25, 1500, OFF, 0, 1000, END,
};
int seqIdx = -1;
int sizeOfSequences = 20;     // length of sequences array

unsigned long nextAction = 0; // timer marker for next action
int rampInc;                  // speed incs for ramps in steps
int rampNo = 0;               // counts number of step in ramp phase
int rampDelay;                // speed delay for ramps in steps

void loop() {
  unsigned long now = millis();
  if( ledBlinks > 0 && now > nextBlink ) {
    nextBlink = now + 100;
    if( ledState ) {
      digitalWrite(LED, LOW) ;
      ledBlinks--;
      if( ledBlinks == 0 ) nextBlink = 0;
    } else {
      digitalWrite(LED, HIGH );
    }
    ledState = !ledState;
  }
  if( now > nextAction ) {

    // ramp active
    if( rampNo > 0 ) {
      nextAction = now + rampDelay;
      setSpeed( currentSpeed + rampInc );
      rampNo--;
    } else {
      // read next command from sequence
      int cmd = sequences[seqIdx++];
      if( cmd == END ) {
        seqIdx = -1; // end of sequence
        nextAction = 0;
      } else {
        int val = sequences[seqIdx++];
        int delay = sequences[seqIdx++];
        nextAction = now + delay;
        switch(cmd) {
          case ON:  setSpeed(val); break;
          case OFF: off(); break;
          case RAMP:
            rampNo = delay / 200; // inc every x ms
            rampDelay = 200;
            nextAction = now;
            rampInc = (val - currentSpeed) / rampNo; 
            break;
        }
      }
    }
  }
}

void playSequence( int startIndex ) {
  if( startIndex >= 0 && startIndex < sizeOfSequences ) {
    seqIdx = startIndex;
    nextAction = millis();
  }
}

void receiveEvent(int count) {
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte value = Wire.read();
//    Wire.read();
//    Wire.read();
    ledBlinks = cmd;
    switch( cmd ) {
      case ON:
        on();
        break;
      case OFF:
        off();
        break;
      case SETSPEED:
        setSpeed(value);
        break;
      case PLAYSEQ:
        playSequence(value);
        break;
      default:
        // unknown command
        break;
    }
  }
}
