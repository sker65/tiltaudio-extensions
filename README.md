# Extensions for TILT!Audio

Some examples on how to extend the TILT!Audio board with slave controller boards like arduino mini pro.

* prominiextender: a super simple sketch borrowed from letscontrolit.com. Gives you additional GPIOs and also PWM outputs
* ta-rgb-extender: uses the famous FastLED library to control RGB-LED strips with 2812 LED controller.
* ta-servo-extender: simple example that controls one (or more) servo based on sound.
* ta-shaker-control: example how to controle a DC motor (like a shaker) with a DC motor driver module and a pwm ouput.
* speaker-lights is another example that uses 2812 RGB leds to controls speaker light rings. it has some self sufficient effects, that can be controlled by TILT!Audio

## WPC Sound Emu
is a ardunio based emulator of sound bus commands as the would generate by the pinball game. can be used to test your TILT!Audio board outside of the game.

## I2C Addresses
all extensions mentioned in this repository are using the i2c bus to communicate from TILT!Audio Pi to the arduino. If you had more than one "extension arduino" you need to avoid address conflicts.

Also the board has some "internal" addresses already reserved:
* 0x05: the stm slave board for all boards newer than rev. 3.5
* 0x3C: the OLED display (if applied).

Other than that the extensions mentioned here are on these addresses:
* servo 0x5F
* rgb (2812 also speaker lights) 0x60
* shaker 0x61
* miniproextender: 0x7F

So if you are going to extend these examples please avoid collisions of addresses