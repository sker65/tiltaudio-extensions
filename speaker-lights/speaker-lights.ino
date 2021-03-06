#include "FastLED.h"
#include <Wire.h>

// define LEDS per ring
#define NUM_LEDS 60
CRGB leds[NUM_LEDS * 2];

// if defined both rings are controlled with one wire, if not each ring has its own
#define ONE_WIRE
#define PIN 6
// only needed for two wires
// #define PIN_R 7

#define I2C_MSG_IN_SIZE 4
#define I2C_MSG_OUT_SIZE 4
#define I2C_ADDRESS 0x60

#define LED_PIN 13

volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

int selectedEffect = -1; // start with "-1" since the "loop" will right away increase it by 1
int cancelEffect = 0;
int calloutToPlay = 0;

int reverse_rotation = 1; // if not equal to zero rotation on second ring will be reversed

void setup()
{
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

#ifdef ONE_WIRE
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS*2).setCorrection(TypicalLEDStrip);
#else
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2811, PIN_R, GRB>(leds, NUM_LEDS, NUM_LEDS).setCorrection(TypicalLEDStrip); //Reverse direction
#endif
  // use internal led as optical feedback when i2c command incoming
  // or effects running
  pinMode(LED_PIN, OUTPUT);
}

void requestEvent()
{
  Wire.write((const uint8_t *)sendBuffer, sizeof(sendBuffer));
}

void receiveEvent(int count)
{
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte cancel = Wire.read();
    int val = Wire.read();
    switch (cmd)
    {
    case 1: // set effect
      selectedEffect = val;
      cancelEffect = cancel;
      break;
    case 2: // set rotation
      reverse_rotation = val;
      break;
    case 3: // play callout
      cancelEffect = 1;
      calloutToPlay = val;
      break;
    }
  }
}

void handleLed(unsigned long now) {
  static int state = 0;
  static unsigned long lastSwitch = 0;
  if( lastSwitch + 200 < now ) {
    lastSwitch = now;
    state = !state;
    digitalWrite(LED_PIN, state ? LOW : HIGH );
  }
}

void loopCallback(unsigned long now) {
  handleLed(now);
}

// effects rotating 3/4/9/10/11/12/15
// effect not working: #14

int cancelableDelay(int val)
{
  unsigned long now = millis();
  unsigned long end = now + val;
  int checkInterval = val > 20 ? 20 : val; // check every 20 ms
  while (now < end)
  {
    now = millis();
    loopCallback(now);
    if (cancelEffect)
    {
      return 1;
    }
    delay(end - now > checkInterval ? checkInterval : end - now);
  }
  return 0; // not canceled
}
// use to have cancelable effects
#define effectDelay(a)                \
  {                                   \
    int _cancel = cancelableDelay(a); \
    if (_cancel)                      \
      return;                         \
  }
// else simple delay
// #define effectDelay(a) delay(a)

