# TILT!Audio shaker control

Shaker control via Arduino mini pro with full pwm control.

Find the blog post for this shaker extension here: https://tiltaudio.com/2020/11/14/shaker-control-with-pwm/

## Commands

The script listens for simple commands on the i2c bus at address 0x61 (can be changed in the code)

Most complex command is sequence playback. For this there is a small state engine implemented in the
main loop, that changes shaker speed based on sequences that a defined in the code.
