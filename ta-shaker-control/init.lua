function shakerOn()
  sendI2C(0x61,1,0)
end

function shakerOff()
  sendI2C(0x61,2,0)
end

function shakerSpeed(speed)
  sendI2C(0x61,4,speed)
end

function shakerSequence(startIdx)
  sendI2C(0x61,5,startIdx)
end

