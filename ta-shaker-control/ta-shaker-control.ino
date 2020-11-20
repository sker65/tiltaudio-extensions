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
  // let led blink 10 times at startup
  ledBlinks = 10;
  // shaker speed 0
  analogWrite(PWM_PIN,0); 
}

// remember and limit speed
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

// command bytes
#define END 0x00
#define ON 0x01
#define OFF 0x02
#define RAMP 0x03
#define SETSPEED 0x04
#define PLAYSEQ 0x05

// array that stores all sequences that can be triggered, must end with END command
// each sequence step consists of CMD, val & and delay in milliseconds, e.g. RAMP, 90, 1000 means
// RAMP up / down current speed to 90 in 1 second.
int sequences[] = { 
  RAMP, 90, 5000, RAMP, 00, 5000, RAMP, 160, 5000, RAMP, 00, 5000, OFF, 0, 100, END, 
  // 16
  ON, 25, 1000, OFF, 0, 1000, ON, 25, 1500, OFF, 0, 1000, END,
};

int seqIdx = -1;
int startIndex[] = { 0, 16 }; // start index for each sequence
#define MAX_SEQUENCES 2

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

void playSequence( int seq ) {
  if( seq >= 0 && seq < MAX_SEQUENCES ) {
    seqIdx = startIndex[seq];
    nextAction = millis();
  }
}

void receiveEvent(int count) {
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte value = Wire.read();
    // blink to ack the received cmd
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
