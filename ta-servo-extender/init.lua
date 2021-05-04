function setServo(servoNumber, value)
  sendI2C(0x5F,1,servo,value)
end