void loop()
{
  /* UNCOMMENT to ENABLE auto loop (without i2c control)*/
    
    selectedEffect++; // increase effect to the next one

    if (selectedEffect > 15)
    {
      selectedEffect = 0;
    } 
    
    

  if (cancelEffect)
  { // fade to black if effect was cancel by command
    cancelEffect = 0;
    FadeToBlack(64, 6, calloutToPlay ? 1 : 50); // for callout rapidly fade to black
  }
  if (calloutToPlay)
  {
    switch (calloutToPlay)
    {
    case 1:
      playNowISeeYou();
      break;
    }
    calloutToPlay = 0;
  }
  else
  {

    switch (selectedEffect)
    {

    case 0:
    {
      // RGBLoop - no parameters
      RGBLoop();
      break;
    }

    case 1:
    {
      // FadeInOut - Color (red, green. blue)
      FadeInOut(0xff, 0x00, 0x00); // red
      FadeInOut(0x88, 0x00, 0xff); // vio
      FadeInOut(0x00, 0x00, 0xff); // blue
      break;
    }

    case 2:
    {
      // Strobe - Color (red, green, blue), number of flashes, flash speed, end pause
      Strobe(0xff, 0xff, 0xff, random(5, 20), 50, 500);
      break;
    }

    case 3:
    {
      // fast rainbowCycle - speed delay
      rainbowCycle(1);
      break;
    }

    case 4:
    {
      // NewKITT - Color (red, green, blue), eye size, speed delay, end pause
      NewKITT(0xff, 0x00, 0x00, 8, 10, 20);
      break;
    }

    case 5:
    {
      // Twinkle - Color (red, green, blue), count, speed delay, only one twinkle (true/false)
      Twinkle(0x88, 0x00, 0xff, 10, 50, false);
      break;
    }

    case 6:
    {
      // TwinkleRandom - twinkle count, speed delay, only one (true/false)
      TwinkleRandom(20, 50, false);
      break;
    }

    case 7:
    {
      // Sparkle - Color (red, green, blue), speed delay
      Sparkle(0xff, 0xff, 0xff, 50);
      break;
    }

    case 8:
    {
      // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
      SnowSparkle(0x10, 0x10, 0x10, 20, random(100, 200));
      break;
    }

    case 9:
    {
      // Running Lights - Color (red, green, blue), wave dealy
      RunningLights(0xff, 0x00, 0x00, 30); // red
      RunningLights(0x00, 0xff, 0x00, 30); // green
      RunningLights(0x00, 0x00, 0xff, 30); // blue
      break;
    }

    case 10:
    {
      // colorWipe - Color (red, green, blue), speed delay
      colorWipe(0x00, 0xff, 0x00, 20);
      colorWipe(0xff, 0x00, 0x00, 20);
      colorWipe(0x00, 0x00, 0xff, 20);
      //second round:
      colorWipe(0x00, 0xff, 0x00, 20);
      colorWipe(0xff, 0x00, 0x00, 20);
      colorWipe(0x00, 0x00, 0xff, 20);
      break;
    }

    case 11:
    {
      // rainbowCycle - speed delay
      rainbowCycle(5);
      break;
    }

    case 12:
    {
      // theatherChase - Color (red, green, blue), speed delay
      theaterChase(0x88, 0, 0xff, 50);
      break;
    }

    case 13:
    {
      // theaterChaseRainbow - Speed delay
      theaterChaseRainbow(20);
      break;
    }

    case 14:
    {
      // Fire - Cooling rate, Sparking rate, speed delay
      Fire(55, 120, 150, 30);
      break;
    }

    case 15:
    {
      // meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
      meteorRain(0xff, 0xff, 0xff, 10, 64, true, 20);
      meteorRain(0xff, 0xff, 0xff, 10, 64, true, 20);
      meteorRain(0x00, 0xff, 0x00, 10, 64, true, 30);
      meteorRain(0x00, 0xff, 0x00, 10, 64, true, 30);
      break;
    }
    case 16:
    {
      FadeToBlack(64, 6, 150);
      break;
    }
    }
  }
}

// *************************
// ** LEDEffect Functions **
// *************************

void FadeToBlack(int fadeValue, int iterations, int SpeedDelay)
{
  for (int i = 0; i < iterations; i++)
  {
    fadeAllToBlack(fadeValue);
    effectDelay(SpeedDelay);
  }
}

void RGBLoop()
{
  for (int j = 0; j < 4; j++)
  {
    // Fade IN
    for (int k = 0; k < 256; k++)
    {
      switch (j)
      {
      case 0:
        setAll(k, 0, 0);
        break;
      case 1:
        setAll(k, 0, k);
        break;
      case 2:
        setAll(0, 0, k);
        break;
      case 3:
        setAll(0, k, 0);
        break;
      }
      showStrip();
      effectDelay(1);
    }
    // Fade OUT
    for (int k = 255; k >= 0; k--)
    {
      switch (j)
      {
      case 0:
        setAll(k, 0, 0);
        break;
      case 1:
        setAll(k, 0, k);
        break;
      case 2:
        setAll(0, 0, k);
        break;
      case 3:
        setAll(0, k, 0);
        break;
      }
      showStrip();
      effectDelay(1);
    }
  }
}

void FadeInOut(byte red, byte green, byte blue)
{
  float r, g, b;

  for (int k = 0; k < 256; k = k + 1)
  {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    showStrip();
  }

  for (int k = 255; k >= 0; k = k - 2)
  {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    showStrip();
  }
}

void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause)
{
  for (int j = 0; j < StrobeCount; j++)
  {
    setAll(red, green, blue);
    showStrip();
    effectDelay(FlashDelay);
    setAll(0, 0, 0);
    showStrip();
    effectDelay(FlashDelay);
  }

  effectDelay(EndPause);
}

void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
}

// used by NewKITT
void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  for (int i = ((NUM_LEDS - EyeSize) / 2); i >= 0; i--)
  {
    setAll(0, 0, 0);

    setPixelR(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(i + j, red, green, blue);
    }
    setPixelR(i + EyeSize + 1, red / 10, green / 10, blue / 10);

    setPixelR(NUM_LEDS - i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(NUM_LEDS - i - j, red, green, blue);
    }
    setPixelR(NUM_LEDS - i - EyeSize - 1, red / 10, green / 10, blue / 10);

    showStrip();
    effectDelay(SpeedDelay);
  }
  effectDelay(ReturnDelay);
}

