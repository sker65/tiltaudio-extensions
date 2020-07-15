#include <FastLED.h>
#include <platforms.h>

/*
 
 */

// This file is to be loaded onto an Arduino Pro Mini so it will act as a 2812 rgb led controller.
// Communication between tiltaudio and Arduino is using the I2C bus, so only two wires needed.
// It best to run the Pro Mini on 5V and 16MHz.
// That way, you can skip levelconverters on I2C.
// Arduino Mini Pro uses A4 and A5 for I2C bus.
// i2c address in this demo sketch is set to 0x60

#include <Wire.h>

#define I2C_MSG_IN_SIZE    4
#define I2C_MSG_OUT_SIZE   4
#define I2C_ADDRESS 0x60

#define MAX_BRIGHTNESS 164
#define MIN_BRIGHTNESS 32

#define CMD_SET_COLOR 1
#define CMD_SET_BRIGHTNESS 2
#define CMD_ALL_OFF 3
#define CMD_SET_EFFECT 4


volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

#define LEDS_PER_RING 16
#define NUM_LEDS 32
CRGB leds[ NUM_LEDS ];

#define MAX_COLORS 5
CRGB palette[] = {CRGB::Black, CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Yellow, CRGB::Magenta };

int currentEffect = 0;
int effectDelay = 500;
unsigned long lastTick = 0;

#define EFFECT_RUNNING_LED 1

CRGB currentColor = CRGB::Red;

void setup()
{
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  FastLED.addLeds<NEOPIXEL, 4>(leds, NUM_LEDS);
  fill_solid( leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(MIN_BRIGHTNESS);
  FastLED.show();
  
}

int effectRunningPos = 0;

void doRunningLedEffect() {
  effectRunningPos++;
  if( effectRunningPos >= LEDS_PER_RING ) { // let effectRunningPos run from 0 .. LEDS_PER_RING-1
    effectRunningPos = 0;
  }
  // update LED data
  for( int i = 0; i < LEDS_PER_RING, i++) {
    if( i == effectRunningPos){
      leds[i] = currentColor; // for ring 0
      leds[i+LEDS_PER_RING] = currentColor; // for ring 1
    } else {
      leds[i] = CRGB::Black;
      leds[i+LEDS_PER_RING] = CRGB::Black;
    }
  }
  FastLED.show();
}

void loop() {
  unsigned long now = millis();
  if( now > lastTick + effectDelay ) {    // do some effect every "delay millis"
    lastTick =  now;
    switch (currentEffect)
    {
    case EFFECT_RUNNING_LED:
      doRunningLedEffect();
      break;
    
    case 0: //  effect 0 all off
      fill_solid( leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      break;

    default:
      break;
    }
  }
}


int rangeCheck(int v, int min, int max ) {
  return v<min?min: ( v>max? max : v );
}

void receiveEvent(int count)
{
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte ring = Wire.read();
    int val = Wire.read();
    switch(cmd)
      {
        case CMD_SET_COLOR:
          val = rangeCheck(val, 0, MAX_COLORS);
          currentColor =  palette[val];
          if( currentEffect == 0 ) fill_solid(&leds[ring*LEDS_PER_RING], LEDS_PER_RING, palette[val]);
          FastLED.show();
          break;
        case CMD_SET_BRIGHTNESS:
          val = rangeCheck(val, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
          FastLED.setBrightness(val);
          FastLED.show();
          break;
        case CMD_ALL_OFF:
          fill_solid( leds, NUM_LEDS, CRGB::Black);
          FastLED.show();
          break;
        case CMD_SET_EFFECT:
          currentEffect = ring;
          effectDelay = val;
          break;
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