// used by NewKITT
void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  for (int i = 0; i <= ((NUM_LEDS - EyeSize) / 2); i++)
  {
    setAll(0, 0, 0);

    setPixelR(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(i + j, red, green, blue);
    }
    setPixelR(i + EyeSize + 1, red / 10, green / 10, blue / 10);

    setPixelR(NUM_LEDS - i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(NUM_LEDS - i - j, red, green, blue);
    }
    setPixelR(NUM_LEDS - i - EyeSize - 1, red / 10, green / 10, blue / 10);

    showStrip();
    effectDelay(SpeedDelay);
  }
  effectDelay(ReturnDelay);
}

// used by NewKITT
void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++)
  {
    setAll(0, 0, 0);
    setPixelR(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(i + j, red, green, blue);
    }
    setPixelR(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    effectDelay(SpeedDelay);
  }
  effectDelay(ReturnDelay);
}

// used by NewKITT
void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--)
  {
    setAll(0, 0, 0);
    setPixelR(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++)
    {
      setPixelR(i + j, red, green, blue);
    }
    setPixelR(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    effectDelay(SpeedDelay);
  }
  effectDelay(ReturnDelay);
}

void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne)
{
  setAll(0, 0, 0);

  for (int i = 0; i < Count; i++)
  {
    setPixel(random(NUM_LEDS), red, green, blue);
    showStrip();
    effectDelay(SpeedDelay);
    if (OnlyOne)
    {
      setAll(0, 0, 0);
    }
  }

  effectDelay(SpeedDelay);
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne)
{
  setAll(0, 0, 0);

  for (int i = 0; i < Count; i++)
  {
    setPixel(random(NUM_LEDS), random(0, 255), random(0, 255), random(0, 255));
    showStrip();
    effectDelay(SpeedDelay);
    if (OnlyOne)
    {
      setAll(0, 0, 0);
    }
  }

  effectDelay(SpeedDelay);
}

void Sparkle(byte red, byte green, byte blue, int SpeedDelay)
{
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, red, green, blue);
  showStrip();
  effectDelay(SpeedDelay);
  setPixel(Pixel, 0, 0, 0);
}

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay)
{
  setAll(red, green, blue);

  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, 0xff, 0xff, 0xff);
  showStrip();
  effectDelay(SparkleDelay);
  setPixel(Pixel, red, green, blue);
  showStrip();
  effectDelay(SpeedDelay);
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay)
{
  int Position = 0;

  for (int i = 0; i < NUM_LEDS * 2; i++)
  {
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < NUM_LEDS; i++)
    {
      // sine wave, 3 offset waves make a rainbow!
      //float level = sin(i+Position) * 127 + 128;
      //setPixel(i,level,0,0);
      //float level = sin(i+Position) * 127 + 128;
      setPixelR(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                ((sin(i + Position) * 127 + 128) / 255) * green,
                ((sin(i + Position) * 127 + 128) / 255) * blue);
    }

    showStrip();
    effectDelay(WaveDelay);
  }
}

void colorWipe(byte red, byte green, byte blue, int SpeedDelay)
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    setPixelR(i, red, green, blue);
    showStrip();
    effectDelay(SpeedDelay);
  }
}

void rainbowCycle(int SpeedDelay)
{
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < NUM_LEDS; i++)
    {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixelR(i, *c, *(c + 1), *(c + 2));
    }
    showStrip();
    effectDelay(SpeedDelay);
  }
}

// used by rainbowCycle and theaterChaseRainbow
byte *Wheel(byte WheelPos)
{
  static byte c[3];

  if (WheelPos < 85)
  {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  }
  else
  {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}

void theaterChase(byte red, byte green, byte blue, int SpeedDelay)
{
  for (int j = 0; j < 10; j++)
  { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++)
    {
      for (int i = 0; i < NUM_LEDS; i = i + 3)
      {
        setPixelR(i + q, red, green, blue); //turn every third pixel on
      }
      showStrip();

      effectDelay(SpeedDelay);

      for (int i = 0; i < NUM_LEDS; i = i + 3)
      {
        setPixelR(i + q, 0, 0, 0); //turn every third pixel off
      }
    }
  }
}

void theaterChaseRainbow(int SpeedDelay)
{
  byte *c;

  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++)
    {
      for (int i = 0; i < NUM_LEDS; i = i + 3)
      {
        c = Wheel((i + j) % 255);
        setPixel(i + q, *c, *(c + 1), *(c + 2)); //turn every third pixel on
      }
      showStrip();

      effectDelay(SpeedDelay);

      for (int i = 0; i < NUM_LEDS; i = i + 3)
      {
        setPixel(i + q, 0, 0, 0); //turn every third pixel off
      }
    }
  }
}

void Fire(int Cooling, int Sparking, int SpeedDelay, int Iterations)
{
  static byte heat[NUM_LEDS];
  int cooldown;

  for (int p = 0; p < Iterations; p++)
  {
    // Step 1.  Cool down every cell a little
    for (int i = 0; i < NUM_LEDS; i++)
    {
      cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);

      if (cooldown > heat[i])
      {
        heat[i] = 0;
      }
      else
      {
        heat[i] = heat[i] - cooldown;
      }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = NUM_LEDS - 1; k >= 2; k--)
    {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if (random(255) < Sparking)
    {
      int y = random(7);
      heat[y] = heat[y] + random(160, 255);
      //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for (int j = 0; j < NUM_LEDS; j++)
    {
      setPixelHeatColor(j, heat[j]);
    }

    showStrip();
    effectDelay(SpeedDelay);
  }
}

void setPixelHeatColor(int Pixel, byte temperature)
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2;              // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if (t192 > 0x80)
  { // hottest
    setPixel(Pixel, 255, 255, heatramp);
  }
  else if (t192 > 0x40)
  { // middle
    setPixel(Pixel, 255, heatramp, 0);
  }
  else
  { // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay)
{
  setAll(0, 0, 0);

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++)
  {

    // fade brightness all LEDs one step
    for (int j = 0; j < NUM_LEDS; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        fadeToBlack(j, meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < NUM_LEDS) && (i - j >= 0))
      {
        setPixelR(i - j, red, green, blue);
      }
    }

    showStrip();
    effectDelay(SpeedDelay);
  }
}

// used by meteorrain
void fadeToBlack(int ledNo, byte fadeValue)
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  uint32_t oldColor;
  uint8_t r, g, b;
  int value;

  oldColor = strip.getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r = (r <= 10) ? 0 : (int)r - (r * fadeValue / 256);
  g = (g <= 10) ? 0 : (int)g - (g * fadeValue / 256);
  b = (b <= 10) ? 0 : (int)b - (b * fadeValue / 256);

  strip.setPixelColor(ledNo, r, g, b);
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[ledNo].fadeToBlackBy(fadeValue);
  leds[ledNo + NUM_LEDS].fadeToBlackBy(fadeValue);
#endif
}

void fadeAllToBlack(byte fadeValue)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    fadeToBlack(i, fadeValue);
  }
}

// ***************************************
// ** FastLed/NeoPixel Common Functions **
// ***************************************

// Apply LED color changes
void showStrip()
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  FastLED.show();
#endif
}

// Set a LED color (not yet visible) / both rings
void setPixel(int Pixel, byte red, byte green, byte blue)
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
  leds[Pixel + NUM_LEDS].r = red;
  leds[Pixel + NUM_LEDS].g = green;
  leds[Pixel + NUM_LEDS].b = blue;
#endif
}

void setPixelR(int Pixel, byte red, byte green, byte blue)
{
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  int e = NUM_LEDS * 2 - 1;
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
  if (reverse_rotation)
  {
    leds[e - Pixel].r = red;
    leds[e - Pixel].g = green;
    leds[e - Pixel].b = blue;
  }
  else
  {
    leds[Pixel + NUM_LEDS].r = red;
    leds[Pixel + NUM_LEDS].g = green;
    leds[Pixel + NUM_LEDS].b = blue;
  }
#endif
}

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

#define NOW_I_SEE_YOU_SAMPLES 107
// volume sample from callout "Now I see you" sample frequecy 21ms
byte nowISeeYou[NOW_I_SEE_YOU_SAMPLES] = {
    230,
    120,
    130,
    120,
    110,
    110,
    20,
    110,
    40,
    40,
    140,
    0,
    40,
    80,
    30,
    110,
    100,
    50,
    20,
    40,
    60,
    70,
    60,
    70,
    50,
    60,
    40,
    130,
    50,
    60,
    120,
    30,
    60,
    20,
    50,
    80,
    50,
    80,
    90,
    100,
    70,
    20,
    90,
    50,
    80,
    100,
    150,
    20,
    60,
    140,
    30,
    70,
    150,
    60,
    30,
    20,
    50,
    255,
    120,
    60,
    120,
    70,
    180,
    140,
    50,
    30,
    50,
    150,
    110,
    110,
    70,
    255,
    255,
    60,
    150,
    120,
    70,
    140,
    20,
    70,
    30,
    50,
    110,
    90,
    160,
    120,
    200,
    160,
    90,
    140,
    255,
    90,
    140,
    70,
    120,
    40,
    50,
    100,
    130,
    90,
    200,
    60,
    100,
    110,
    50,
    210,
    140,
};

void playNowISeeYou()
{
  for (int i = 0; i < NOW_I_SEE_YOU_SAMPLES; i++)
  {
    setAll(nowISeeYou[i], nowISeeYou[i], nowISeeYou[i]);
    effectDelay(21);
  }
}
